//FULL CORRECTED DRISHTIKON DETECTOR SIDE CODE RUNNING SUCCESSFULLY ON SERIAL TERMINAL AND BLINK TERMINAL WIDGET
//WITH NEW DEVICE AND UPDATED AUTH TOKENS

/**************************************************************
 * DETECTOR DEVICE SKETCH (ESP32-S3)
 * MLX90640 and OPT3001 with TensorFlow Lite and Blynk Integration.
 **************************************************************/

// =============================================================
// ⚡ BLYNK INTEGRATION START
// =============================================================

// BLYNK CREDENTIALS - UPDATE THESE WITH YOUR ACTUAL VALUES
#define BLYNK_TEMPLATE_NAME "BUBBA"
#define BLYNK_TEMPLATE_ID   "TMPL3WgBo7eIO"   // Use the ID from your image
#define BLYNK_DEVICE_NAME   "detector_deba"           // Use the name for your Detector device
#define BLYNK_AUTH_TOKEN    "Ur6epRci-aIiDYLXdta9fKl2EKcgem5b" // Use the unique token for the Detector





#include <BlynkSimpleEsp32.h>
#include "Blynk/BlynkTimer.h" // Include the timer library

// --- BLYNK VIRTUAL PIN FOR LOGGING ---
#define VIRTUAL_PIN_TERMINAL V9
char logBuffer[256];


// Function to print text messages to both the Serial Monitor and the Blynk Terminal Widget
void blynkLog(const char* message) {
    Serial.println(message);
    // VIRTUAL_PIN_TERMINAL is V9
    Blynk.virtualWrite(VIRTUAL_PIN_TERMINAL, message); 
}


// Wi-Fi Credentials - UPDATE THESE WITH YOUR ACTUAL VALUES
char ssid[] = "ERROR";
char pass[] = "skansal937";

BlynkTimer timer; // Timer object for periodic Blynk tasks

// Global variables to store the most recent sensor data for sending
float current_lux = 0.0f;
float current_avg_temp = 0.0f;

// Function to send data to Blynk (called by the timer)
void sendSensorDataToBlynk() {
  // Send Illuminance (lux) to Virtual Pin V0
  Blynk.virtualWrite(V0, current_lux);

  // Send Average Temperature (Avg Temp) to Virtual Pin V1
  Blynk.virtualWrite(V1, current_avg_temp);

  Serial.println(">>> Blynk Data Sent <<<");
  Serial.print("V0 (Lux): "); Serial.println(current_lux);
  Serial.print("V1 (Avg Temp): "); Serial.println(current_avg_temp);
}


// =============================================================
// ⚡ BLYNK INTEGRATION END
// =============================================================

//OPT and MLX full woking correct

/**
 * TensorFlow Lite Bounding Box Predictor
 * Sensor: MLX90640 (32x24 Thermal Array)
 * Note: Assumes the model was trained on 32x24 thermal data normalized between T_MIN and T_MAX.
 */

#include "mlx_sensor_model_quantized.h"
#include <tflm_esp32.h>
#include <eloquent_tinyml.h>
#include <Wire.h> // MLX90640 uses I2C
#include <Adafruit_MLX90640.h> // The MLX sensor library
#include <opt3001.h>


// --- PIN DEFINITIONS ---
// I2C Bus 1: MLX90640 (Kept on standard 'Wire' object)
#define MLX_SDA_PIN 8
#define MLX_SCL_PIN 9

// I2C Bus 2: OPT3001 (Moved to custom 'Wire_OPT' object)
#define OPT_SDA_PIN 11
#define OPT_SCL_PIN 10
#define OPT_INT_PIN 12

// ... (rest of the #defines for TF and global variables)

// --- I2C OBJECTS ---
// 1. OPT3001 will use the custom I2C object (Wire_OPT)
TwoWire Wire_OPT = TwoWire(1); // Use I2C port 0 for OPT (Custom pins 10, 11)

#define TF_NUM_OPS 10
#define ARENA_SIZE 60000

Eloquent::TF::Sequential<TF_NUM_OPS, ARENA_SIZE> tf;

//CRITICAL CHANGE: Dimensions match the MLX90640 sensor 
#define IMG_WIDTH 32
#define IMG_HEIGHT 24
#define INPUT_SIZE (IMG_WIDTH * IMG_HEIGHT) // 32 * 24 = 768

// Define normalization constants used during model training
// YOU MUST CONFIRM THESE VALUES based on your training data
const float T_MIN = 0.0f; // Minimum temp (C) used in training
const float T_MAX = 50.0f; // Maximum temp (C) used in training
const float T_RANGE = T_MAX - T_MIN;

// Sensor data buffers
float mlx_frame[INPUT_SIZE];// Buffer to hold raw MLX90640 temp readings (32x24)
float input_image[INPUT_SIZE];// Global array to feed the TFLite model

// Define the MLX90640 sensor object
Adafruit_MLX90640 mlx;
// Create sensor object
opt3001 sensor;

// I2C address (0x44, 0x45, 0x46, or 0x47)
const uint8_t I2C_ADDRESS = 0x44;








//-------------------------------------------------------------
// SETUP
//-------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    delay(1500);

    // 1. --- I2C Initialization for MLX90640 (Pins 8, 9) ---
    // The standard 'Wire' object is used and initialized to MLX pins.
    Wire.begin(MLX_SDA_PIN, MLX_SCL_PIN);

    Serial.println("TENSORFLOW THERMAL OBJECT DETECTION");
    Serial.print("MLX Bus (Wire): SDA: GPIO "); Serial.print(MLX_SDA_PIN);
    Serial.print(", SCL: GPIO "); Serial.println(MLX_SCL_PIN);

    // Initialize MLX90640, passing the standard '&Wire' object
    if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
      Serial.println("MLX90640 not found! Check wiring and power.");
      while (1) delay(100);
    }
    // Set refresh rate
    mlx.setRefreshRate(MLX90640_4_HZ);

    // 2. --- I2C Initialization for OPT3001 (Pins 10, 11) ---
    // The custom 'Wire_OPT' object is initialized here.
    Wire_OPT.begin(OPT_SDA_PIN, OPT_SCL_PIN, 100000);
    pinMode(OPT_INT_PIN, INPUT_PULLUP);


    Serial.println("\nOPT3001 Sensor Initialization");
    Serial.print("OPT Bus (Wire_OPT): SDA: GPIO "); Serial.print(OPT_SDA_PIN);
    Serial.print(", SCL: GPIO "); Serial.println(OPT_SCL_PIN);

    // Setup the OPT3001, passing the custom 'Wire_OPT' object
    // 2. --- I2C Initialization for OPT3001 (Pins 10, 11 using Wire_OPT) ---
// ... (Wire_OPT.begin() and pinMode(OPT_INT_PIN) remain here) ...

// Setup the OPT3001, passing the custom 'Wire_OPT' object
if (sensor.setup(Wire_OPT, I2C_ADDRESS) != 0) {
    // If setup fails, we assume a connection issue or wrong address.
    Serial.println("Failed to setup OPT3001. Check I2C bus connection/address.");
} else { // <--- SUCCESS: setup() returned 0 (i.e., NO error)
    // Assume success and proceed directly to configuration
    sensor.config_set(OPT3001_CONVERSION_TIME_100MS);
    sensor.conversion_continuous_enable();
    Serial.println("OPT3001 initialized successfully!");
}
// The redundant 'else' case is now covered by the first 'if' statement.

    // 3. --- TensorFlow Setup ---
    delay(1500); // Give sensors time to stabilize
    tf.setNumInputs(INPUT_SIZE);
    tf.setNumOutputs(4);

    tf.resolver.AddFullyConnected();
    tf.resolver.AddConv2D();
    tf.resolver.AddDepthwiseConv2D();
    tf.resolver.AddReshape();
    
    while (!tf.begin(mlx_sensor_model_quantized).isOk())
        Serial.println(tf.exception.toString());
    
    Serial.println("TensorFlow Model Initialized.");

    // =============================================================
    // ⚡ BLYNK SETUP CODE
    // =============================================================
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Set up the timer to call sendSensorDataToBlynk() every 5 seconds (5000 milliseconds)
    // This allows your loop() to run your TFLite model at its own pace.
    timer.setInterval(5000L, sendSensorDataToBlynk); //data sent every 2 seconds to blink
    // =============================================================
}





//-------------------------------------------------------------
// LOOP
//-------------------------------------------------------------
void loop() {
    // =============================================================
    // ⚡ BLYNK LOOP CALLS
    // =============================================================
    Blynk.run();
    timer.run();
    // =============================================================

    float lux;
    delay(150); // Your existing delay to match OPT3001 conversion time

    // Read illuminance in lux
    if (sensor.lux_read(&lux) == 0) {
        // Store the value globally for the Blynk timer to send
        current_lux = lux; 
        // Format the Lux message into the buffer
        snprintf(logBuffer, sizeof(logBuffer), "Illuminance: %.2f lux", current_lux);
        blynkLog(logBuffer); // Send to both Serial and Blynk
    } else {
        // This output confirms the lux read failed (usually due to not being ready).
        blynkLog("Failed to read illuminance (Data not ready or communication error)");
    }



    // Reading interval set to 500ms (half a second)
    // You should ensure this delay is longer than the conversion time (100ms or 800ms)
    //delay(500); // <-- If this delay is needed for your MLX/TFLite speed, keep it!

    // 1. Read thermal data from the MLX90640 sensor
    if (mlx.getFrame(mlx_frame) == 0) {
    
        // 2. Normalize and prepare data for the model
        for (int i = 0; i < INPUT_SIZE; i++) {
            input_image[i] = (mlx_frame[i] - T_MIN) / T_RANGE;
    
            // Safety clamp
            if (input_image[i] < 0.0f) input_image[i] = 0.0f;
            if (input_image[i] > 1.0f) input_image[i] = 1.0f;
        }

        // 3. Run prediction
        if (!tf.predict(input_image).isOk()) {
            Serial.println(tf.exception.toString());
            return;
        }
    
        // 4. Interpret Results
        float xmin = tf.output(0);
        float ymin = tf.output(1);
        float xmax = tf.output(2);
        float ymax = tf.output(3);

        // Calculate the predicted width, height, and area (normalized)
        float width = xmax - xmin;
        float height = ymax - ymin;
        float area = width * height;

        // --- 1A. Bounding Box Area Threshold Check ---
        const float MIN_AREA_THRESHOLD = 0.01f; // Tune this

        if (area > MIN_AREA_THRESHOLD) {
            // A box of sufficient size was detected. Now, verify the temperature.
    
            // Denormalize coordinates
            int pixel_xmin = (int)(xmin * IMG_WIDTH);
            int pixel_ymin = (int)(ymin * IMG_HEIGHT);
            int pixel_xmax = (int)(xmax * IMG_WIDTH);
            int pixel_ymax = (int)(ymax * IMG_HEIGHT);

            //START OF ADDED THERMAL ANALYSIS CODE 
            float total_temp = 0.0f;
            int pixel_count = 0;

            // Loop through the MLX90640's 32x24 grid pixels inside the bounding box
            for (int y = pixel_ymin; y < pixel_ymax; y++) {
                for (int x = pixel_xmin; x < pixel_xmax; x++) {
                    // Safety check to ensure we stay within the 32x24 boundaries
                    if (x >= 0 && x < IMG_WIDTH && y >= 0 && y < IMG_HEIGHT) {
                        // Calculate the 1D index: index = y * width + x
                        int index = y * IMG_WIDTH + x;
    
                        // Add the raw temperature (Celsius) from the MLX frame
                        total_temp += mlx_frame[index];
                        pixel_count++;
                    }
                }
            }

            float avg_temp = (pixel_count > 0) ? (total_temp / pixel_count) : 0.0f;
            
            // Store the value globally for the Blynk timer to send
            current_avg_temp = avg_temp;

            // --- 2C. Temperature Threshold Check ---
            const float HUMAN_TEMP_THRESHOLD = 28.0f; // Tune this

            if (avg_temp > HUMAN_TEMP_THRESHOLD) {
                // Format the full HUMAN DETECTED message
                snprintf(logBuffer, sizeof(logBuffer), 
                "STATUS: HUMAN DETECTED (Avg Temp: %.1f°C) Box: [%.2f,%.2f to %.2f,%.2f]",
                avg_temp, xmin, ymin, xmax, ymax);
                blynkLog(logBuffer);
            } else {
                // Format the full WARM OBJECT message
                snprintf(logBuffer, sizeof(logBuffer), 
                "STATUS: Warm Object (Avg Temp: %.1f°C, not hot enough)",
                avg_temp);
                blynkLog(logBuffer);
            }
            //END OF ADDED THERMAL ANALYSIS CODE 

        } else {
            blynkLog("STATUS: Empty Room / No Significant Detection.");
            // If no detection, set temp to a default/low value
            current_avg_temp = 0.0f; 
        }
    }
    
    // Short delay to keep the system responsive
    delay(10);

}
//drishtikon talks with python script to fetch feed

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <Wire.h>
#include <Adafruit_MLX90640.h>

// --- Wi-Fi Configuration ---
const char* WIFI_SSID = "ERROR";
const char* WIFI_PASSWORD = "skansal937";
const int TCP_PORT = 11000;

WiFiServer server(TCP_PORT);
WiFiClient client;

// --- Sensor and Frame Definitions (Keep these from the previous code) ---
#define I2C_SDA_PIN 8 // CHECK YOUR ESP32-S3 PINS
#define I2C_SCL_PIN 9 // CHECK YOUR ESP32-S3 PINS
#define MLX_PIXELS_X 32
#define MLX_PIXELS_Y 24
#define MLX_PIXELS_TOTAL (MLX_PIXELS_X * MLX_PIXELS_Y)

// Interpolation Target Resolution (e.g., 4x upscale)
#define INTERP_SCALE_FACTOR 4
#define INTERP_PIXELS_X (MLX_PIXELS_X * INTERP_SCALE_FACTOR) // 128
#define INTERP_PIXELS_Y (MLX_PIXELS_Y * INTERP_SCALE_FACTOR) // 96
#define INTERP_PIXELS_TOTAL (INTERP_PIXELS_X * INTERP_PIXELS_Y)

// Buffers
float raw_frame[MLX_PIXELS_TOTAL];
float interp_frame[INTERP_PIXELS_TOTAL];
const size_t FRAME_BYTE_SIZE = INTERP_PIXELS_TOTAL * sizeof(float); // 128*96*4 bytes

Adafruit_MLX90640 mlx;

// --- Function Prototypes (Keep this and the full function body) ---
void bilinearInterpolation(const float *src, float *dst, uint8_t srcW, uint8_t srcH, uint8_t scaleFactor);

// ---------------------------
// Setup Function
// ---------------------------
void setup() {
  Serial.begin(115200);
  delay(100);

  // 1. Initialize Wi-Fi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("ESP32-S3 IP Address: ");
  Serial.println(WiFi.localIP());
  
  // Start TCP Server
  server.begin();
  Serial.print("TCP Server started on port: ");
  Serial.println(TCP_PORT);
  Serial.println("Waiting for client connection...");

  // 2. Initialize MLX90640
  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setClock(1000000); // 1MHz I2C Speed

  if (!mlx.begin(MLX90640_I2CADDR_DEFAULT, &Wire)) {
    Serial.println("Error! MLX90640 not found.");
    while (1) delay(100);
  }
  mlx.setRefreshRate(MLX90640_16_HZ);
}

// ---------------------------
// Main Loop
// ---------------------------
void loop() {
  // Check for new client connection
  if (!client.connected() && server.hasClient()) {
    client = server.available();
    Serial.print("New client connected from: ");
    Serial.println(client.remoteIP());
  }

  // If a client is connected, process and send data
  if (client.connected()) {
    // 1. Capture Raw Data
    if (mlx.getFrame(raw_frame) != 0) {
      // If read fails, wait and try again
      delay(10); 
      return; 
    }
    
    // 2. Interpolate the Frame
    bilinearInterpolation(raw_frame, interp_frame, MLX_PIXELS_X, MLX_PIXELS_Y, INTERP_SCALE_FACTOR);
    
    // 3. Transmit Raw Binary Data
    // We cast the float array to a byte pointer for raw, efficient transmission.
    size_t bytesSent = client.write((const uint8_t*)interp_frame, FRAME_BYTE_SIZE);

    if (bytesSent != FRAME_BYTE_SIZE) {
      // If the send failed or was incomplete, the client may have disconnected
      Serial.println("Failed to send full frame. Client disconnected or error.");
      client.stop(); // Close the connection
      return;
    }
    
    Serial.print("."); // Indicate a successful frame send
    
    // The delay here is not strictly necessary for 8Hz but can reduce Wi-Fi congestion
    // or limit the data rate for your host machine's processing speed.
    // delay(10); 
  }
}

// ---------------------------
// Bilinear Interpolation Function (Place the full function body here)
// ---------------------------
void bilinearInterpolation(const float *src, float *dst, uint8_t srcW, uint8_t srcH, uint8_t scaleFactor) {
  // ... [PASTE THE FULL BILINEAR INTERPOLATION FUNCTION FROM THE PREVIOUS RESPONSE HERE] ...
    // Calculate destination dimensions
  uint16_t dstW = srcW * scaleFactor;
  uint16_t dstH = srcH * scaleFactor;

  for (uint16_t j = 0; j < dstH; ++j) {
    for (uint16_t i = 0; i < dstW; ++i) {
      // Find the corresponding float coordinates in the source image
      float gx = ((float)i) / scaleFactor;
      float gy = ((float)j) / scaleFactor;
      
      // Calculate coordinates of the top-left source pixel
      uint8_t gxi = (uint8_t)gx;
      uint8_t gyi = (uint8_t)gy;

      // Handle edge cases by clamping (use the last valid pixel)
      if (gxi >= srcW - 1) gxi = srcW - 2;
      if (gyi >= srcH - 1) gyi = srcH - 2;

      // Get the four surrounding pixel temperatures (Q11, Q21, Q12, Q22)
      // Note: Coordinates are (x, y) where x is horizontal index, y is vertical index.
      float Q11 = src[gyi * srcW + gxi];       // Top-left
      float Q21 = src[gyi * srcW + gxi + 1];   // Top-right
      float Q12 = src[(gyi + 1) * srcW + gxi]; // Bottom-left
      float Q22 = src[(gyi + 1) * srcW + gxi + 1]; // Bottom-right
      
      // Get the fractional parts
      float a = gx - gxi;
      float b = gy - gyi;

      // Perform Bilinear Interpolation
      // 1. Interpolate horizontally for the top row (R1)
      float R1 = Q11 * (1.0f - a) + Q21 * a;
      // 2. Interpolate horizontally for the bottom row (R2)
      float R2 = Q12 * (1.0f - a) + Q22 * a;
      // 3. Interpolate vertically (P)
      float P = R1 * (1.0f - b) + R2 * b;

      // Store the result in the destination array
      dst[j * dstW + i] = P;
    }
  }
}
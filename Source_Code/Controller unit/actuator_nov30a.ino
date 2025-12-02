// ACTUATOR - corrected to actively sync V0 & V1 from server

#define BLYNK_TEMPLATE_NAME "BUBBA"
#define BLYNK_TEMPLATE_ID   "TMPL3WgBo7eIO"
#define BLYNK_DEVICE_NAME   "actuator_deba"
#define BLYNK_AUTH_TOKEN    "O2b45fOqHpsXpWNF1oER5rfkrAiF6SCV"



#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include "Blynk/BlynkTimer.h"

#define VIRTUAL_PIN_TERMINAL_ACTUATOR V9

char logBufferActuator[256];

// Simple logging helper: Serial + (optionally) Terminal widget
void blynkLogActuator(const char* message) {
    Serial.println(message);
    // If V9 exists in your project as a terminal/label, this will show it there
    //Blynk.virtualWrite(VIRTUAL_PIN_TERMINAL_ACTUATOR, message);
}

// Wi-Fi credentials
char ssid[] = "ERROR";
char pass[] = "skansal937";

const int LED_PIN = 2;
const float HUMAN_TEMP_THRESHOLD = 28.0f;

float receivedLux = 0.0f;
float receivedAvgTemp = 0.0f;

BlynkTimer actuatorTimer; // timer for periodic syncs

// Called when server pushes V0 (lux)
BLYNK_WRITE(V0) {
    receivedLux = param.asFloat();
    snprintf(logBufferActuator, sizeof(logBufferActuator), "V0 Lux Received: %.2f", receivedLux);
    blynkLogActuator(logBufferActuator);
}

// Called when server pushes V1 (avg temp)
BLYNK_WRITE(V1) {
    receivedAvgTemp = param.asFloat();
    snprintf(logBufferActuator, sizeof(logBufferActuator), "V1 Temp Received: %.1f°C", receivedAvgTemp);
    blynkLogActuator(logBufferActuator);

    if (receivedAvgTemp > HUMAN_TEMP_THRESHOLD) {
        digitalWrite(LED_PIN, HIGH);
        blynkLogActuator("-> LED ON: Human Temp Threshold Exceeded!");
    } else {
        digitalWrite(LED_PIN, LOW);
        blynkLogActuator("-> LED OFF: Temperature is below threshold.");
    }
}

// This runs when Blynk connection is established/re-established
BLYNK_CONNECTED() {
    // Ask server to send current values for V0 & V1 right away
    Blynk.syncVirtual(V0);
    Blynk.syncVirtual(V1);

    blynkLogActuator("Blynk Connected: synced V0 & V1");
}

// Periodic sync function (keeps actuator in sync with detector)
void periodicSync() {
    // This forces the server to push the latest datastream values to this device
    Blynk.syncVirtual(V0);
    Blynk.syncVirtual(V1);
    Serial.println("Periodic sync requested (V0 & V1).");
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

Serial.print("Connecting to WiFi: ");
Serial.println(ssid);


unsigned long start = millis();
while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > 10000) {
        Serial.println("WiFi failed!");
        break;
    }
}
Serial.println();
Serial.print("IP Address: ");
Serial.println(WiFi.localIP());



    // Connect to Blynk (auth token, ssid, pass)
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    // Setup a timer to call periodicSync every 2 seconds (adjust if needed)
    actuatorTimer.setInterval(5000L, periodicSync);

    blynkLogActuator("Actuator setup complete. Waiting for Blynk...");
}

void loop() {
Serial.print("WiFi Status: ");
Serial.println(WiFi.status());

    Blynk.run();
    actuatorTimer.run();
}
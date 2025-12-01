/**
 * @file tag_esp32_hs.ino
 * @author Renzo Mischianti
 * @brief Unified example for basic ANCHOR communication, demonstrating both Async and Sync methods.
 * @version 1.0.0
 * @date 2025-10-10
 *
 * This sketch contains two methods for communicating with a Tag.
 * METHOD 1 (Asynchronous) is recommended as it does not block the main loop.
 * METHOD 2 (Synchronous) is simpler but blocks the loop while waiting for a response.
 *
 * **Instructions:** Uncomment the method you want to use in the `loop()` function and comment out the other.
 *
 * @copyright Copyright (c) 2024
 *
 */

// Converted to TAG sketch (responds to Anchor polls)

#include <RYUW122.h>

// --- Configuration ---
const char* NETWORK_ID = "AABBCCDD";
const char* TAG_ADDRESS = "T3T3T3T3"; // unique for this tag

// ------------------ ARDUINO UNO SOFTWARE SERIAL ------------------
// Define pins for SoftwareSerial (adjust based on your board)
// #define RX_PIN 10  // Connect to RYUW122 TX
// #define TX_PIN 11  // Connect to RYUW122 RX
//
// // Create RYUW122 instance with SoftwareSerial for Arduino UNO
// RYUW122 uwb(TX_PIN, RX_PIN, RYUW122BaudRate::B_9600);
// -----------------------------------------------------------------
// -------------------------- ARDUINO MEGA -------------------------
// RYUW122 uwb(&Serial1);
// -----------------------------------------------------------------
// ------------------------ ESP32 ----------------------------------
// --- Configuration ---
#define RX_PIN 9  // Connect to RYUW122 TX
#define TX_PIN 10  // Connect to RYUW122 RX
#define RESET_PIN 13 // Connect to RYUW122 NRST (active LOW)
//
// #define RX_PIN 5  // Connect to RYUW122 TX
// #define TX_PIN 4  // Connect to RYUW122 RX
// #define RESET_PIN 6 // Connect to RYUW122 NRST (active LOW)
//
RYUW122 uwb(TX_PIN, RX_PIN, &Serial1, RESET_PIN);
// -----------------------------------------------------------------

void onTagDataReceived(int payloadLength, const char* data, int rssi) {
    Serial.println(F("\n--- Data Received from Anchor ---"));
    Serial.print(F("Data: ")); Serial.println(data);
    Serial.print(F("RSSI: ")); Serial.print(rssi); Serial.println(F(" dBm"));

    // Respond with distance reading placeholder (module will fill real distance in reply path)
    const char* response = "PONG";
    Serial.print(F("Sending response: '")); Serial.print(response); Serial.println(F("'"));
    uwb.tagSendData(strlen(response), response);
    Serial.println(F("---------------------------------"));
}

void setup() {
    Serial.begin(115200);
    // while (!Serial) { delay(10); }
    delay(1000);
    Serial.println(F("RYUW122 TAG (esp32devkitv4)"));

    if (!uwb.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }

    // Configure module settings
    uwb.setMode(RYUW122Mode::TAG);
    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(TAG_ADDRESS);

    // Register the callback for incoming data
    uwb.onTagReceive(onTagDataReceived);

    Serial.println(F("Tag configured and listening for messages."));
}

void loop() {
    uwb.loop();
    delay(10);
}

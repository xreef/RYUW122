/**
 * @file tag_arduino_mega_serial.ino
 * @author Renzo Mischianti
 * @brief Unified example for a TAG to respond to an Anchor.
 * @version 1.0.0
 * @date 2025-10-10
 *
 * This sketch configures the module as a Tag. It listens for incoming data
 * from an Anchor and sends a response back. This single Tag sketch works
 * seamlessly with both the Asynchronous and Synchronous methods in the
 * corresponding Anchor sketch.
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <RYUW122.h>

// --- Configuration ---
const char* NETWORK_ID = "AABBCCDD";
const char* TAG_ADDRESS = "T2T2T2T2"; // unique address for this Mega tag

// ------------------ ARDUINO UNO SOFTWARE SERIAL ------------------
// Define pins for SoftwareSerial (adjust based on your board)
// #define RX_PIN 10  // Connect to RYUW122 TX
// #define TX_PIN 11  // Connect to RYUW122 RX
//
// // Create RYUW122 instance with SoftwareSerial for Arduino UNO
// RYUW122 uwb(TX_PIN, RX_PIN, RYUW122BaudRate::B_9600);
// -----------------------------------------------------------------
// -------------------------- ARDUINO MEGA -------------------------
RYUW122 ryuw122(&Serial1);
// -----------------------------------------------------------------
// ------------------------ ESP32 ----------------------------------
// --- Configuration ---
// #define RX_PIN 9  // Connect to RYUW122 TX
// #define TX_PIN 10  // Connect to RYUW122 RX
// #define RESET_PIN 13 // Connect to RYUW122 NRST (active LOW)
//
// #define RX_PIN 5  // Connect to RYUW122 TX
// #define TX_PIN 4  // Connect to RYUW122 RX
// #define RESET_PIN 6 // Connect to RYUW122 NRST (active LOW)
// //
// RYUW122 ryuw122(TX_PIN, RX_PIN, &Serial1, RESET_PIN);
// -----------------------------------------------------------------

void onTagDataReceived(int payloadLength, const char* data, int rssi) {
    Serial.println(F("\n--- Data Received from Anchor ---"));
    Serial.print(F("Data: ")); Serial.println(data);
    Serial.print(F("RSSI: ")); Serial.print(rssi); Serial.println(F(" dBm"));

    const char* response = "PONG";
    ryuw122.tagSendData(strlen(response), response);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    Serial.println(F("RYUW122 TAG (Arduino Mega)"));

    if (!ryuw122.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }

    // Configure module settings
    ryuw122.setMode(RYUW122Mode::TAG);
    ryuw122.setNetworkId(NETWORK_ID);
    ryuw122.setAddress(TAG_ADDRESS);

    // Register the callback for incoming data
    ryuw122.onTagReceive(onTagDataReceived);

    Serial.println(F("Tag configured and listening for messages."));
}

void loop() {
    ryuw122.loop();
    delay(10);
}

/**
 * @file basic_tag.ino
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
const char* TAG_ADDRESS = "T1T1T1T1";

// ------------------ ARDUINO UNO SOFTWARE SERIAL ------------------
// Define pins for SoftwareSerial (adjust based on your board)
// #define RX_PIN 10  // Connect to RYUW122 TX
// #define TX_PIN 11  // Connect to RYUW122 RX
//
// // Create RYUW122 instance with SoftwareSerial for Arduino UNO
// RYUW122 uwb(TX_PIN, RX_PIN, RYUW122BaudRate::B_9600);
// -----------------------------------------------------------------
// -------------------------- ARDUINO MEGA -------------------------
// RYUW122 ryuw122(&Serial1);
// -----------------------------------------------------------------
// ------------------------ ESP32 ----------------------------------
#define RX_PIN 5  // Connect to RYUW122 TX
#define TX_PIN 4  // Connect to RYUW122 RX
#define RESET_PIN 6 // Connect to RYUW122 NRST (active LOW)

RYUW122 ryuw122(RX_PIN, TX_PIN, &Serial1, RESET_PIN);
// -----------------------------------------------------------------

/**
 * @brief Callback function to handle data received from an Anchor.
 * This is the standard way for a Tag to operate. It automatically
 * processes incoming requests without blocking the main loop.
 */
void onTagDataReceived(int payloadLength, const char* data, int rssi) {
    Serial.println(F("\n--- Data Received from Anchor ---"));
    Serial.print(F("Data: ")); Serial.println(data);
    Serial.print(F("RSSI: ")); Serial.print(rssi); Serial.println(F(" dBm"));

    // Prepare and send a response back to the Anchor
    const char* response = "PONG";
    Serial.print(F("Sending response: '")); Serial.print(response); Serial.println(F("'"));
    ryuw122.tagSendData(strlen(response), response);
    Serial.println(F("---------------------------------"));
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(100); }
    Serial.println(F("RYUW122 Unified Tag Example"));

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
    // The library's loop() function processes incoming serial data.
    // When a message from an Anchor arrives, it will trigger the
    // `onTagDataReceived` callback automatically.
    ryuw122.loop();

    // A small delay is good practice to prevent the loop from spinning
    // too fast, which can be inefficient on some microcontrollers.
    delay(10);
}

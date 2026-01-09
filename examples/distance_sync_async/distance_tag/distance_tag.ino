/**
 * @file distance_tag.ino
 * @author Renzo Mischianti
 * @brief Example of a TAG for the distance measurement example.
 * @version 1.0.0
 * @date 2025-10-10
 *
 * This sketch configures the module as a Tag. It requires minimal code as the
 * library automatically handles responding to distance measurement requests
 * from an Anchor in the background.
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <RYUW122.h>

// --- Configuration ---
const char* NETWORK_ID = "AABBCCDD";
// *** IMPORTANT: Change this address for each Tag! ***
// Use "T1T1T1T1" for the first tag, "T2T2T2T2" for the second, etc.
// This must match one of the addresses in the Anchor's list.
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
// RYUW122 uwb(&Serial1);
// -----------------------------------------------------------------
// ------------------------ ESP32 ----------------------------------
// --- Configuration ---
// #define RX_PIN 18  // Connect to RYUW122 TX
// #define TX_PIN 17  // Connect to RYUW122 RX
// #define RESET_PIN 13 // Connect to RYUW122 NRST (active LOW)
//
// #define RX_PIN 9  // Connect to RYUW122 TX
// #define TX_PIN 10  // Connect to RYUW122 RX
// #define RESET_PIN 13 // Connect to RYUW122 NRST (active LOW)
//
#define RX_PIN 5  // Connect to RYUW122 TX
#define TX_PIN 4  // Connect to RYUW122 RX
#define RESET_PIN 6 // Connect to RYUW122 NRST (active LOW)
// //
RYUW122 uwb(TX_PIN, RX_PIN, &Serial1, RESET_PIN);
// -----------------------------------------------------------------

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(100); }
    Serial.println(F("RYUW122 Simple Distance Tag Example"));

    if (!uwb.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }

    // Configure module settings
    uwb.setMode(RYUW122Mode::TAG);
    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(TAG_ADDRESS);

    Serial.print(F("Tag configured with address "));
    Serial.println(TAG_ADDRESS);
    Serial.println(F("Ready to respond to distance requests."));
}

void loop() {
    // The library's loop() function automatically handles responding to
    // distance requests from an Anchor. We just need to call it repeatedly.
    uwb.loop();
    delay(10);
}

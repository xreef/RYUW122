/**
 * @file 02_Distance_Tag.ino
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
#define RX_PIN 16
#define TX_PIN 17
#define RESET_PIN 4

const char* NETWORK_ID = "AABBCCDD";
// *** IMPORTANT: Change this address for each Tag! ***
// Use "T1T1T1T1" for the first tag, "T2T2T2T2" for the second, etc.
// This must match one of the addresses in the Anchor's list.
const char* TAG_ADDRESS = "T1T1T1T1";

RYUW122 ryuw122(&Serial1, RESET_PIN);

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(100); }
    Serial.println(F("RYUW122 Simple Distance Tag Example"));

    if (!ryuw122.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }

    // Configure module settings
    ryuw122.setMode(RYUW122Mode::TAG);
    ryuw122.setNetworkId(NETWORK_ID);
    ryuw122.setAddress(TAG_ADDRESS);

    Serial.print(F("Tag configured with address "));
    Serial.println(TAG_ADDRESS);
    Serial.println(F("Ready to respond to distance requests."));
}

void loop() {
    // The library's loop() function automatically handles responding to
    // distance requests from an Anchor. We just need to call it repeatedly.
    ryuw122.loop();
    delay(10);
}

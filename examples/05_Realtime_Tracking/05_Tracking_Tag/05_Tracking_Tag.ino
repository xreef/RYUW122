/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file 05_Tracking_Tag.ino
 * @brief TAG sketch for the Real-Time Tracking example.
 * @version 1.0.0
 * @date 2025-10-10
 *
 * This sketch configures the module as a mobile Tag. Its only job is to
 * be "visible" to the Anchors so they can measure its distance. It requires
 * minimal code as the library handles all responses automatically.
 *
 */

#include <RYUW122.h>

// ========================================
// CONFIGURATION
// ========================================

// UWB module connection (adjust for your board)
#define RX_PIN 16
#define TX_PIN 17
#define RESET_PIN 4

RYUW122 ryuw122(&Serial1, RESET_PIN);

// Network and Device Addresses
const char* NETWORK_ID = "TRACKSYS"; // Must match the network ID of the anchors
const char* TAG_ADDRESS = "TAG_TRACK1"; // The address the Master Anchor is looking for

// ========================================
// SETUP & MAIN LOOP
// ========================================

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println(F("\n================================================"));
    Serial.println(F("  RYUW122 Real-Time Tracking - TAG"));
    Serial.println(F("================================================"));

    if (!ryuw122.begin()) {
        Serial.println(F("FATAL: Failed to initialize RYUW122 module!"));
        while(1);
    }

    // Configure this device as a TAG
    ryuw122.setMode(RYUW122Mode::TAG);
    ryuw122.setNetworkId(NETWORK_ID);
    ryuw122.setAddress(TAG_ADDRESS);

    Serial.print(F("Tag configured with address: "));
    Serial.println(TAG_ADDRESS);
    Serial.println(F("Ready to be tracked by the Anchor system."));
}

void loop() {
    // The library's loop function automatically handles responding to
    // distance requests from the Master Anchor. We just need to call it
    // repeatedly to process incoming messages.
    ryuw122.loop();

    // A small delay is good practice to prevent the loop from spinning
    // too fast, which can be inefficient on some microcontrollers.
    delay(10);
}

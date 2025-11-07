/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file RYUW122_Simple_Distance.ino
 * @brief Simple example using high-level API to measure distance
 *
 * This example demonstrates the simplified API for distance measurement.
 * An ANCHOR continuously measures distance to a TAG and displays it.
 *
 * Hardware connections:
 * - RYUW122 TX  -> Arduino RX (Pin 10)
 * - RYUW122 RX  -> Arduino TX (Pin 11)
 * - RYUW122 GND -> Arduino GND
 * - RYUW122 VCC -> 3.3V or 5V
 *
 * @note This module should be configured as ANCHOR
 * @note A TAG with address "TAG00001" should be in range
 */

#include <RYUW122.h>

// Create UWB instance
RYUW122 uwb(11, 10, RYUW122BaudRate::B_115200); // TX=11, RX=10

// Configuration
const char* TARGET_TAG = "TAG00001";
const char* NETWORK_ID = "REYAX123";
const char* ANCHOR_ADDR = "ANCHOR01";

// Measurement interval
unsigned long lastMeasurement = 0;
const unsigned long MEASURE_INTERVAL = 1000; // Measure every 1 second

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("\n=== RYUW122 Simple Distance Measurement ===");

    // Initialize module
    Serial.println("Initializing...");
    if (!uwb.begin()) {
        Serial.println("ERROR: Failed to initialize!");
        while(1);
    }

    // Configure as ANCHOR
    Serial.println("Configuring as ANCHOR...");
    uwb.setMode(RYUW122Mode::ANCHOR);
    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(ANCHOR_ADDR);
    uwb.setRssiDisplay(RYUW122RSSI::ENABLE);

    Serial.println("\n=== Ready to measure distance ===");
    Serial.print("Target TAG: ");
    Serial.println(TARGET_TAG);
    Serial.println("Measuring distance every 1 second...\n");
}

void loop() {
    unsigned long currentTime = millis();

    if (currentTime - lastMeasurement >= MEASURE_INTERVAL) {
        lastMeasurement = currentTime;

        Serial.print("Measuring distance... ");

        // Simplified API - just get the distance!
        float distanceCm = uwb.getDistanceFrom(TARGET_TAG);

        if (distanceCm >= 0) {
            // Success - display in multiple units
            Serial.print("Distance: ");
            Serial.print(distanceCm);
            Serial.print(" cm");

            // You can also get it in other units directly:
            float distanceInches = uwb.getDistanceFrom(TARGET_TAG, MeasureUnit::INCHES);
            if (distanceInches >= 0) {
                Serial.print(" (");
                Serial.print(distanceInches, 1);
                Serial.print(" inches)");
            }

            float distanceMeters = uwb.getDistanceFrom(TARGET_TAG, MeasureUnit::METERS);
            if (distanceMeters >= 0) {
                Serial.print(" (");
                Serial.print(distanceMeters, 2);
                Serial.print(" m)");
            }

            Serial.println();
        } else {
            Serial.println("FAILED - TAG not responding");
        }
    }
}


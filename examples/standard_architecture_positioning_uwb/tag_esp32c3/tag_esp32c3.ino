/**
 * @file 04_Tag_esp32c3.ino
 * @author Renzo Mischianti
 * @brief UWB Tag for ESP32-C3, designed for a trilateration system.
 * @version 1.0.0
 * @date 2024-05-23
 *
 * This sketch configures an ESP32-C3 as a passive UWB Tag.
 * More info and reference projects at https://mischianti.org/category/my-libraries/ryuw122-uwb/
 *
 * Its role is to be discoverable by anchors. It listens for polling
 * messages and automatically responds. The RYUW122 module handles
 * the response internally, allowing anchors to calculate their distance.
 *
 * This device does not require network connectivity.
 *
 * @copyright Copyright (c) 2024 Renzo Mischianti - www.mischianti.org
 *
 */

#include <RYUW122.h>

// --- UWB Configuration ---
#define RX_PIN 5      // Connect to RYUW122 TX
#define TX_PIN 4      // Connect to RYUW122 RX
#define RESET_PIN 6   // Connect to RYUW122 NRST (active LOW)

RYUW122 uwb(TX_PIN, RX_PIN, &Serial1, RESET_PIN);

const char* NETWORK_ID = "AABBCCDD";
const char* TAG_ADDRESS = "T1T1T1T1"; // This Tag's unique address

/**
 * @brief Callback for when the Tag receives data from an Anchor.
 * The library handles the response automatically for distance calculation.
 * This callback can be used to know WHEN the Tag was polled.
 */
void onTagDataReceived(int payloadLength, const char* data, int rssi) {
    Serial.println(F("\n--- Polled by an Anchor ---"));
    Serial.print(F("Received Data: ")); Serial.println(data);
    Serial.print(F("RSSI: ")); Serial.print(rssi); Serial.println(F(" dBm"));
    Serial.println(F("---------------------------"));

    // No need to call tagSendData() here. The module's firmware
    // automatically replies to the anchor's distance request.
}

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println(F("RYUW122 Distance-Based TAG (esp32c3)"));

    if (!uwb.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }

    uwb.setMode(RYUW122Mode::TAG);
    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(TAG_ADDRESS);

    // Register a callback to be notified when an anchor polls us.
    uwb.onTagReceive(onTagDataReceived);

    Serial.println(F("Tag configured and listening for Anchor polls."));
}

void loop() {
    // The loop() function processes incoming commands from anchors.
    // When an anchor sends a distance request, the library and firmware
    // handle the response automatically.
    uwb.loop();

    // A small delay can be added to be power-efficient.
    delay(5);
}

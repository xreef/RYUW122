/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file RYUW122_Indoor_Positioning_TAG.ino
 * @brief TAG counterpart for indoor positioning example.
 *
 * The TAG replies to ANCHOR requests and stores a small payload that anchors
 * can request to perform ranging (used by the anchor-side trilateration sketch).
 */

#include <RYUW122.h>

RYUW122 uwb(11, 10, RYUW122BaudRate::B_115200);

const char* NETWORK_ID = "REYAX123";
const char* TAG_ADDRESS = "TAG00001";

unsigned long lastStore = 0;
const unsigned long STORE_INTERVAL = 1500; // store message every 1.5s
int seq = 0;

// Uncomment to enable TAG receive callback (useful to log when ANCHOR sends commands)
// void onTagReceive(int payloadLength, const char* data, int rssi) {
//     Serial.println("\n*** TAG received command from ANCHOR ***");
//     Serial.print("Payload Length: "); Serial.println(payloadLength);
//     Serial.print("Data: "); Serial.println(data);
//     Serial.print("RSSI: "); Serial.println(rssi);
//     Serial.println("*************************************\n");
// }

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("\n=== RYUW122 TAG for Indoor Positioning ===");

    if (!uwb.begin()) {
        Serial.println("ERROR: UWB init failed");
        while(1);
    }

    uwb.setMode(RYUW122Mode::TAG);
    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(TAG_ADDRESS);
    uwb.setRssiDisplay(RYUW122RSSI::ENABLE);

    // Register callback
    // (disabled by default - uncomment to enable)
    // uwb.onTagReceive(onTagReceive);

    Serial.print("TAG address: "); Serial.println(TAG_ADDRESS);
}

void loop() {
    uwb.processMessages();

    unsigned long now = millis();
    if (now - lastStore >= STORE_INTERVAL) {
        lastStore = now;
        char payload[RYUW122_MAX_PAYLOAD_LENGTH + 1];
        snprintf(payload, sizeof(payload), "P%03d", seq++);
        Serial.print("Storing payload in TAG: "); Serial.println(payload);
        if (!uwb.sendMessageFromTag(payload)) {
            Serial.println("Failed to store payload");
        }
    }

    delay(10);
}


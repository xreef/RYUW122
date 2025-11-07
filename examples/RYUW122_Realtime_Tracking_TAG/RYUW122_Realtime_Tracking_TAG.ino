/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file RYUW122_Realtime_Tracking_TAG.ino
 * @brief TAG counterpart for realtime tracking example.
 *
 * The TAG periodically updates its stored payload. The coordinator ANCHOR polls
 * for distances to each anchor to obtain the TAG position.
 */

#include <RYUW122.h>

RYUW122 uwb(11, 10, RYUW122BaudRate::B_115200);

const char* NETWORK_ID = "REYAX123";
const char* TAG_ADDRESS = "TAG00001";

unsigned long lastStore = 0;
const unsigned long STORE_INTERVAL = 1000; // store every 1s
int counter = 0;

// Uncomment to enable TAG receive callback (useful to log when ANCHOR sends commands)
// void onTagReceive(int payloadLength, const char* data, int rssi) {
//     Serial.println("\n--- TAG received command from ANCHOR ---");
//     Serial.print("Payload Length: "); Serial.println(payloadLength);
//     Serial.print("Data: "); Serial.println(data);
//     Serial.print("RSSI: "); Serial.println(rssi);
//     Serial.println("--------------------------------------\n");
// }

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println("\n=== RYUW122 TAG for Realtime Tracking ===");

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
        snprintf(payload, sizeof(payload), "R%03d", counter++);
        Serial.print("Storing payload in TAG: "); Serial.println(payload);
        if (!uwb.sendMessageFromTag(payload)) {
            Serial.println("Failed to store payload");
        }
    }

    delay(10);
}


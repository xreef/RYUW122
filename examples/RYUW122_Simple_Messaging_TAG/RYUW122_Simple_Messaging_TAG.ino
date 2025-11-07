/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file RYUW122_Simple_Messaging_TAG.ino
 * @brief TAG counterpart of the Simple Messaging example.
 *
 * TAG will periodically store messages which ANCHOR can request.
 */

#include <RYUW122.h>

RYUW122 uwb(11, 10, RYUW122BaudRate::B_115200);

const char* NETWORK_ID = "REYAX123";
const char* TAG_ADDRESS = "TAG00001";

unsigned long lastStore = 0;
const unsigned long STORE_INTERVAL = 3000;
int ctr = 0;

// Uncomment to enable TAG receive callback
// void onTagReceive(int payloadLength, const char* data, int rssi) {
//     Serial.println();
//     Serial.println("### TAG received message from ANCHOR ###");
//     Serial.print("Payload Length: "); Serial.println(payloadLength);
//     Serial.print("Message: "); Serial.println(data);
//     Serial.print("RSSI: "); Serial.println(rssi);
//     Serial.println("######################################");
// }

void setup() {
    Serial.begin(115200);
    while (!Serial);

    if (!uwb.begin()) {
        Serial.println("ERROR: failed to init");
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
        char msg[RYUW122_MAX_PAYLOAD_LENGTH + 1];
        snprintf(msg, sizeof(msg), "TG%03d", ctr++);
        Serial.print("Storing TAG message: "); Serial.println(msg);
        if (!uwb.sendMessageFromTag(msg)) {
            Serial.println("Failed to store TAG message");
        }
    }
    delay(10);
}


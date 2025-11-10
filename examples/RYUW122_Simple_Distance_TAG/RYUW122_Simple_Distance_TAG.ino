/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file RYUW122_Simple_Distance_TAG.ino
 * @brief TAG counter-part for Simple Distance example.
 *
 * The TAG stores a small message periodically so an ANCHOR can request it
 * and calculate distance using AT+ANCHOR_SEND.
 */

#include <RYUW122.h>

#define RESET_PIN 6 // NRST active LOW

// Create UWB instance
RYUW122 uwb(11, 10, RESET_PIN, RYUW122BaudRate::B_115200); // TX=11, RX=10

const char* NETWORK_ID = "REYAX123";
const char* TAG_ADDRESS = "TAG00001";

unsigned long lastStore = 0;
const unsigned long STORE_INTERVAL = 2000; // store message every 2 seconds
int counter = 0;

// Uncomment to enable TAG receive callback (useful to log when ANCHOR sends commands)
// void onTagReceive(int payloadLength, const char* data, int rssi) {
//     Serial.println(F("\n--- TAG received command from ANCHOR ---"));
//     Serial.print(F("Payload Length: ")); Serial.println(payloadLength);
//     Serial.print(F("Data: ")); Serial.println(data);
//     Serial.print(F("RSSI: ")); Serial.println(rssi);
//     Serial.println(F("--------------------------------------\n"));
// }

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println(F("\n=== RYUW122 Simple Distance TAG ==="));

    if (!uwb.begin()) {
        Serial.println(F("ERROR: Failed to init UWB module"));
        while(1);
    }

    uwb.setMode(RYUW122Mode::TAG);
    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(TAG_ADDRESS);
    uwb.setRssiDisplay(RYUW122RSSI::ENABLE);

    // Register callback to be notified when ANCHOR sends data to this TAG
    // (disabled by default - uncomment to enable)
    // uwb.onTagReceive(onTagReceive);

    Serial.print(F("TAG address: ")); Serial.println(TAG_ADDRESS);
    Serial.println(F("Storing small message periodically for ANCHOR to request"));
}

void loop() {
    uwb.processMessages();

    unsigned long now = millis();
    if (now - lastStore >= STORE_INTERVAL) {
        lastStore = now;
        char payload[RYUW122_MAX_PAYLOAD_LENGTH + 1];
        snprintf(payload, sizeof(payload), "T%04d", counter++);
        Serial.print(F("Storing message in TAG: ")); Serial.println(payload);
        bool ok = uwb.sendMessageFromTag(payload);
        if (!ok) {
            Serial.println(F("Failed to store message in TAG"));
        }
    }

    delay(10);
}

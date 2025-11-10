/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file RYUW122_Basic_Test_TAG.ino
 * @brief TAG counterpart for the Basic Test example.
 *
 * This sketch performs a minimal set of operations from the TAG perspective
 * and stores a few test messages to verify communication.
 */

#include <RYUW122.h>

#define RESET_PIN 6 // NRST active LOW

// Create RYUW122 instance with SoftwareSerial
RYUW122 uwb(11, 10, RESET_PIN, RYUW122BaudRate::B_115200);

const char* NETWORK_ID = "REYAX123";
const char* TAG_ADDRESS = "TAG00001";

// Optional: TAG receive callback (uncomment if you want to log incoming commands)
// void onTagReceive(int payloadLength, const char* data, int rssi) {
//     Serial.print(F("TAG received: ")); Serial.println(data);
// }

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println(F("\n=== RYUW122 Basic Test TAG ==="));

    if (!uwb.begin()) {
        Serial.println(F("Failed to init UWB"));
        while(1);
    }

    if (!uwb.test()) {
        Serial.println(F("Module not responding to AT"));
        while(1);
    }

    uwb.setMode(RYUW122Mode::TAG);
    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(TAG_ADDRESS);
    uwb.setPassword("00000000000000000000000000000000");
    uwb.setRssiDisplay(RYUW122RSSI::ENABLE);

    // Register tag receive callback (disabled by default)
    // uwb.onTagReceive(onTagReceive);

    Serial.println(F("TAG ready"));
}

void loop() {
    // store some messages for anchors to read
    static unsigned long last = 0;
    static int n = 0;
    if (millis() - last > 2000) {
        last = millis();
        char buf[RYUW122_MAX_PAYLOAD_LENGTH + 1];
        snprintf(buf, sizeof(buf), "B%03d", n++);
        Serial.print(F("Store test message: ")); Serial.println(buf);
        bool ok = uwb.sendMessageFromTag(buf);
        if (!ok) Serial.println(F("Store failed"));
    }
    uwb.processMessages();
}

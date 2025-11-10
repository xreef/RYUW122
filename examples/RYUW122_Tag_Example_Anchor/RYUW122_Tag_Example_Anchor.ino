/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

// Anchor counterpart created earlier; copied here to match new folder structure

#include <RYUW122.h>

#define RESET_PIN 6 // NRST active LOW

// Define pins for SoftwareSerial (adjust based on your board)
#define RX_PIN 10  // Connect to RYUW122 TX
#define TX_PIN 11  // Connect to RYUW122 RX

// Create RYUW122 instance
RYUW122 uwb(TX_PIN, RX_PIN, RESET_PIN, RYUW122BaudRate::B_115200);

// Configuration parameters
const char* NETWORK_ID = "REYAX123";
const char* ANCHOR_ADDRESS = "ANCHOR01";
const char* TAG_ADDRESS = "DAVID123"; // must match TAG example
const char* PASSWORD = "00000000000000000000000000000000"; // Default password

// Timing variables
unsigned long lastSyncSend = 0;
unsigned long lastAsyncSend = 0;
const unsigned long SYNC_INTERVAL = 7000;   // Send synchronously every 7 seconds
const unsigned long ASYNC_INTERVAL = 4000;  // Send asynchronously every 4 seconds

// Callback when ANCHOR receives from TAG (async)
void onAnchorReceive(const char* tagAddress, int payloadLength, const char* tagData, int distance, int rssi) {
    Serial.println(F("=== ANCHOR RECEIVED (Async) ==="));
    Serial.print(F("From TAG: ")); Serial.println(tagAddress);
    Serial.print(F("Payload Length: ")); Serial.println(payloadLength);
    Serial.print(F("Data: ")); Serial.println(tagData);
    Serial.print(F("Distance: ")); Serial.print(distance); Serial.println(F(" cm"));
    Serial.print(F("RSSI: ")); Serial.println(rssi);
    Serial.println(F("=============================="));
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { ; }

    Serial.println(F("\n=== RYUW122 ANCHOR (for TAG example) ==="));

    if (!uwb.begin()) {
        Serial.println(F("Failed to initialize RYUW122!"));
        while (1) delay(1000);
    }

    if (!uwb.test()) {
        Serial.println(F("AT communication test failed!"));
        while (1) delay(1000);
    }

    uwb.setMode(RYUW122Mode::ANCHOR);
    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(ANCHOR_ADDRESS);
    uwb.setPassword(PASSWORD);
    uwb.setRssiDisplay(RYUW122RSSI::ENABLE);

    // Register callback
    uwb.onAnchorReceive(onAnchorReceive);

    Serial.print(F("Anchor address: ")); Serial.println(ANCHOR_ADDRESS);
    Serial.print(F("Target TAG: ")); Serial.println(TAG_ADDRESS);
    Serial.println(F("Setup complete. Sending commands to TAG periodically..."));
}

void loop() {
    uwb.processMessages();

    unsigned long currentTime = millis();

    // Synchronous send example - blocks until TAG responds
    if (currentTime - lastSyncSend >= SYNC_INTERVAL) {
        lastSyncSend = currentTime;

        char sendData[RYUW122_MAX_PAYLOAD_LENGTH + 1];
        snprintf(sendData, sizeof(sendData), "S%04lu", currentTime / 1000);

        Serial.print(F("[SYNC] Sending to TAG ")); Serial.print(TAG_ADDRESS); Serial.print(F(": ")); Serial.println(sendData);

        char responseData[RYUW122_MAX_PAYLOAD_LENGTH + 1];
        int distance = 0;
        int rssi = 0;

        bool ok = uwb.anchorSendDataSync(TAG_ADDRESS, strlen(sendData), sendData, responseData, &distance, &rssi, 2000);
        if (ok) {
            Serial.println(F("[SYNC] Response:"));
            Serial.print(F("  Data: ")); Serial.println(responseData);
            Serial.print(F("  Distance: ")); Serial.print(distance); Serial.println(F(" cm"));
            Serial.print(F("  RSSI: ")); Serial.println(rssi);
        } else {
            Serial.println(F("[SYNC] No response or timeout"));
        }
    }

    // Asynchronous send example - response handled by callback
    if (currentTime - lastAsyncSend >= ASYNC_INTERVAL) {
        lastAsyncSend = currentTime;

        const char* msg = "HELLO";
        Serial.print(F("[ASYNC] Sending to TAG ")); Serial.print(TAG_ADDRESS); Serial.print(F(": ")); Serial.println(msg);
        bool sent = uwb.anchorSendData(TAG_ADDRESS, strlen(msg), msg);
        if (!sent) Serial.println(F("[ASYNC] Send failed"));
    }

    delay(10);
}

/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file RYUW122_Anchor_Example.ino
 * @brief Example demonstrating ANCHOR mode with both synchronous and asynchronous communication
 *
 * This example shows how to:
 * - Configure the RYUW122 module as an ANCHOR
 * - Send data to a TAG synchronously (blocking) and receive distance
 * - Send data to a TAG asynchronously (non-blocking) with callback
 * - Handle incoming TAG responses with callbacks
 *
 * Hardware connections:
 * - RYUW122 TX  -> Arduino RX (e.g., Pin 10 for SoftwareSerial)
 * - RYUW122 RX  -> Arduino TX (e.g., Pin 11 for SoftwareSerial)
 * - RYUW122 GND -> Arduino GND
 * - RYUW122 VCC -> Arduino 3.3V or 5V (check module specifications)
 *
 * @note Ensure the TAG is configured with matching NETWORKID and PASSWORD
 * @note The TAG address used in this example is "DAVID123"
 */

#include <RYUW122.h>

// Define pins for SoftwareSerial (adjust based on your board)
#define RX_PIN 10  // Connect to RYUW122 TX
#define TX_PIN 11  // Connect to RYUW122 RX

// Create RYUW122 instance with SoftwareSerial
RYUW122 uwb(TX_PIN, RX_PIN, RYUW122BaudRate::B_115200);

// Configuration parameters
const char* NETWORK_ID = "REYAX123";
const char* ANCHOR_ADDRESS = "REYAX001";
const char* TAG_ADDRESS = "DAVID123";
const char* PASSWORD = "00000000000000000000000000000000"; // Default password

// Timing variables
unsigned long lastSyncSend = 0;
unsigned long lastAsyncSend = 0;
const unsigned long SYNC_INTERVAL = 5000;   // Send synchronously every 5 seconds
const unsigned long ASYNC_INTERVAL = 3000;  // Send asynchronously every 3 seconds

/**
 * @brief Callback function for asynchronous ANCHOR receive events
 *
 * This callback is triggered when the ANCHOR receives data from a TAG
 * in response to an asynchronous send operation.
 *
 * @param tagAddress The address of the TAG that sent the data
 * @param payloadLength Length of the received payload
 * @param tagData The actual data received from the TAG
 * @param distance Calculated distance to the TAG in centimeters
 * @param rssi Received Signal Strength Indication
 */
void onAnchorReceive(const char* tagAddress, int payloadLength, const char* tagData, int distance, int rssi) {
    Serial.println("=== ANCHOR RECEIVED (Async) ===");
    Serial.print("From TAG: ");
    Serial.println(tagAddress);
    Serial.print("Payload Length: ");
    Serial.println(payloadLength);
    Serial.print("Data: ");
    Serial.println(tagData);
    Serial.print("Distance: ");
    Serial.print(distance);
    Serial.println(" cm");
    Serial.print("RSSI: ");
    Serial.println(rssi);
    Serial.println("==============================");
}

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect (needed for native USB)
    }

    Serial.println("RYUW122 ANCHOR Example");
    Serial.println("======================");

    // Initialize the UWB module
    Serial.println("Initializing RYUW122...");
    if (!uwb.begin()) {
        Serial.println("Failed to initialize RYUW122!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("RYUW122 initialized successfully");

    // Test AT command communication
    Serial.println("Testing AT communication...");
    if (!uwb.test()) {
        Serial.println("AT communication test failed!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("AT communication OK");

    // Configure as ANCHOR
    Serial.println("Setting mode to ANCHOR...");
    if (!uwb.setMode(RYUW122Mode::ANCHOR)) {
        Serial.println("Failed to set ANCHOR mode!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("ANCHOR mode set");

    // Set Network ID
    Serial.print("Setting Network ID to: ");
    Serial.println(NETWORK_ID);
    if (!uwb.setNetworkId(NETWORK_ID)) {
        Serial.println("Failed to set Network ID!");
    }

    // Set ANCHOR address
    Serial.print("Setting ANCHOR address to: ");
    Serial.println(ANCHOR_ADDRESS);
    if (!uwb.setAddress(ANCHOR_ADDRESS)) {
        Serial.println("Failed to set address!");
    }

    // Set encryption password
    Serial.println("Setting encryption password...");
    if (!uwb.setPassword(PASSWORD)) {
        Serial.println("Failed to set password!");
    }

    // Enable RSSI display
    Serial.println("Enabling RSSI display...");
    if (!uwb.setRssiDisplay(RYUW122RSSI::ENABLE)) {
        Serial.println("Failed to enable RSSI!");
    }

    // Register callback for asynchronous receives
    uwb.onAnchorReceive(onAnchorReceive);

    // Display firmware version
    char version[32];
    if (uwb.getFirmwareVersion(version)) {
        Serial.print("Firmware Version: ");
        Serial.println(version);
    }

    Serial.println("\n======================");
    Serial.println("Setup complete!");
    Serial.println("Waiting for TAG responses...");
    Serial.println("======================\n");

    delay(1000);
}

void loop() {
    // Process incoming messages (for asynchronous callbacks)
    uwb.loop();

    unsigned long currentTime = millis();

    // Synchronous send example - blocking call with immediate response
    if (currentTime - lastSyncSend >= SYNC_INTERVAL) {
        lastSyncSend = currentTime;

        Serial.println("\n--- Synchronous Send Example ---");

        const char* sendData = "SYNC";
        char responseData[RYUW122_MAX_PAYLOAD_LENGTH + 1];
        int distance = 0;
        int rssi = 0;

        Serial.print("Sending '");
        Serial.print(sendData);
        Serial.print("' to TAG ");
        Serial.println(TAG_ADDRESS);

        // This call blocks until response is received or timeout occurs
        bool success = uwb.anchorSendDataSync(
            TAG_ADDRESS,           // Target TAG address
            strlen(sendData),      // Payload length
            sendData,              // Data to send
            responseData,          // Buffer for response data
            &distance,             // Pointer to store distance
            &rssi,                 // Pointer to store RSSI
            2000                   // Timeout in milliseconds
        );

        if (success) {
            Serial.println("=== ANCHOR RECEIVED (Sync) ===");
            Serial.print("Response Data: ");
            Serial.println(responseData);
            Serial.print("Distance: ");
            Serial.print(distance);
            Serial.println(" cm");
            Serial.print("RSSI: ");
            Serial.println(rssi);
            Serial.println("==============================");
        } else {
            Serial.println("Synchronous send failed or timeout!");
        }
    }

    // Asynchronous send example - non-blocking call with callback response
    if (currentTime - lastAsyncSend >= ASYNC_INTERVAL) {
        lastAsyncSend = currentTime;

        Serial.println("\n--- Asynchronous Send Example ---");

        const char* sendData = "ASYNC";

        Serial.print("Sending '");
        Serial.print(sendData);
        Serial.print("' to TAG ");
        Serial.println(TAG_ADDRESS);

        // This call returns immediately after sending the command
        // Response will be handled by the onAnchorReceive callback
        bool success = uwb.anchorSendData(
            TAG_ADDRESS,           // Target TAG address
            strlen(sendData),      // Payload length
            sendData               // Data to send
        );

        if (success) {
            Serial.println("Command sent successfully (response via callback)");
        } else {
            Serial.println("Asynchronous send failed!");
        }
    }

    // Small delay to prevent overwhelming the serial output
    delay(10);
}

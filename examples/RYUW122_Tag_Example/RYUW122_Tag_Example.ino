/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file RYUW122_Tag_Example.ino
 * @brief Example demonstrating TAG mode with both synchronous and asynchronous communication
 *
 * This example shows how to:
 * - Configure the RYUW122 module as a TAG
 * - Send data from TAG synchronously (blocking) with confirmation
 * - Send data from TAG asynchronously (non-blocking) with callback
 * - Handle incoming ANCHOR requests with callbacks
 *
 * Hardware connections:
 * - RYUW122 TX  -> Arduino RX (e.g., Pin 10 for SoftwareSerial)
 * - RYUW122 RX  -> Arduino TX (e.g., Pin 11 for SoftwareSerial)
 * - RYUW122 GND -> Arduino GND
 * - RYUW122 VCC -> Arduino 3.3V or 5V (check module specifications)
 *
 * @note Ensure the ANCHOR is configured with matching NETWORKID and PASSWORD
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
const char* TAG_ADDRESS = "DAVID123";
const char* PASSWORD = "00000000000000000000000000000000"; // Default password

// Timing variables
unsigned long lastSyncSend = 0;
unsigned long lastAsyncSend = 0;
const unsigned long SYNC_INTERVAL = 7000;   // Send synchronously every 7 seconds
const unsigned long ASYNC_INTERVAL = 4000;  // Send asynchronously every 4 seconds

// Data counter for demonstration
int dataCounter = 0;

/**
 * @brief Callback function for asynchronous TAG receive events
 *
 * This callback is triggered when the TAG receives data from an ANCHOR.
 * The TAG stores data which is transmitted back to ANCHOR when requested.
 *
 * @param payloadLength Length of the received payload
 * @param data The actual data received from the ANCHOR
 * @param rssi Received Signal Strength Indication
 */
void onTagReceive(int payloadLength, const char* data, int rssi) {
    Serial.println("=== TAG RECEIVED ===");
    Serial.print("Payload Length: ");
    Serial.println(payloadLength);
    Serial.print("Data from ANCHOR: ");
    Serial.println(data);
    Serial.print("RSSI: ");
    Serial.println(rssi);
    Serial.println("====================");
}

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect (needed for native USB)
    }

    Serial.println("RYUW122 TAG Example");
    Serial.println("===================");

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

    // Configure as TAG
    Serial.println("Setting mode to TAG...");
    if (!uwb.setMode(RYUW122Mode::TAG)) {
        Serial.println("Failed to set TAG mode!");
        while (1) {
            delay(1000);
        }
    }
    Serial.println("TAG mode set");

    // Set Network ID
    Serial.print("Setting Network ID to: ");
    Serial.println(NETWORK_ID);
    if (!uwb.setNetworkId(NETWORK_ID)) {
        Serial.println("Failed to set Network ID!");
    }

    // Set TAG address
    Serial.print("Setting TAG address to: ");
    Serial.println(TAG_ADDRESS);
    if (!uwb.setAddress(TAG_ADDRESS)) {
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

    // Optional: Configure TAG RF duty cycle for power saving
    // Example: RF enabled for 1 second, disabled for 1 second
    // Uncomment the lines below to enable power saving mode
    /*
    Serial.println("Setting TAG RF duty cycle (1sec ON, 1sec OFF)...");
    if (!uwb.setTagRfDutyCycle(1000, 1000)) {
        Serial.println("Failed to set RF duty cycle!");
    } else {
        Serial.println("RF duty cycle configured");
    }
    */

    // Register callback for receiving data from ANCHOR
    uwb.onTagReceive(onTagReceive);

    // Display firmware version
    char version[32];
    if (uwb.getFirmwareVersion(version)) {
        Serial.print("Firmware Version: ");
        Serial.println(version);
    }

    Serial.println("\n===================");
    Serial.println("Setup complete!");
    Serial.println("Waiting for ANCHOR requests...");
    Serial.println("===================\n");

    delay(1000);
}

void loop() {
    // Process incoming messages (for callbacks)
    uwb.loop();

    unsigned long currentTime = millis();

    // Synchronous send example - blocking call with confirmation
    if (currentTime - lastSyncSend >= SYNC_INTERVAL) {
        lastSyncSend = currentTime;

        Serial.println("\n--- Synchronous Send Example ---");

        char sendData[RYUW122_MAX_PAYLOAD_LENGTH + 1];
        snprintf(sendData, sizeof(sendData), "S%04d", dataCounter++);

        Serial.print("Sending '");
        Serial.print(sendData);
        Serial.println("' to TAG buffer (sync)");

        // This call blocks until +OK response is received or timeout occurs
        // The data is stored in the TAG and will be transmitted when ANCHOR requests it
        bool success = uwb.tagSendDataSync(
            strlen(sendData),      // Payload length
            sendData,              // Data to send
            1000                   // Timeout in milliseconds
        );

        if (success) {
            Serial.println("Data stored in TAG successfully (sync)");
            Serial.println("Waiting for ANCHOR to request this data...");
        } else {
            Serial.println("Synchronous send failed or timeout!");
        }
    }

    // Asynchronous send example - non-blocking call
    if (currentTime - lastAsyncSend >= ASYNC_INTERVAL) {
        lastAsyncSend = currentTime;

        Serial.println("\n--- Asynchronous Send Example ---");

        char sendData[RYUW122_MAX_PAYLOAD_LENGTH + 1];
        snprintf(sendData, sizeof(sendData), "A%04d", dataCounter++);

        Serial.print("Sending '");
        Serial.print(sendData);
        Serial.println("' to TAG buffer (async)");

        // This call returns immediately after sending the command
        // The data is stored in the TAG and will be transmitted when ANCHOR requests it
        bool success = uwb.tagSendData(
            strlen(sendData),      // Payload length
            sendData               // Data to send
        );

        if (success) {
            Serial.println("Command sent successfully (async)");
            Serial.println("Data will be transmitted when ANCHOR requests it");
        } else {
            Serial.println("Asynchronous send failed!");
        }
    }

    // Small delay to prevent overwhelming the serial output
    delay(10);
}


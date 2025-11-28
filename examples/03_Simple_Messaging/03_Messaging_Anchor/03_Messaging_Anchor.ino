/**
 * @file 03_Messaging_Anchor.ino
 * @author Renzo Mischianti
 * @brief Unified example for an ANCHOR to send and receive text messages, with both Sync and Async methods.
 * @version 1.0.0
 * @date 2025-10-10
 *
 * This sketch allows you to send messages from the Serial Monitor to a Tag.
 * It demonstrates two ways to send the message.
 *
 * **Instructions:** When a message is entered, choose which method to use by uncommenting the desired line.
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <RYUW122.h>

// --- Configuration ---
#define RX_PIN 16
#define TX_PIN 17
#define RESET_PIN 4

const char* NETWORK_ID = "AABBCCDD";
const char* ANCHOR_ADDRESS = "A1A1A1A1";
const char* TAG_ADDRESS = "T1T1T1T1";

RYUW122 ryuw122(&Serial1, RESET_PIN);

/**
 * @brief Callback for receiving messages from the Tag.
 * This function is triggered for both sync and async responses.
 */
void onMessageReceived(const char* tagAddress, int payloadLength, const char* data, int distance, int rssi) {
    Serial.println(F("\n--- Message Received from Tag (Callback) ---"));
    Serial.print(F("From: ")); Serial.println(tagAddress);
    Serial.print(F("Message: ")); Serial.println(data);
    Serial.print(F("RSSI: ")); Serial.println(rssi);
    Serial.println(F("------------------------------------------"));
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(100); }
    Serial.println(F("RYUW122 Simple Messaging Anchor Example"));
    Serial.println(F("Type a message and press Enter to send it to the Tag."));

    if (!ryuw122.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }

    ryuw122.setMode(RYUW122Mode::ANCHOR);
    ryuw122.setNetworkId(NETWORK_ID);
    ryuw122.setAddress(ANCHOR_ADDRESS);

    // Register the callback for incoming messages
    ryuw122.onAnchorReceive(onMessageReceived);
}

void loop() {
    // Check for new messages from the module (triggers the callback)
    ryuw122.loop();

    // Check for user input from the Serial Monitor
    if (Serial.available()) {
        String message = Serial.readStringUntil('\n');
        message.trim();

        if (message.length() > 0) {
            Serial.print(F("\nSending message: '"));
            Serial.print(message);
            Serial.println(F("'..."));

            // =======================================================================
            // === CHOOSE YOUR SEND METHOD: Uncomment the desired section          ===
            // =======================================================================

            // --- METHOD 1: Asynchronous (Non-Blocking) Send ---
            // The function returns immediately. The response will be caught by the callback.
            ryuw122.anchorSendData(TAG_ADDRESS, message.length(), message.c_str());
            Serial.println(F("Message sent (Async). The main loop was not blocked."));
            Serial.println(F("The response will appear via callback when it arrives."));


            // --- METHOD 2: Synchronous (Blocking) Send ---
            /*
            // This function waits for the Tag's response or a timeout.
            // The `sendMessageToTag` is a simplified version of `anchorSendDataSync`.
            bool success = ryuw122.sendMessageToTag(TAG_ADDRESS, message.c_str());
            if (success) {
                Serial.println(F("Message sent and acknowledged by Tag (Sync)."));
                // Note: The callback will ALSO fire, so you might see the response twice.
            } else {
                Serial.println(F("Failed to send message or get acknowledgement (Sync)."));
            }
            */
        }
    }
}

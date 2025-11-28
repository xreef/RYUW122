/**
 * @file 03_Messaging_Tag.ino
 * @author Renzo Mischianti
 * @brief Example for a TAG to send and receive text messages.
 * @version 1.0.0
 * @date 2025-10-10
 *
 * This sketch allows you to send messages from the Serial Monitor to an Anchor
 * and displays any messages received from it.
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
const char* TAG_ADDRESS = "T1T1T1T1";

RYUW122 ryuw122(&Serial1, RESET_PIN);

/**
 * @brief Callback for receiving messages from the Anchor.
 */
void onTagDataReceived(int payloadLength, const char* data, int rssi) {
    Serial.println(F("\n--- Message Received from Anchor ---"));
    Serial.print(F("Message: ")); Serial.println(data);
    Serial.print(F("RSSI: ")); Serial.println(rssi);
    Serial.println(F("------------------------------------"));
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(100); }
    Serial.println(F("RYUW122 Simple Messaging Tag Example"));
    Serial.println(F("Type a message and press Enter to send it to the Anchor."));

    if (!ryuw122.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }

    ryuw122.setMode(RYUW122Mode::TAG);
    ryuw122.setNetworkId(NETWORK_ID);
    ryuw122.setAddress(TAG_ADDRESS);

    // Register the callback for incoming messages
    ryuw122.onTagReceive(onTagDataReceived);
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

            // Send the message from the Tag. This is a non-blocking "fire-and-forget" operation.
            ryuw122.tagSendData(message.length(), message.c_str());
            Serial.println(F("You can now send another message."));
        }
    }
}

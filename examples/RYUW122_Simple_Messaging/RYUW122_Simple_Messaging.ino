/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file RYUW122_Simple_Messaging.ino
 * @brief Simple messaging example using callbacks
 *
 * This example demonstrates simplified message exchange between ANCHOR and TAG.
 * The module automatically handles incoming messages through callbacks.
 *
 * Hardware connections:
 * - RYUW122 TX  -> Arduino RX (Pin 10)
 * - RYUW122 RX  -> Arduino TX (Pin 11)
 * - RYUW122 GND -> Arduino GND
 * - RYUW122 VCC -> 3.3V or 5V
 *
 * @note Configure one module as ANCHOR, another as TAG
 * @note This example shows ANCHOR side - see below for TAG version
 */

#include <RYUW122.h>

#define RESET_PIN 6 // NRST active LOW

// Create UWB instance
RYUW122 uwb(11, 10, RESET_PIN, RYUW122BaudRate::B_115200);

// Network configuration
const char* NETWORK_ID = "REYAX123";

#ifdef MODULE_MODE_ANCHOR
    const char* MY_ADDRESS = "ANCHOR01";
    const char* TARGET_ADDRESS = "TAG00001";
#else
    const char* MY_ADDRESS = "TAG00001";
    const char* TARGET_ADDRESS = "ANCHOR01";
#endif

// Timing
unsigned long lastSend = 0;
const unsigned long SEND_INTERVAL = 3000;

// Message counter
int messageCount = 0;

// ========================================
// CALLBACK FUNCTIONS
// ========================================

/**
 * @brief Called when a message is received
 * This is the simplified callback - just message and RSSI!
 */
void onMessageReceived(const char* fromAddress, const char* message, int rssi) {
    Serial.println();
    Serial.println(F("┌─────────────────────────────┐"));
    Serial.println(F("│     NEW MESSAGE RECEIVED    │"));
    Serial.println(F("└─────────────────────────────┘"));
    Serial.print(F("From: "));
    Serial.println(fromAddress);
    Serial.print(F("Message: \""));
    Serial.print(message);
    Serial.println(F("\""));
    Serial.print(F("Signal Strength (RSSI): "));
    Serial.println(rssi);
    Serial.println(F("─────────────────────────────"));
    Serial.println();
}

/**
 * @brief Called when distance is measured (ANCHOR only)
 */
void onDistanceMeasured(const char* fromAddress, float distance, MeasureUnit unit, int rssi) {
    Serial.print(F("📏 Distance to "));
    Serial.print(fromAddress);
    Serial.print(F(": "));
    Serial.print(distance, 1);

    switch(unit) {
        case MeasureUnit::CENTIMETERS: Serial.print(F(" cm")); break;
        case MeasureUnit::INCHES: Serial.print(F(" inches")); break;
        case MeasureUnit::METERS: Serial.print(F(" m")); break;
        case MeasureUnit::FEET: Serial.print(F(" ft")); break;
    }

    Serial.print(F(" (RSSI: "));
    Serial.print(rssi);
    Serial.println(F(")"));
}

// ========================================
// SETUP
// ========================================

void setup() {
    Serial.begin(115200);
    while (!Serial);

    Serial.println(F("\n"));
    Serial.println(F("================================================"));
    Serial.println(F("     RYUW122 Simple Messaging Example"));
    Serial.println(F("================================================"));
    Serial.println();

    // Initialize
    Serial.println(F("Initializing UWB module..."));
    if (!uwb.begin()) {
        Serial.println(F("ERROR: Failed to initialize!"));
        while(1) delay(1000);
    }
    Serial.println(F("✓ Module initialized"));

    // Configure mode
#ifdef MODULE_MODE_ANCHOR
    Serial.println(F("Configuring as ANCHOR..."));
    uwb.setMode(RYUW122Mode::ANCHOR);
#else
    Serial.println(F("Configuring as TAG..."));
    uwb.setMode(RYUW122Mode::TAG);
#endif

    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(MY_ADDRESS);
    uwb.setRssiDisplay(RYUW122RSSI::ENABLE);

    Serial.print(F("✓ My address: "));
    Serial.println(MY_ADDRESS);
    Serial.print(F("✓ Target address: "));
    Serial.println(TARGET_ADDRESS);
    Serial.println();

    // Register simplified callbacks
    uwb.onMessageReceived(onMessageReceived);

#ifdef MODULE_MODE_ANCHOR
    uwb.onDistanceMeasured(onDistanceMeasured, MeasureUnit::CENTIMETERS);
    Serial.println(F("✓ Callbacks registered (message + distance)"));
#else
    Serial.println(F("✓ Message callback registered"));
#endif

    Serial.println();
    Serial.println(F("================================================"));
    Serial.println(F("System ready! Starting communication..."));
    Serial.println(F("================================================"));
    Serial.println();

    delay(2000);
}

// ========================================
// MAIN LOOP
// ========================================

void loop() {
    // Process incoming messages - THIS IS IMPORTANT!
    uwb.processMessages();

    unsigned long currentTime = millis();

    // Send periodic messages
    if (currentTime - lastSend >= SEND_INTERVAL) {
        lastSend = currentTime;
        messageCount++;

        // Create a simple message
        char message[13];

#ifdef MODULE_MODE_ANCHOR
        snprintf(message, sizeof(message), "ANC%04d", messageCount);

        Serial.print(F("📤 Sending to TAG: \""));
        Serial.print(message);
        Serial.println(F("\""));

        // Simplified send - just one line!
        bool success = uwb.sendMessageToTag(TARGET_ADDRESS, message);

#else
        snprintf(message, sizeof(message), "TAG%04d", messageCount);

        Serial.print(F("📤 Sending from TAG: \""));
        Serial.print(message);
        Serial.println(F("\""));

        // TAG simplified send
        bool success = uwb.sendMessageFromTag(message);
#endif

        if (success) {
            Serial.println(F("✓ Message sent successfully"));
        } else {
            Serial.println(F("✗ Failed to send message"));
        }

        Serial.println();
    }
}

/**
 * @file tag_node.ino
 * @author Renzo Mischianti
 * @brief Unified example for basic positioning TAG communication. IPS (Indoor Positioning System)
 * @version 1.0.0
 * @date 2024-05-23
 *
 * More info and reference projects at https://mischianti.org/category/my-libraries/ryuw122-uwb/
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <RYUW122.h>

const char* NETWORK_ID = "AABBCCDD";
// UPPER LEFT NODE
const char* TAG_ADDRESS = "T1T1T1T1"; // unique tag address
// UPPER RIGHT NODE
// const char* TAG_ADDRESS = "T2T2T2T2"; // unique tag address
// BOTTOM RIGHT NODE
// const char* TAG_ADDRESS = "T3T3T3T3"; // unique tag address

// ------------------ ARDUINO UNO SOFTWARE SERIAL ------------------
// Define pins for SoftwareSerial (adjust based on your board)
// #define RX_PIN 10  // Connect to RYUW122 TX
// #define TX_PIN 11  // Connect to RYUW122 RX
//
// // Create RYUW122 instance with SoftwareSerial for Arduino UNO
// RYUW122 uwb(TX_PIN, RX_PIN, RYUW122BaudRate::B_9600);
// -----------------------------------------------------------------
// -------------------------- ARDUINO MEGA -------------------------
// RYUW122 uwb(&Serial1);
// -----------------------------------------------------------------
// ------------------------ ESP32 ----------------------------------
// --- Configuration ---
#define RX_PIN 9  // Connect to RYUW122 TX
#define TX_PIN 10  // Connect to RYUW122 RX
#define RESET_PIN 13 // Connect to RYUW122 NRST (active LOW)
//
// #define RX_PIN 5  // Connect to RYUW122 TX
// #define TX_PIN 4  // Connect to RYUW122 RX
// #define RESET_PIN 6 // Connect to RYUW122 NRST (active LOW)
// //
RYUW122 uwb(TX_PIN, RX_PIN, &Serial1, RESET_PIN);
// -----------------------------------------------------------------

void onTagDataReceived(int payloadLength, const char* data, int rssi) {
    Serial.println(F("\n--- Data Received from Anchor ---"));
    Serial.print(F("Data: ")); Serial.println(data);
    Serial.print(F("RSSI: ")); Serial.print(rssi); Serial.println(F(" dBm"));

    // The following line is NOT required for positioning.
    // It is included only to demonstrate how a tag *could* send data back if needed for a different application.
    // For pure positioning, the module handles the response automatically.
    const char* response = "PONG";
    uwb.tagSendData(strlen(response), response);
}

void setup() {
    Serial.begin(115200);
    // while (!Serial) { delay(10); }
    delay(1000);

    Serial.println(F("RYUW122 TAG (esp32c3)"));

    if (!uwb.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }

    uwb.setMode(RYUW122Mode::TAG);
    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(TAG_ADDRESS);
    uwb.onTagReceive(onTagDataReceived);

    Serial.println(F("Tag configured and listening for messages."));
}

void loop() {
    // uwb.loop() must be called continuously to allow the library to process
    // incoming UWB messages, manage module state, and trigger callbacks.
    uwb.loop();

    // A small delay can be added to prevent the loop from running too fast,
    // which can be more power-efficient and is generally good practice.
    delay(10);
}

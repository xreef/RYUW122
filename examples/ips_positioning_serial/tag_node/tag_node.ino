/**
 * @file tag_node.ino
 * @author Renzo Mischianti
 * @brief Unified example for basic positioning TAG communication. IPS (Indoor Positioning System)
 * @version 1.0.0
 * @date 2025-10-10
 *
 * This sketch contains two methods for communicating with a Tag.
 * METHOD 1 (Asynchronous) is recommended as it does not block the main loop.
 * METHOD 2 (Synchronous) is simpler but blocks the loop while waiting for a response.
 *
 * **Instructions:** Uncomment the method you want to use in the `loop()` function and comment out the other.
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
RYUW122 ryuw122(&Serial1);
// -----------------------------------------------------------------
// ------------------------ ESP32 ----------------------------------
// --- Configuration ---
// #define RX_PIN 9  // Connect to RYUW122 TX
// #define TX_PIN 10  // Connect to RYUW122 RX
// #define RESET_PIN 13 // Connect to RYUW122 NRST (active LOW)
//
// #define RX_PIN 5  // Connect to RYUW122 TX
// #define TX_PIN 4  // Connect to RYUW122 RX
// #define RESET_PIN 6 // Connect to RYUW122 NRST (active LOW)
// //
// RYUW122 ryuw122(TX_PIN, RX_PIN, &Serial1, RESET_PIN);
// -----------------------------------------------------------------

void onTagDataReceived(int payloadLength, const char* data, int rssi) {
    Serial.println(F("\n--- Data Received from Anchor ---"));
    Serial.print(F("Data: ")); Serial.println(data);
    Serial.print(F("RSSI: ")); Serial.print(rssi); Serial.println(F(" dBm"));

    const char* response = "PONG";
    ryuw122.tagSendData(strlen(response), response);
}

void setup() {
    Serial.begin(115200);
    // while (!Serial) { delay(10); }
    delay(1000);

    Serial.println(F("RYUW122 TAG (esp32c3)"));

    if (!ryuw122.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }

    ryuw122.setMode(RYUW122Mode::TAG);
    ryuw122.setNetworkId(NETWORK_ID);
    ryuw122.setAddress(TAG_ADDRESS);
    ryuw122.onTagReceive(onTagDataReceived);

    Serial.println(F("Tag configured and listening for messages."));
}

void loop() {
    ryuw122.loop();
    delay(10);
}

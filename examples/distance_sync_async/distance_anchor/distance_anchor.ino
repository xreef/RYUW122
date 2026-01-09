/**
 * @file distance_anchor.ino
 * @author Renzo Mischianti
 * @brief Unified example for an ANCHOR measuring distances to multiple TAGs, demonstrating both Async and Sync methods.
 * @version 1.0.0
 * @date 2025-10-10
 *
 * This sketch demonstrates two ways to poll multiple Tags for their distances.
 * METHOD 1 (Asynchronous) is more complex but doesn't block the loop.
 * METHOD 2 (Synchronous) is simpler but blocks the loop for each measurement.
 *
 * **Instructions:** Uncomment the method you want to use in the `loop()` function and comment out the other.
 *
 * @copyright Copyright (c) 2024
 *
 */

#include <RYUW122.h>



const char* NETWORK_ID = "AABBCCDD";
const char* ANCHOR_ADDRESS = "A1A1A1A1";

// List of Tag addresses to poll
const char* tagAddresses[] = {
    "T1T1T1T1",
    "T2T2T2T2",
    "T3T3T3T3"
};
const int numTags = sizeof(tagAddresses) / sizeof(tagAddresses[0]);

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
// #define RX_PIN 18  // Connect to RYUW122 TX
// #define TX_PIN 17  // Connect to RYUW122 RX
// #define RESET_PIN 13 // Connect to RYUW122 NRST (active LOW)
//
// #define RX_PIN 9  // Connect to RYUW122 TX
// #define TX_PIN 10  // Connect to RYUW122 RX
// #define RESET_PIN 13 // Connect to RYUW122 NRST (active LOW)
//
#define RX_PIN 5  // Connect to RYUW122 TX
#define TX_PIN 4  // Connect to RYUW122 RX
#define RESET_PIN 6 // Connect to RYUW122 NRST (active LOW)
// //
RYUW122 uwb(TX_PIN, RX_PIN, &Serial1, RESET_PIN);
// -----------------------------------------------------------------

// --- State Variables (for Async method) ---
int currentTagIndex = 0;
unsigned long lastPingTime = 0;
const unsigned long PING_INTERVAL = 1500; // Time between polling each tag

/**
 * @brief Callback function for the Asynchronous method.
 * This is triggered when any Tag responds to a ping.
 */
void onDistanceResponse(const char* tagAddress, int payloadLength, const char* data, int distance, int rssi) {
    Serial.print(F("Received response from "));
    Serial.print(tagAddress);
    Serial.print(F(" -> Distance: "));
    Serial.print(distance);
    Serial.println(F(" cm"));
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(100); }
    Serial.println(F("RYUW122 Unified Distance Anchor Example"));

    if (!uwb.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }

    uwb.setMode(RYUW122Mode::ANCHOR);
    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(ANCHOR_ADDRESS);

    // Register the callback function for the async method.
    uwb.onAnchorReceive(onDistanceResponse);

    Serial.println(F("Anchor configured. Starting main loop."));
}

void loop() {
    // =================================================================================
    // === CHOOSE YOUR METHOD: Uncomment the desired section and comment out the other ===
    // =================================================================================

    // --- METHOD 1: Asynchronous (Non-Blocking) Polling (Recommended) ---
    // This method cycles through the tags, sending a non-blocking request to each one.
    // It's more efficient as the main loop is never stalled waiting for a single tag.

    // Check if it's time to ping the next tag in the sequence
    if (millis() - lastPingTime >= PING_INTERVAL) {
        lastPingTime = millis();

        const char* currentTag = tagAddresses[currentTagIndex];
        Serial.print(F("\n[Async] Pinging TAG: "));
        Serial.println(currentTag);

        // Send a non-blocking request with an empty payload to get a distance reading.
        uwb.anchorSendData(currentTag, 0, "");

        // Move to the next tag for the next interval
        currentTagIndex = (currentTagIndex + 1) % numTags;
        if (currentTagIndex == 0) {
            Serial.println(F("--- Cycle complete ---"));
        }
    }
    // The uwb.loop() function is crucial. It processes incoming serial data
    // and triggers the onDistanceResponse callback when a reply arrives.
    uwb.loop();
    delay(10); // Small delay to keep the loop from running too fast


    // --- METHOD 2: Synchronous (Blocking) Polling ---
    /*
    Serial.println(F("\n--- Starting new measurement cycle (Sync) ---"));
    for (int i = 0; i < numTags; i++) {
        Serial.print(F("[Sync] Measuring distance to TAG: "));
        Serial.print(tagAddresses[i]);
        Serial.print(F("... "));

        // This function BLOCKS until a response is received or a timeout occurs.
        float distance = uwb.getDistanceFrom(tagAddresses[i]);

        if (distance >= 0) {
            Serial.print(F("Success! Distance: "));
            Serial.print(distance);
            Serial.println(F(" cm"));
        } else {
            Serial.println(F("Failed to get a response."));
        }
        // The delay is inside the loop, so the total cycle time is numTags * 1000ms
        delay(1000);
    }
    Serial.println(F("--- Cycle complete ---"));
    // No need for an extra delay here as it's already handled within the loop.
    */
}

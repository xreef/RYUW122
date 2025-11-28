/**
 * @file 01_Basic_Anchor.ino
 * @author Renzo Mischianti
 * @brief Unified example for basic ANCHOR communication, demonstrating both Async and Sync methods.
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

// --- Configuration ---
#define RX_PIN 5  // Connect to RYUW122 TX
#define TX_PIN 4  // Connect to RYUW122 RX
#define RESET_PIN 6 // Connect to RYUW122 NRST (active LOW)

const char* NETWORK_ID = "AABBCCDD";
const char* ANCHOR_ADDRESS = "A1A1A1A1";
const char* TAG_ADDRESS = "T1T1T1T1";

RYUW122 ryuw122(RX_PIN, TX_PIN, &Serial1, RESET_PIN);

// --- State Variables (for both methods) ---
unsigned long lastSendTime = 0;
const unsigned long SEND_INTERVAL = 5000; // 5 seconds
int loopCounter = 0;
volatile bool responseReceived = false; // Used by Async method

/**
 * @brief Callback function for the Asynchronous method.
 * This function is automatically called when a response from a Tag is received.
 */
void onAnchorDataReceived(const char* tagAddress, int payloadLength, const char* data, int distance, int rssi) {
    Serial.println(F("\n--- Callback Triggered (Async Response) ---"));
    Serial.print(F("Response from TAG: ")); Serial.println(tagAddress);
    Serial.print(F("Data: ")); Serial.println(data);
    Serial.print(F("Distance: ")); Serial.print(distance); Serial.println(F(" cm"));
    Serial.print(F("RSSI: ")); Serial.print(rssi); Serial.println(F(" dBm"));
    Serial.println(F("-------------------------------------------"));
    responseReceived = true;
}

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(100); }
    Serial.println(F("RYUW122 Unified Anchor Example (Async/Sync)"));

    if (!ryuw122.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }

    // Configure module settings
    ryuw122.setMode(RYUW122Mode::ANCHOR);
    ryuw122.setNetworkId(NETWORK_ID);
    ryuw122.setAddress(ANCHOR_ADDRESS);

    // Register the callback function. It's needed for the async method.
    // It doesn't harm to have it registered for the sync method too.
    ryuw122.onAnchorReceive(onAnchorDataReceived);

    Serial.println(F("Anchor configured. Starting main loop."));
}

void loop() {
    // =================================================================================
    // === CHOOSE YOUR METHOD: Uncomment the desired section and comment out the other ===
    // =================================================================================

    // --- METHOD 1: Asynchronous (Non-Blocking) Communication (Recommended) ---
    // The main loop continues to run while waiting for a response.
    Serial.print(F("Main loop running... Counter: "));
    Serial.println(loopCounter++);

    // Periodically send a non-blocking request
    if (millis() - lastSendTime >= SEND_INTERVAL) {
        lastSendTime = millis();
        const char* message = "PING (Async)";
        Serial.print(F("\nSending '")); Serial.print(message); Serial.print(F("' to TAG ")); Serial.println(TAG_ADDRESS);
        ryuw122.anchorSendData(TAG_ADDRESS, strlen(message), message); // Returns immediately
    }
    // The ryuw122.loop() function processes incoming data and triggers the callback when a response arrives.
    ryuw122.loop();
    delay(500); // Slow down loop for readability

    // --- METHOD 2: Synchronous (Blocking) Communication ---
    /*
    Serial.println(F("\nAttempting to send data to Tag (blocking)..."));
    const char* message = "PING (Sync)";

    // This function WAITS for a response or timeout
    AnchorResponse response = ryuw122.anchorSendDataSync(TAG_ADDRESS, strlen(message), message);

    if (response.success) {
        Serial.println(F("--- Sync Response Received ---"));
        Serial.print(F("Data: ")); Serial.println(response.responseData);
        Serial.print(F("Distance: ")); Serial.print(response.distance); Serial.println(F(" cm"));
        Serial.print(F("RSSI: ")); Serial.print(response.rssi); Serial.println(F(" dBm"));
        Serial.println(F("------------------------------"));
    } else {
        Serial.println(F("Failed to receive sync response from Tag."));
    }
    delay(SEND_INTERVAL); // Wait before sending the next blocking request
    */
}

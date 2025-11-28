/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file 05_Tracking_Master_Anchor.ino
 * @brief Unified Master/Anchor sketch for a real-time tracking system, with Sync and Async methods.
 * @version 1.0.0
 * @date 2025-10-10
 *
 * This example implements a real-time indoor tracking system.
 *
 * **Instructions:** Choose the desired measurement method (Sync or Async) in the `loop()` function.
 *
 * @note This sketch is designed to be run on ONE device (the Master). The other Anchor devices
 *       can run a minimal sketch, and the Tag runs the corresponding `05_Tracking_Tag.ino`.
 */

#include <RYUW122.h>

// ========================================
// SYSTEM CONFIGURATION
// ========================================

RYUW122 ryuw122(&Serial1, RESET_PIN);

const char* NETWORK_ID = "TRACKSYS";
const char* THIS_ADDRESS = "ANCHOR01"; // Master's address
const char* TARGET_TAG_ADDRESS = "TAG_TRACK1";

const int NUM_ANCHORS = 3;
struct Anchor {
    const char* address;
    float x;  // position in cm
    float y;
};

Anchor anchors[NUM_ANCHORS] = {
    {"ANCHOR01", 0.0,   0.0},
    {"ANCHOR02", 500.0, 0.0},
    {"ANCHOR03", 250.0, 433.0}
};

const float UPDATE_RATE_HZ = 1.0;
const unsigned long UPDATE_INTERVAL = (unsigned long)(1000.0 / UPDATE_RATE_HZ);
unsigned long lastUpdate = 0;

// --- State Variables for Asynchronous Method ---
enum class SystemState { IDLE, GATHERING_DISTANCES };
SystemState currentState = SystemState::IDLE;
float measuredDistances[NUM_ANCHORS];
bool responseReceived[NUM_ANCHORS];
int responsesCount = 0;

// ========================================
// POSITION CALCULATION & HELPERS
// ========================================

struct Position { float x; float y; bool valid; };

Position calculatePosition(float d1, float d2, float d3) {
    Position result = {0, 0, false};
    float x1 = anchors[0].x, y1 = anchors[0].y;
    float x2 = anchors[1].x, y2 = anchors[1].y;
    float x3 = anchors[2].x, y3 = anchors[2].y;

    float A = 2 * (x2 - x1);
    float B = 2 * (y2 - y1);
    float C = d1*d1 - d2*d2 - x1*x1 + x2*x2 - y1*y1 + y2*y2;
    float D = 2 * (x3 - x1);
    float E = 2 * (y3 - y1);
    float F = d1*d1 - d3*d3 - x1*x1 + x3*x3 - y1*y1 + y3*y3;

    float denominator = (A*E - B*D);
    if (abs(denominator) < 0.01) return result;

    result.x = (C*E - F*B) / denominator;
    result.y = (A*F - D*C) / denominator;
    result.valid = !isnan(result.x) && !isnan(result.y);
    return result;
}

/**
 * @brief Callback for the Asynchronous method.
 */
void onDistanceResponse(const char* tagAddress, int payloadLength, const char* data, int distance, int rssi) {
    if (currentState != SystemState::GATHERING_DISTANCES || strcmp(tagAddress, TARGET_TAG_ADDRESS) != 0) {
        return;
    }
    if (responsesCount < NUM_ANCHORS && !responseReceived[responsesCount]) {
        measuredDistances[responsesCount] = distance;
        responseReceived[responsesCount] = true;
        responsesCount++;
    }
}

// ========================================
// SETUP
// ========================================

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println(F("\n\n╔════════════════════════════════════════════════╗"));
    Serial.println(F("║   RYUW122 Real-Time Indoor Tracking System    ║"));
    Serial.println(F("╚════════════════════════════════════════════════╝"));

    if (!ryuw122.begin()) {
        Serial.println(F("❌ FATAL: UWB module initialization failed!"));
        while(1);
    }

    ryuw122.setMode(RYUW122Mode::ANCHOR);
    ryuw122.setNetworkId(NETWORK_ID);
    ryuw122.setAddress(THIS_ADDRESS);
    ryuw122.onAnchorReceive(onDistanceResponse);

    Serial.println(F("👑 This device is the MASTER."));
}

// ========================================
// MAIN LOOP
// ========================================

void loop() {
    ryuw122.loop();

    // =================================================================================
    // === CHOOSE YOUR METHOD: Uncomment the desired section and comment out the other ===
    // =================================================================================

    // --- METHOD 1: Asynchronous (Non-Blocking) Measurement (Recommended) ---
    if (currentState == SystemState::IDLE && (millis() - lastUpdate >= UPDATE_INTERVAL)) {
        responsesCount = 0;
        for (int i = 0; i < NUM_ANCHORS; i++) responseReceived[i] = false;

        for (int i = 0; i < NUM_ANCHORS; i++) {
            ryuw122.anchorSendData(TARGET_TAG_ADDRESS, 0, "");
            delay(50);
        }
        currentState = SystemState::GATHERING_DISTANCES;
        lastUpdate = millis();
    }

    if (currentState == SystemState::GATHERING_DISTANCES && responsesCount >= NUM_ANCHORS) {
        Position pos = calculatePosition(measuredDistances[0], measuredDistances[1], measuredDistances[2]);
        if (pos.valid) {
            Serial.println(F("✅ SUCCESS: Tag Position Calculated (Async)!"));
            Serial.print(F("   - CSV:")); Serial.print(millis()); Serial.print(F(","));
            Serial.print(pos.x, 1); Serial.print(F(",")); Serial.println(pos.y, 1);
        } else {
            Serial.println(F("❌ ERROR: Trilateration calculation failed (Async)."));
        }
        currentState = SystemState::IDLE;
    }

    // --- METHOD 2: Synchronous (Blocking) Measurement ---
    /*
    if (millis() - lastUpdate >= UPDATE_INTERVAL) {
        lastUpdate = millis();
        float distances[NUM_ANCHORS];
        int successCount = 0;

        // A more correct synchronous approach would be:
        for (int i = 0; i < NUM_ANCHORS; i++) {
            distances[i] = ryuw122.getDistanceFrom(TARGET_TAG_ADDRESS);
            if(distances[i] >= 0) successCount++;
        }


        if (successCount >= 3) {
            Position pos = calculatePosition(distances[0], distances[1], distances[2]);
            if (pos.valid) {
                Serial.println(F("✅ SUCCESS: Tag Position Calculated (Sync)!"));
                Serial.print(F("   - CSV:")); Serial.print(millis()); Serial.print(F(","));
                Serial.print(pos.x, 1); Serial.print(F(",")); Serial.println(pos.y, 1);
            } else {
                Serial.println(F("❌ ERROR: Trilateration calculation failed (Sync)."));
            }
        } else {
            Serial.println(F("⚠️ ERROR: Could not get enough distance measurements (Sync)."));
        }
    }
    */
}

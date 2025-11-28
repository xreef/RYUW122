/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file 04_Positioning_Master_Anchor.ino
 * @brief Unified example for 2D indoor positioning, with both Sync and Async methods.
 * @version 1.0.0
 * @date 2025-10-10
 *
 * This sketch acts as the "Master" in a positioning system. It gathers distances
 * to a mobile Tag and calculates its (x, y) position.
 *
 * **Instructions:** Choose the desired measurement method (Sync or Async) in the `loop()` function.
 *
 * @note This sketch must be uploaded to ONE device (the Master). The other Anchor devices
 *       can run a minimal sketch as they only need to be "visible" for ranging. The Tag
 *       device runs the corresponding `04_Positioning_Tag.ino` sketch.
 */

#include <RYUW122.h>

// ========================================
// CONFIGURATION
// ========================================

RYUW122 ryuw122(&Serial1, RESET_PIN);

const char* NETWORK_ID = "REYAXPOS";
const char* THIS_ADDRESS = "ANCHOR01"; // Master's address
const char* MOBILE_TAG_ADDRESS = "TAG_POS1";

const int NUM_ANCHORS = 3;
struct Anchor {
    const char* address;
    float x; // position in cm
    float y;
};

Anchor anchors[NUM_ANCHORS] = {
    {"ANCHOR01", 0.0, 0.0},
    {"ANCHOR02", 400.0, 0.0},
    {"ANCHOR03", 200.0, 350.0}
};

// --- Master Settings ---
unsigned long lastPositionUpdate = 0;
const unsigned long UPDATE_INTERVAL = 2000;

// --- State Variables for Asynchronous Method ---
enum class SystemState { IDLE, GATHERING_DISTANCES };
SystemState currentState = SystemState::IDLE;
float measuredDistances[NUM_ANCHORS];
bool responseReceived[NUM_ANCHORS];
int responsesCount = 0;

// ========================================
// TRILATERATION & HELPER FUNCTIONS
// ========================================

struct Position { float x; float y; };

bool calculatePosition2D(Position p1, Position p2, Position p3, float d1, float d2, float d3, Position &result) {
    float A = 2 * (p2.x - p1.x);
    float B = 2 * (p2.y - p1.y);
    float C = d1*d1 - d2*d2 - p1.x*p1.x + p2.x*p2.x - p1.y*p1.y + p2.y*p2.y;
    float D = 2 * (p3.x - p1.x);
    float E = 2 * (p3.y - p1.y);
    float F = d1*d1 - d3*d3 - p1.x*p1.x + p3.x*p3.x - p1.y*p1.y + p3.y*p3.y;
    float denominator = (A*E - B*D);
    if (abs(denominator) < 0.01) return false;
    result.x = (C*E - F*B) / denominator;
    result.y = (A*F - D*C) / denominator;
    return !isnan(result.x) && !isnan(result.y);
}

/**
 * @brief Callback for the Asynchronous method.
 * This function is called when a distance response is received from the Tag.
 */
void onDistanceResponse(const char* tagAddress, int payloadLength, const char* data, int distance, int rssi) {
    if (currentState != SystemState::GATHERING_DISTANCES || strcmp(tagAddress, MOBILE_TAG_ADDRESS) != 0) {
        return; // Ignore responses if not in the right state or from the wrong tag
    }

    // This is a simplification: the response doesn't tell us WHICH anchor's request it's for.
    // In a real async multi-anchor system, the request or response would need a unique ID.
    // For this example, we assume the responses arrive in the order they were requested.
    if (responsesCount < NUM_ANCHORS) {
        if (!responseReceived[responsesCount]) {
            Serial.print(F("  -> Response received for request "));
            Serial.print(responsesCount + 1);
            Serial.print(F(": "));
            Serial.print(distance);
            Serial.println(F(" cm"));
            measuredDistances[responsesCount] = distance;
            responseReceived[responsesCount] = true;
            responsesCount++;
        }
    }
}

// ========================================
// SETUP & MAIN LOOP
// ========================================

void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println(F("\n================================================"));
    Serial.println(F("  RYUW122 Indoor Positioning System (Master)"));
    Serial.println(F("================================================"));

    if (!ryuw122.begin()) {
        Serial.println(F("FATAL: Failed to initialize RYUW122 module!"));
        while(1);
    }

    ryuw122.setMode(RYUW122Mode::ANCHOR);
    ryuw122.setNetworkId(NETWORK_ID);
    ryuw122.setAddress(THIS_ADDRESS);

    ryuw122.onAnchorReceive(onDistanceResponse);

    Serial.println(F("Master configured. System is running..."));
}

void loop() {
    // The loop() function must always be called to process incoming messages
    ryuw122.loop();

    // =================================================================================
    // === CHOOSE YOUR METHOD: Uncomment the desired section and comment out the other ===
    // =================================================================================

    // --- METHOD 1: Asynchronous (Non-Blocking) Measurement (Recommended) ---
    if (currentState == SystemState::IDLE && (millis() - lastPositionUpdate >= UPDATE_INTERVAL)) {
        Serial.println(F("\n--- [Async] Starting new measurement cycle ---"));
        // Reset state
        responsesCount = 0;
        for (int i = 0; i < NUM_ANCHORS; i++) {
            responseReceived[i] = false;
        }

        // Send out all ranging requests without blocking
        for (int i = 0; i < NUM_ANCHORS; i++) {
            Serial.print(F("Pinging for distance from Anchor "));
            Serial.print(i+1);
            Serial.println(F("..."));
            // In a real system, you'd command each anchor. Here, the master does all ranging.
            ryuw122.anchorSendData(MOBILE_TAG_ADDRESS, 0, "");
            delay(50); // Small delay between requests
        }
        currentState = SystemState::GATHERING_DISTANCES;
        lastPositionUpdate = millis();
    }

    // Check if we have gathered all responses
    if (currentState == SystemState::GATHERING_DISTANCES && responsesCount >= NUM_ANCHORS) {
        Serial.println(F("All responses received. Calculating position..."));
        Position tagPosition;
        Position p1 = {anchors[0].x, anchors[0].y};
        Position p2 = {anchors[1].x, anchors[1].y};
        Position p3 = {anchors[2].x, anchors[2].y};

        if (calculatePosition2D(p1, p2, p3, measuredDistances[0], measuredDistances[1], measuredDistances[2], tagPosition)) {
            Serial.println(F("\n*** SUCCESS: Tag Position Calculated (Async) ***"));
            Serial.print(F("Coordinates (X, Y): ("));
            Serial.print(tagPosition.x, 1); Serial.print(F(" cm, "));
            Serial.print(tagPosition.y, 1); Serial.println(F(" cm)"));
        } else {
            Serial.println(F("ERROR: Trilateration calculation failed."));
        }
        currentState = SystemState::IDLE; // Reset for the next cycle
    }


    // --- METHOD 2: Synchronous (Blocking) Measurement ---
    /*
    if (millis() - lastPositionUpdate >= UPDATE_INTERVAL) {
        lastPositionUpdate = millis();
        Serial.println(F("\n--- [Sync] Starting new measurement cycle ---"));
        float distances[NUM_ANCHORS];
        int successCount = 0;

        // Poll each anchor sequentially, blocking each time
        for (int i = 0; i < NUM_ANCHORS; i++) {
            Serial.print(F("Querying distance for Anchor "));
            Serial.print(i + 1);
            Serial.print(F("... "));

            // The Master measures the distance to the Tag.
            distances[i] = ryuw122.getDistanceFrom(MOBILE_TAG_ADDRESS);

            if (distances[i] >= 0) {
                Serial.print(F("Success! Distance = "));
                Serial.print(distances[i]);
                Serial.println(F(" cm"));
                successCount++;
            } else {
                Serial.println(F("Failed."));
            }
        }

        if (successCount >= 3) {
            Position tagPosition;
            Position p1 = {anchors[0].x, anchors[0].y};
            Position p2 = {anchors[1].x, anchors[1].y};
            Position p3 = {anchors[2].x, anchors[2].y};

            if (calculatePosition2D(p1, p2, p3, distances[0], distances[1], distances[2], tagPosition)) {
                Serial.println(F("\n*** SUCCESS: Tag Position Calculated (Sync) ***"));
                Serial.print(F("Coordinates (X, Y): ("));
                Serial.print(tagPosition.x, 1); Serial.print(F(" cm, "));
                Serial.print(tagPosition.y, 1); Serial.println(F(" cm)"));
            } else {
                Serial.println(F("ERROR: Trilateration calculation failed."));
            }
        } else {
            Serial.println(F("ERROR: Not enough successful measurements."));
        }
    }
    */
}

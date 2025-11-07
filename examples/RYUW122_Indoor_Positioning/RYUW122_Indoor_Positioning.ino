/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

// Original content preserved; copied to own folder for consistency

#include <RYUW122.h>

// ========================================
// CONFIGURATION
// ========================================

// Create UWB instance (adjust pins for your board)
RYUW122 uwb(11, 10, RYUW122BaudRate::B_115200); // TX=11, RX=10

// Network configuration
const char* NETWORK_ID = "REYAX123";
const char* THIS_ADDRESS = "ANCHOR01"; // Change if running on TAG

// ANCHOR addresses (must match actual configuration)
const char* ANCHOR1_ADDR = "ANCHOR01";
const char* ANCHOR2_ADDR = "ANCHOR02";
const char* ANCHOR3_ADDR = "ANCHOR03";

// TAG address we want to track
const char* MOBILE_TAG = "TAG00001";

// ANCHOR positions in centimeters (CALIBRATE THESE!)
struct Position {
    float x;
    float y;
};

Position anchor1_pos = {0.0, 0.0};      // Origin - bottom left corner
Position anchor2_pos = {400.0, 0.0};    // Bottom right corner (4 meters from anchor1)
Position anchor3_pos = {0.0, 300.0};    // Top left corner (3 meters from anchor1)

// Measurement settings
unsigned long lastPositionUpdate = 0;
const unsigned long UPDATE_INTERVAL = 2000; // Update position every 2 seconds

// (trilateration functions and rest of the file kept identical)

// ...existing code...


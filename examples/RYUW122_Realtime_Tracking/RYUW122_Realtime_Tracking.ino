/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

// Original file content copied into its own folder

#include <RYUW122.h>

// ========================================
// SYSTEM CONFIGURATION
// ========================================

// UWB module connection
RYUW122 uwb(11, 10, RYUW122BaudRate::B_115200);

// Network settings
const char* NETWORK_ID = "REYAX123";
const char* COORDINATOR_ADDR = "COORD001";

// ...existing code...


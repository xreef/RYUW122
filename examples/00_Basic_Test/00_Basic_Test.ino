/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * Part of the RYUW122 Arduino library examples. See LICENSE for details.
 */

/**
 * @file RYUW122_Basic_Test.ino
 * @brief Basic example for testing RYUW122 module communication and configuration
 *
 * This example performs basic AT command tests to verify:
 * - Serial communication with the module
 * - Configuration of basic parameters
 * - Reading module information
 *
 * Hardware connections:
 * - RYUW122 TX  -> Arduino RX (e.g., Pin 10 for SoftwareSerial)
 * - RYUW122 RX  -> Arduino TX (e.g., Pin 11 for SoftwareSerial)
 * - RYUW122 GND -> Arduino GND
 * - RYUW122 VCC -> Arduino 3.3V or 5V (check module specifications)
 *
 * Open the Serial Monitor at 115200 baud to see the test results.
 */

#include <RYUW122.h>

// Define pins for SoftwareSerial (adjust based on your board)
#define RX_PIN 10  // Connect to RYUW122 TX
#define TX_PIN 11  // Connect to RYUW122 RX
#define RESET_PIN 6 // Connect to RYUW122 NRST (active LOW)

// Create RYUW122 instance with SoftwareSerial
RYUW122 uwb(TX_PIN, RX_PIN, RESET_PIN);

void printTestResult(const char* testName, bool result) {
    Serial.print(F("["));
    Serial.print(result ? F("PASS") : F("FAIL"));
    Serial.print(F("] "));
    Serial.println(testName);
}

void printSeparator() {
    Serial.println(F("========================================"));
}

void setup() {
    // Initialize serial for debugging
    Serial.begin(115200);
    while (!Serial) {
        ; // Wait for serial port to connect (needed for native USB)
    }

    delay(1000); // Give time for module to power up

    Serial.println(F("\n"));
    printSeparator();
    Serial.println(F("RYUW122 Basic Test"));
    printSeparator();

    // Test 1: Initialize module
    Serial.println(F("\nTest 1: Initialize Module"));
    bool initResult = uwb.begin();
    printTestResult("Module Initialization", initResult);
    if (!initResult) {
        Serial.println(F("ERROR: Cannot initialize module!"));
        Serial.println(F("Check connections and baud rate."));
        while (1) {
            delay(1000);
        }
    }

    delay(500);

    // Test 2: AT command test
    Serial.println(F("\nTest 2: AT Command Communication"));
    bool atResult = uwb.test();
    printTestResult("AT Command Test", atResult);
    if (!atResult) {
        Serial.println(F("ERROR: Module not responding to AT commands!"));
        while (1) {
            delay(1000);
        }
    }

    delay(500);

    // Test 3: Get firmware version
    Serial.println(F("\nTest 3: Read Firmware Version"));
    char version[32];
    bool versionResult = uwb.getFirmwareVersion(version);
    printTestResult("Get Firmware Version", versionResult);
    if (versionResult) {
        Serial.print(F("  Version: "));
        Serial.println(version);
    }

    delay(500);

    // Test 4: Get unique ID
    Serial.println(F("\nTest 4: Read Unique ID"));
    char uid[32];
    bool uidResult = uwb.getUid(uid);
    printTestResult("Get Unique ID", uidResult);
    if (uidResult) {
        Serial.print(F("  UID: "));
        Serial.println(uid);
    }

    delay(500);

    // Test 5: Set and verify mode
    Serial.println(F("\nTest 5: Set Operating Mode (TAG)"));
    bool setModeResult = uwb.setMode(RYUW122Mode::TAG);
    printTestResult("Set Mode", setModeResult);

    RYUW122Mode currentMode = uwb.getMode();
    bool verifyModeResult = (currentMode == RYUW122Mode::TAG);
    printTestResult("Verify Mode", verifyModeResult);
    if (verifyModeResult) {
        Serial.print(F("  Current Mode: "));
        Serial.println(RYUW122Mode_description(currentMode));
    }

    delay(500);

    // Test 6: Set and verify baud rate
    Serial.println(F("\nTest 6: Verify Baud Rate"));
    RYUW122BaudRate currentBaud = uwb.getBaudRate();
    Serial.print(F("  Current Baud Rate: "));
    Serial.println(RYUW122BaudRate_description(currentBaud));

    delay(500);

    // Test 7: Set and verify RF channel
    Serial.println(F("\nTest 7: Set RF Channel"));
    bool setChannelResult = uwb.setRfChannel(RYUW122RFChannel::CH_5);
    printTestResult("Set RF Channel", setChannelResult);

    RYUW122RFChannel currentChannel = uwb.getRfChannel();
    bool verifyChannelResult = (currentChannel == RYUW122RFChannel::CH_5);
    printTestResult("Verify RF Channel", verifyChannelResult);
    if (verifyChannelResult) {
        Serial.print(F("  Current Channel: "));
        Serial.println(RYUW122RFChannel_description(currentChannel));
    }

    delay(500);

    // Test 8: Set and verify bandwidth
    Serial.println(F("\nTest 8: Set Bandwidth"));
    bool setBandwidthResult = uwb.setBandwidth(RYUW122Bandwidth::BW_850K);
    printTestResult("Set Bandwidth", setBandwidthResult);

    RYUW122Bandwidth currentBandwidth = uwb.getBandwidth();
    bool verifyBandwidthResult = (currentBandwidth == RYUW122Bandwidth::BW_850K);
    printTestResult("Verify Bandwidth", verifyBandwidthResult);
    if (verifyBandwidthResult) {
        Serial.print(F("  Current Bandwidth: "));
        Serial.println(RYUW122Bandwidth_description(currentBandwidth));
    }

    delay(500);

    // Test 9: Set and verify network ID
    Serial.println(F("\nTest 9: Set Network ID"));
    const char* testNetworkId = "REYAX123";
    bool setNetworkResult = uwb.setNetworkId(testNetworkId);
    printTestResult("Set Network ID", setNetworkResult);

    char currentNetworkId[16];
    bool getNetworkResult = uwb.getNetworkId(currentNetworkId);
    printTestResult("Get Network ID", getNetworkResult);
    if (getNetworkResult) {
        Serial.print(F("  Network ID: "));
        Serial.println(currentNetworkId);
    }

    delay(500);

    // Test 10: Set and verify address
    Serial.println(F("\nTest 10: Set Address"));
    const char* testAddress = "TEST1234";
    bool setAddressResult = uwb.setAddress(testAddress);
    printTestResult("Set Address", setAddressResult);

    char currentAddress[16];
    bool getAddressResult = uwb.getAddress(currentAddress);
    printTestResult("Get Address", getAddressResult);
    if (getAddressResult) {
        Serial.print(F("  Address: "));
        Serial.println(currentAddress);
    }

    delay(500);

    // Test 11: Set RF power
    Serial.println(F("\nTest 11: Set RF Power"));
    bool setPowerResult = uwb.setRfPower(RYUW122RFPower::N32dBm);
    printTestResult("Set RF Power", setPowerResult);

    RYUW122RFPower currentPower = uwb.getRfPower();
    bool verifyPowerResult = (currentPower == RYUW122RFPower::N32dBm);
    printTestResult("Verify RF Power", verifyPowerResult);
    if (verifyPowerResult) {
        Serial.print(F("  RF Power: "));
        Serial.println(RYUW122RFPower_description(currentPower));
    }

    delay(500);

    // Test 12: RSSI display setting
    Serial.println(F("\nTest 12: RSSI Display Setting"));
    bool setRssiResult = uwb.setRssiDisplay(RYUW122RSSI::ENABLE);
    printTestResult("Set RSSI Display", setRssiResult);

    RYUW122RSSI currentRssi = uwb.getRssiDisplay();
    bool verifyRssiResult = (currentRssi == RYUW122RSSI::ENABLE);
    printTestResult("Verify RSSI Display", verifyRssiResult);
    if (verifyRssiResult) {
        Serial.print(F("  RSSI Display: "));
        Serial.println(RYUW122RSSI_description(currentRssi));
    }

    delay(500);

    // Test 13: Distance calibration
    Serial.println(F("\nTest 13: Distance Calibration"));
    bool setCalResult = uwb.setDistanceCalibration(0);
    printTestResult("Set Calibration", setCalResult);

    int currentCal = uwb.getDistanceCalibration();
    Serial.print(F("  Calibration Value: "));
    Serial.print(currentCal);
    Serial.println(F(" cm"));

    delay(500);

    // Summary
    printSeparator();
    Serial.println(F("Basic Test Complete!"));
    Serial.println(F("All essential functions tested."));
    printSeparator();
    Serial.println(F("\nModule is ready for operation."));
    Serial.println(F("Use RYUW122_Anchor_Example or RYUW122_Tag_Example"));
    Serial.println(F("for communication testing."));
}

void loop() {
    // Nothing to do in loop for basic test
    delay(1000);
}

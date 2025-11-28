/*
  Heltec_Anchor_Display.ino
  Example for RYUW122 library
  Heltec WiFi Kit 32 V2 used as ANCHOR with OLED display

  Connect RYUW122: TX->GPIO16, RX->GPIO17, NRST->GPIO13
  Note: If your OLED reset pin conflicts with RX, move RX to GPIO18.

  This example polls a TAG and shows received distance, RSSI and payload
  on the built-in SSD1306 OLED. It uses Serial2 for the RYUW122 module.
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RYUW122.h>

// OLED Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     RST_OLED
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RYUW122 UWB Module configuration
// Serial2 pins: TX2=GPIO17, RX2=GPIO18 (RX moved to 18 if RST_OLED uses 16)
#define RX_PIN 18
#define TX_PIN 17
#define RESET_PIN 13  // Connect to RYUW122 NRST (active LOW)

// Configuration - MUST MATCH OTHER DEVICES
const char* NETWORK_ID = "AABBCCDD";       // Same network as other devices
const char* ANCHOR_ADDRESS = "A2A2A2A2";   // Anchor address
const char* TAG_TARGET = "T1T1T1T1";       // Target TAG address

// Initialize RYUW122 library with Serial2
// Constructor: RYUW122(mcuTxPin, mcuRxPin, serial, lowResetTriggerInputPin)
RYUW122 ryuw122(TX_PIN, RX_PIN, &Serial2, RESET_PIN);

// Data structure for received data
struct ReceivedData {
  String tagAddress;
  float distance;  // in meters
  int rssi;
  String data;
  unsigned long timestamp;
  bool valid;
} lastData;

// Statistics
unsigned long packetsReceived = 0;
unsigned long lastPollTime = 0;
const unsigned long SEND_INTERVAL = 5000; // Poll every 5 seconds

// Function declarations
void updateDisplay();
void onAnchorDataReceived(const char* tagAddress, int payloadLength, const char* data, int distance, int rssi);

void onAnchorDataReceived(const char* tagAddress, int payloadLength, const char* data, int distance, int rssi) {
  // Update received data structure
  lastData.tagAddress = String(tagAddress);
  lastData.data = String(data);
  lastData.distance = distance / 100.0; // Convert cm to meters
  lastData.rssi = rssi;
  lastData.timestamp = millis();
  lastData.valid = true;
  packetsReceived++;

  // Update display immediately
  updateDisplay();
}

void setup() {
  // Initialize Serial for debug
  Serial.begin(115200);
  delay(500);
  Serial.println(F("\n=== RYUW122 ANCHOR - Heltec WiFi Kit 32 V2 ==="));

  // Initialize OLED display (Heltec defines SDA_OLED/SCL_OLED/RST_OLED)
  Wire.begin(SDA_OLED, SCL_OLED);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("RYUW122 ANCHOR"));
  display.println(F("Initializing..."));
  display.display();
  delay(800);

  // Initialize data structure
  lastData.valid = false;
  lastData.distance = 0.0;
  lastData.rssi = 0;
  lastData.tagAddress = "";
  lastData.data = "";
  lastData.timestamp = 0;

  // Initialize RYUW122 library
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Init UWB module..."));
  display.display();

  if (!ryuw122.begin()) {
    Serial.println(F("Failed to initialize RYUW122 module"));
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println(F("UWB Init FAILED!"));
    display.println(F("Check wiring"));
    display.display();
    for(;;);
  }

  Serial.println(F("RYUW122 module OK"));
  delay(500);

  // Configure as ANCHOR
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Configuring..."));
  display.println(F("Setting MODE..."));
  display.display();

  if (!ryuw122.setMode(RYUW122Mode::ANCHOR)) {
    Serial.println(F("Failed to set ANCHOR mode"));
  }
  Serial.println(F("Mode: ANCHOR"));
  delay(500);

  // Set Network ID
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Setting Network..."));
  display.display();

  ryuw122.setNetworkId(NETWORK_ID);
  Serial.print(F("Network ID: ")); Serial.println(NETWORK_ID);
  delay(500);

  // Set ANCHOR Address
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("Setting Address..."));
  display.display();

  ryuw122.setAddress(ANCHOR_ADDRESS);
  Serial.print(F("Address: ")); Serial.println(ANCHOR_ADDRESS);
  delay(500);

  // Register the callback function
  ryuw122.onAnchorReceive(onAnchorDataReceived);
  Serial.println(F("Callback registered"));

  Serial.println(F("=== CONFIGURATION COMPLETE ==="));
  Serial.print(F("Network: ")); Serial.println(NETWORK_ID);
  Serial.print(F("Address: ")); Serial.println(ANCHOR_ADDRESS);
  Serial.print(F("Target: ")); Serial.println(TAG_TARGET);

  // Show ready screen
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(F("ANCHOR READY"));
  display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
  display.setCursor(0, 14);
  display.println(F("Waiting for TAG"));
  display.println();
  display.print(F("Net: "));
  display.println(NETWORK_ID);
  display.print(F("Addr: "));
  display.println(ANCHOR_ADDRESS);
  display.display();

  Serial.println(F("=== READY TO POLL ==="));

  lastPollTime = millis();
}

void loop() {
  // Poll TAG periodically
  if (millis() - lastPollTime >= SEND_INTERVAL) {
    const char* payload = "POLL";
    Serial.print(F("\n[ANCHOR] Polling TAG: "));
    Serial.println(TAG_TARGET);

    ryuw122.anchorSendData(TAG_TARGET, strlen(payload), payload);
    lastPollTime = millis();
  }

  // Process incoming data and trigger callbacks
  ryuw122.loop();

  delay(50);
}

void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  // Title
  display.print(F("ANCHOR "));
  display.println(ANCHOR_ADDRESS);
  display.drawLine(0, 9, SCREEN_WIDTH, 9, SSD1306_WHITE);

  // Data section
  display.setCursor(0, 12);
  if (lastData.valid) {
    display.print(F("TAG: "));
    display.println(lastData.tagAddress);

    display.print(F("Dist: "));
    display.print(lastData.distance, 2);
    display.println(F(" m"));

    display.print(F("RSSI: "));
    display.print(lastData.rssi);
    display.println(F(" dBm"));

    display.print(F("Data: "));
    display.println(lastData.data);

    // Time since last packet
    unsigned long elapsed = (millis() - lastData.timestamp) / 1000;
    display.print(F("Age: "));
    display.print(elapsed);
    display.println(F(" s"));

    // Packet count
    display.print(F("Pkts: "));
    display.println(packetsReceived);
  } else {
    display.println(F("No data"));
    display.println(F("Waiting for TAG..."));
  }

  display.display();
}


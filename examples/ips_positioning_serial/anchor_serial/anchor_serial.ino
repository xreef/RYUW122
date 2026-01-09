/**
 * @file anchor_serial.cpp
 * @author Renzo Mischianti
 * @brief RYUW122 MASTER ANCHOR
 * @version 1.0.0
 * @date 2025-11-28
 *
 * This sketch configures the MCU as a MASTER/ANCHOR (mobile) that sequentially
 * polls three fixed anchors, collects distances returned by them and computes
 * the device (mobile) position by trilateration. Results are printed to Serial.
 * Optional ASCII-art map can be toggled.
 */

#include <Arduino.h>
#include <RYUW122.h>

#define ASCI_MAP_ENABLE

// Network and addresses
const char* NETWORK_ID = "AABBCCDD";       // Same network as other devices
const char* MASTER_ADDRESS = "MOB00001";   // Mobile/master address (this device)

// Targets to poll: these are the TAG devices (or fixed nodes) that will reply with distance
// Default values are placeholders - change to match your setup (8-char addresses)
const char* targetTagAddresses[3] = { "T1T1T1T1", "T2T2T2T2", "T3T3T3T3" };

// Anchor coordinates (meters) - change to match your installation
struct Point { double x; double y; };
// Changed mapping per richiesta: (0,0)=top-left, (6,0)=top-right, third anchor at bottom-right (6,4.5)
Point anchorPositions[3] = { {0.0, 0.0}, {5.3, 0.0}, {5.3, 3.65} };

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

// Received distances and flags (meters)
double anchorDistances[3] = { 0.0, 0.0, 0.0 };
bool anchorHave[3] = { false, false, false };
unsigned long anchorTimestamps[3] = {0,0,0};

// Trilateration result
double estimatedX = 0.0;
double estimatedY = 0.0;
bool havePosition = false;

// Polling state
int pollIndex = 0;
unsigned long lastPollTime = 0;
const unsigned long SEND_INTERVAL = 2500; // poll every 250ms -> each tag ~750ms (faster updates)

// Display / statistics
unsigned long packetsReceived = 0;

// Function declarations
void printStatusToSerial();
void printAsciiMap();
void onAnchorDataReceived(const char* tagAddress, int payloadLength, const char* data, int distanceCm, int rssi);
bool tryTrilateration();

// Print status and position to Serial (replaces OLED)
void printStatusToSerial() {
  Serial.println();
  Serial.println(F("---- STATUS ----"));
  Serial.print(F("Network: ")); Serial.println(NETWORK_ID);
  Serial.print(F("This (master) addr: ")); Serial.println(MASTER_ADDRESS);
  Serial.print(F("Packets received: ")); Serial.println(packetsReceived);
  for (int i=0;i<3;i++) {
    Serial.print(F("Anchor ")); Serial.print(i); Serial.print(F(" (")); Serial.print(targetTagAddresses[i]); Serial.print(F("): "));
    if (anchorHave[i]) {
      Serial.print(anchorDistances[i],3); Serial.print(F(" m"));
      unsigned long age = millis() - anchorTimestamps[i];
      Serial.print(F(" (age ")); Serial.print(age); Serial.println(F(" ms)"));
    } else {
      Serial.println(F(" --"));
    }
  }
  if (havePosition) {
    Serial.print(F("Estimated position: x=")); Serial.print(estimatedX,3);
    Serial.print(F(" m, y=")); Serial.print(estimatedY,3);
    Serial.println(F(" m"));
  } else {
    Serial.println(F("Estimated position: waiting..."));
  }
#ifdef ASCI_MAP_ENABLE
  printAsciiMap();
#endif
}

// Simple ASCII map: small fixed grid scaled to anchors extents
void printAsciiMap() {
  const int W = 40; const int H = 12; // ASCII grid
  double minx = anchorPositions[0].x, maxx = anchorPositions[0].x;
  double miny = anchorPositions[0].y, maxy = anchorPositions[0].y;
  for (int i=1;i<3;i++) { if (anchorPositions[i].x < minx) minx = anchorPositions[i].x; if (anchorPositions[i].x > maxx) maxx = anchorPositions[i].x; if (anchorPositions[i].y < miny) miny = anchorPositions[i].y; if (anchorPositions[i].y > maxy) maxy = anchorPositions[i].y; }
  if (havePosition) { if (estimatedX < minx) minx = estimatedX; if (estimatedX > maxx) maxx = estimatedX; if (estimatedY < miny) miny = estimatedY; if (estimatedY > maxy) maxy = estimatedY; }
  double padx = max(0.5, (maxx - minx) * 0.2);
  double pady = max(0.5, (maxy - miny) * 0.2);
  minx -= padx; maxx += padx; miny -= pady; maxy += pady;
  double sx = (double)(W-1) / max(1e-6, (maxx - minx));
  double sy = (double)(H-1) / max(1e-6, (maxy - miny));

  // create grid filled with spaces
  char grid[H][W+1];
  for (int y=0;y<H;y++) { for (int x=0;x<W;x++) grid[y][x] = ' '; grid[y][W] = '\0'; }

  // place anchors '1','2','3'
  for (int i=0;i<3;i++) {
    int gx = (int)((anchorPositions[i].x - minx) * sx + 0.5);
    int gy = (int)((anchorPositions[i].y - miny) * sy + 0.5);
    if (gx<0) gx=0; if (gx>=W) gx=W-1; if (gy<0) gy=0; if (gy>=H) gy=H-1;
    char label = '1' + i; grid[gy][gx] = label;
  }
  // place mobile 'X'
  if (havePosition) {
    int gx = (int)((estimatedX - minx) * sx + 0.5);
    int gy = (int)((estimatedY - miny) * sy + 0.5);
    if (gx<0) gx=0; if (gx>=W) gx=W-1; if (gy<0) gy=0; if (gy>=H) gy=H-1;
    grid[gy][gx] = 'X';
  }

  // Print bordered ASCII map
  Serial.println(F("--- ASCII MAP ---"));
  // top border
  Serial.print('+'); for (int i=0;i<W;i++) Serial.print('-'); Serial.println('+');
  // rows with side borders
  for (int y=0;y<H;y++) {
    Serial.print('|');
    Serial.print(grid[y]);
    Serial.println('|');
  }
  // bottom border
  Serial.print('+'); for (int i=0;i<W;i++) Serial.print('-'); Serial.println('+');
  Serial.println(F("--- END MAP ---"));
}

void onAnchorDataReceived(const char* tagAddress, int payloadLength, const char* data, int distanceCm, int rssi) {
  String addr = String(tagAddress);
  int idx = -1;
  for (int i=0;i<3;i++) if (addr.equalsIgnoreCase(String(targetTagAddresses[i]))) { idx = i; break; }
  if (idx < 0) {
    Serial.print(F("Received from unknown address: ")); Serial.println(addr);
    return;
  }

  // Simplified: convert cm to meters and store directly
  double dist_m = (double)distanceCm / 100.0; // cm -> meters
  anchorDistances[idx] = dist_m;
  anchorHave[idx] = true;
  anchorTimestamps[idx] = millis();
  packetsReceived++;

  Serial.println(F("\n--- Callback Triggered (Data Received) ---"));
  Serial.print(F("From Tag/Node: ")); Serial.println(targetTagAddresses[idx]);
  Serial.print(F("Payload: ")); Serial.println(data);
  Serial.print(F("Distance: ")); Serial.print(dist_m, 3); Serial.println(F(" m"));
  Serial.print(F("RSSI: ")); Serial.print(rssi); Serial.println(F(" dBm"));

  // Try compute position if we have at least three fresh distances
  if (anchorHave[0] && anchorHave[1] && anchorHave[2]) {
    if (tryTrilateration()) {
      Serial.print(F("Estimated position: x=")); Serial.print(estimatedX, 3);
      Serial.print(F(" m, y=")); Serial.print(estimatedY, 3); Serial.println(F(" m"));
      havePosition = true;
    } else {
      Serial.println(F("Trilateration failed (degenerate geometry?)"));
      havePosition = false;
    }
  }

  // print status to serial (replaces updateDisplay)
  printStatusToSerial();
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println(F("\n=== RYUW122 Positioning (mobile) - Heltec WiFi Kit 32 V2 ==="));

  // Initialize arrays and default buffers
  for (int i=0;i<3;i++) {
    anchorHave[i]=false; anchorDistances[i]=0.0; anchorTimestamps[i]=0;
  }
  havePosition = false;

  // Initialize RYUW122 module
  Serial.println(F("Init UWB module..."));
  if (!uwb.begin()) {
    Serial.println(F("Failed to initialize RYUW122 module"));
    Serial.println(F("UWB Init FAILED! Check wiring"));
    for(;;);
  }
  Serial.println(F("RYUW122 module OK"));
  delay(200);

  // Configure as ANCHOR (master)
  Serial.println(F("Configuring..."));
  if (!uwb.setMode(RYUW122Mode::ANCHOR)) {
    Serial.println(F("Failed to set ANCHOR mode"));
  }
  Serial.println(F("Mode: ANCHOR (MASTER)"));

  // Set Network ID
  uwb.setNetworkId(NETWORK_ID);
  Serial.print(F("Network ID: ")); Serial.println(NETWORK_ID);

  // Set this device address (MASTER)
  uwb.setAddress(MASTER_ADDRESS);
  Serial.print(F("Master Address: ")); Serial.println(MASTER_ADDRESS);

  // Register callback that receives +ANCHOR_RCV
  uwb.onAnchorReceive(onAnchorDataReceived);
  Serial.println(F("Callback registered"));

  Serial.println(F("READY"));
  Serial.print(F("Net: ")); Serial.println(NETWORK_ID);
  Serial.print(F("Addr: ")); Serial.println(MASTER_ADDRESS);

  lastPollTime = millis();
}

void loop() {
  // Poll next anchor periodically
  if (millis() - lastPollTime >= SEND_INTERVAL) {
    const char* payload = "POLL";
    const char* target = targetTagAddresses[pollIndex];
    Serial.print(F("\n[MASTER] Polling Tag/Node: ")); Serial.println(target);
    uwb.anchorSendData(target, strlen(payload), payload);
    pollIndex = (pollIndex + 1) % 3;
    lastPollTime = millis();
  }

  // Process incoming data and trigger callbacks
  uwb.loop();

  delay(10);
}

// Simple 2D trilateration using three circle intersections (linearized form)
bool tryTrilateration() {
  double x1 = anchorPositions[0].x; double y1 = anchorPositions[0].y; double r1 = anchorDistances[0];
  double x2 = anchorPositions[1].x; double y2 = anchorPositions[1].y; double r2 = anchorDistances[1];
  double x3 = anchorPositions[2].x; double y3 = anchorPositions[2].y; double r3 = anchorDistances[2];

  if (r1 <= 0 || r2 <= 0 || r3 <= 0) return false;

  double A = 2*(x2 - x1);
  double B = 2*(y2 - y1);
  double C = r1*r1 - r2*r2 - x1*x1 + x2*x2 - y1*y1 + y2*y2;

  double D = 2*(x3 - x2);
  double E = 2*(y3 - y2);
  double F = r2*r2 - r3*r3 - x2*x2 + x3*x3 - y2*y2 + y3*y3;

  double denom = A*E - B*D;
  if (fabs(denom) < 1e-9) return false;

  double x = (C*E - B*F) / denom;
  double y = (A*F - C*D) / denom;

  estimatedX = x; estimatedY = y; return true;
}

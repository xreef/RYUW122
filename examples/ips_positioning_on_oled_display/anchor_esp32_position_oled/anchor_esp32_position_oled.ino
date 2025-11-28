/**
 * @file anchor_esp32_position_oled.cpp
 * @author Renzo Mischianti (modified)
 * @brief RYUW122 MASTER ANCHOR (mobile) example for ESP32 with OLED display
 * @version 1.0.0
 * @date 2025-11-24
 *
 * This sketch configures the esp32 as a MASTER/ANCHOR (mobile) that sequentially
 * polls three fixed anchors, collects distances returned by them and computes
 * the device (mobile) position by trilateration. Results are shown on the OLED
 * and sent to Serial.
 *
 * Hardware connections:
 * - RYUW122 TX -> GPIO18 (RX2) -- ensure this pin does NOT conflict with OLED RST
 * - RYUW122 RX -> GPIO17 (TX2)
 * - RYUW122 NRST -> GPIO13 (optional reset)
 * - OLED is on built-in I2C (SDA_OLED 15, SCL_OLED 4, RST_OLED 16)
 *
 * Notes / assumptions:
 * - Anchors addresses and coordinates must match the physical setup. By default
 *   we assume three anchors placed in a small room for testing. Change
 *   `anchorAddresses` and `anchorPositions` to match your installation.
 * - Distances reported by the module come in centimeters; converted to meters
 *   for trilateration.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RYUW122.h>

// OLED pin definitions
#ifndef SDA_OLED
#define SDA_OLED 4
#endif
#ifndef SCL_OLED
#define SCL_OLED 15
#endif
#ifndef RST_OLED
#define RST_OLED 16
#endif

// OLED Display configuration
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     RST_OLED
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// RYUW122 UWB Module configuration
// Use Serial2 on TX2=GPIO17, RX2=GPIO18 (user moved RX to 18 to avoid conflict with OLED RST)
#define RX_PIN 18
#define TX_PIN 17
#define RESET_PIN 13  // Connect to RYUW122 NRST (active LOW)

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

// Initialize RYUW122 library with Serial2
// Constructor: RYUW122(mcuTxPin, mcuRxPin, serial, lowResetTriggerInputPin)
RYUW122 ryuw122(TX_PIN, RX_PIN, &Serial2, RESET_PIN);

// Received distances and flags (meters)
double anchorDistances[3] = { 0.0, 0.0, 0.0 };
bool anchorHave[3] = { false, false, false };
unsigned long anchorTimestamps[3] = {0,0,0};

// Filtering buffers
#define MEDIAN_WINDOW 3
double distBuffers[3][MEDIAN_WINDOW];
int distIdx[3] = {0,0,0};
int sampleCount[3] = {0,0,0};
double smoothedDistances[3] = {0.0,0.0,0.0};
const double SMOOTH_ALPHA = 0.6; // exponential smoothing alpha (increased for faster response)

// Trilateration result
double estimatedX = 0.0;
double estimatedY = 0.0;
bool havePosition = false;

// Polling state
int pollIndex = 0;
unsigned long lastPollTime = 0;
const unsigned long SEND_INTERVAL = 250; // poll every 250ms -> each tag ~750ms (faster updates)

// Display / statistics
unsigned long packetsReceived = 0;

// Function declarations
void updateDisplay();
void onAnchorDataReceived(const char* tagAddress, int payloadLength, const char* data, int distanceCm, int rssi);
bool tryTrilateration();
double medianOfBuffer(double *buf, int len);

// Draw map and status on OLED
void updateDisplay() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0,0);
  // Title removed to enlarge map area as requested

  // show anchors status in a small column on the left above map if needed
  display.setCursor(0, 0);
  for (int i=0;i<3;i++) {
    display.print(F("T")); display.print(i+1); display.print(F(":"));
    if (anchorHave[i]) {
      display.print(anchorDistances[i], 2);
      // display.print(F("m"));
    } else {
      display.print(F(" -- "));
    }
  }

  // map area enlarged and placed top-left (below status)
  const int MX = 118; // map width (almost full screen)
  const int MY = 50;  // map height
  const int MX0 = 4;  const int MY0 = 12; // map origin (leave small margin for status)

  // compute bounds based on anchors and optional estimated point
  double minx = anchorPositions[0].x, maxx = anchorPositions[0].x;
  double miny = anchorPositions[0].y, maxy = anchorPositions[0].y;
  for (int i=1;i<3;i++) {
    if (anchorPositions[i].x < minx) minx = anchorPositions[i].x;
    if (anchorPositions[i].x > maxx) maxx = anchorPositions[i].x;
    if (anchorPositions[i].y < miny) miny = anchorPositions[i].y;
    if (anchorPositions[i].y > maxy) maxy = anchorPositions[i].y;
  }
  if (havePosition) {
    if (estimatedX < minx) minx = estimatedX;
    if (estimatedX > maxx) maxx = estimatedX;
    if (estimatedY < miny) miny = estimatedY;
    if (estimatedY > maxy) maxy = estimatedY;
  }

  // Ensure minx/miny correspond to top-left semantics: y increases downward on display already
  double padx = max(0.5, (maxx - minx) * 0.2);
  double pady = max(0.5, (maxy - miny) * 0.2);
  minx -= padx; maxx += padx; miny -= pady; maxy += pady;
  double scalex = (double)MX / max(1e-6, (maxx - minx));
  double scaley = (double)MY / max(1e-6, (maxy - miny));

  // draw map box
  display.drawRect(MX0-1, MY0-1, MX+2, MY+2, SSD1306_WHITE);

  // draw anchors (coordinates mapped with origin top-left)
  for (int i=0;i<3;i++) {
    int px = MX0 + (int)((anchorPositions[i].x - minx) * scalex);
    int py = MY0 + (int)((anchorPositions[i].y - miny) * scaley);
    // Clamp to map box
    if (px < MX0) px = MX0; if (px > MX0+MX-1) px = MX0+MX-1;
    if (py < MY0) py = MY0; if (py > MY0+MY-1) py = MY0+MY-1;
    display.fillCircle(px, py, 3, SSD1306_WHITE);
    // label near the anchor
    display.setCursor(px+5, py-4);
    display.print(i+1);
  }

  // draw mobile position
  if (havePosition) {
    int px = MX0 + (int)((estimatedX - minx) * scalex);
    int py = MY0 + (int)((estimatedY - miny) * scaley);
    if (px < MX0) px = MX0; if (px > MX0+MX-1) px = MX0+MX-1;
    if (py < MY0) py = MY0; if (py > MY0+MY-1) py = MY0+MY-1;
    display.fillCircle(px, py, 3, SSD1306_WHITE);
  }

  // show position text at bottom
  display.setCursor(0, MY0 + MY + 2);
  if (havePosition) {
    display.print(F("Pos: "));
    display.print(estimatedX,2); display.print(F(",")); display.print(estimatedY,2);
  } else {
    display.print(F("Pos: waiting..."));
  }

  display.display();
}

void onAnchorDataReceived(const char* tagAddress, int payloadLength, const char* data, int distanceCm, int rssi) {
  String addr = String(tagAddress);
  int idx = -1;
  for (int i=0;i<3;i++) if (addr.equalsIgnoreCase(String(targetTagAddresses[i]))) { idx = i; break; }
  if (idx < 0) {
    Serial.print(F("Received from unknown address: ")); Serial.println(addr);
    return;
  }

  double dist_m = (double)distanceCm / 100.0; // cm -> meters
  // push into circular buffer
  distBuffers[idx][ distIdx[idx] ] = dist_m;
  distIdx[idx] = (distIdx[idx] + 1) % MEDIAN_WINDOW;
  if (sampleCount[idx] < MEDIAN_WINDOW) sampleCount[idx]++;

  // compute median using only valid samples
  double median = medianOfBuffer(distBuffers[idx], sampleCount[idx]);
  // exponential smoothing (use previous smoothed value, but if first sample use median directly)
  if (sampleCount[idx] == 1 && smoothedDistances[idx] <= 0.0) {
    smoothedDistances[idx] = median; // initialize smoothing to first median
  } else {
    smoothedDistances[idx] = SMOOTH_ALPHA * median + (1.0 - SMOOTH_ALPHA) * smoothedDistances[idx];
  }

  anchorDistances[idx] = smoothedDistances[idx];
  anchorHave[idx] = true;
  anchorTimestamps[idx] = millis();
  packetsReceived++;

  Serial.println(F("\n--- Callback Triggered (Data Received) ---"));
  Serial.print(F("From Tag/Node: ")); Serial.println(targetTagAddresses[idx]);
  Serial.print(F("Payload: ")); Serial.println(data);
  Serial.print(F("Raw distance: ")); Serial.print(dist_m, 3); Serial.println(F(" m"));
  Serial.print(F("Median: ")); Serial.print(median,3); Serial.println(F(" m"));
  Serial.print(F("Smoothed: ")); Serial.print(smoothedDistances[idx],3); Serial.println(F(" m"));
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

  updateDisplay();
}

// Helper: compute median of small buffer (copied into temp array)
double medianOfBuffer(double *buf, int len) {
  if (len <= 0) return 0.0;
  double tmp[MEDIAN_WINDOW];
  for (int i=0;i<len;i++) tmp[i] = buf[i];
  // simple insertion sort
  for (int i=1;i<len;i++) {
    double v = tmp[i]; int j = i-1;
    while (j>=0 && tmp[j] > v) { tmp[j+1] = tmp[j]; j--; }
    tmp[j+1] = v;
  }
  if (len % 2 == 1) return tmp[len/2];
  return (tmp[len/2 - 1] + tmp[len/2]) / 2.0;
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println(F("\n=== RYUW122 Positioning (mobile) - ESP32 anchor ==="));

  // Initialize OLED display
  Wire.begin(SDA_OLED, SCL_OLED);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("RYUW122 Positioning"));
  display.println(F("Initializing..."));
  display.display();
  delay(200);

  // Initialize arrays and default buffers
  for (int i=0;i<3;i++) {
    anchorHave[i]=false; anchorDistances[i]=0.0; anchorTimestamps[i]=0;
    smoothedDistances[i]=0.0; distIdx[i]=0; sampleCount[i]=0;
    for (int j=0;j<MEDIAN_WINDOW;j++) distBuffers[i][j] = 0.0;
  }
  havePosition = false;

  // Initialize RYUW122 module
  display.clearDisplay(); display.setCursor(0,0); display.println(F("Init UWB module...")); display.display();
  if (!ryuw122.begin()) {
    Serial.println(F("Failed to initialize RYUW122 module"));
    display.clearDisplay(); display.setCursor(0,0);
    display.println(F("UWB Init FAILED!")); display.println(F("Check wiring")); display.display();
    for(;;);
  }
  Serial.println(F("RYUW122 module OK"));
  delay(200);

  // Configure as ANCHOR (master)
  display.clearDisplay(); display.setCursor(0,0); display.println(F("Configuring...")); display.display();
  if (!ryuw122.setMode(RYUW122Mode::ANCHOR)) {
    Serial.println(F("Failed to set ANCHOR mode"));
  }
  Serial.println(F("Mode: ANCHOR (MASTER)"));

  // Set Network ID
  ryuw122.setNetworkId(NETWORK_ID);
  Serial.print(F("Network ID: ")); Serial.println(NETWORK_ID);

  // Set this device address (MASTER)
  ryuw122.setAddress(MASTER_ADDRESS);
  Serial.print(F("Master Address: ")); Serial.println(MASTER_ADDRESS);

  // Register callback that receives +ANCHOR_RCV
  ryuw122.onAnchorReceive(onAnchorDataReceived);
  Serial.println(F("Callback registered"));

  // Final display
  display.clearDisplay();
  display.setCursor(0,0);
  display.println(F("READY"));
  display.println();
  display.print(F("Net: ")); display.println(NETWORK_ID);
  display.print(F("Addr: ")); display.println(MASTER_ADDRESS);
  display.display();

  lastPollTime = millis();
}

void loop() {
  // Poll next anchor periodically
  if (millis() - lastPollTime >= SEND_INTERVAL) {
    const char* payload = "POLL";
    const char* target = targetTagAddresses[pollIndex];
    Serial.print(F("\n[MASTER] Polling Tag/Node: ")); Serial.println(target);
    ryuw122.anchorSendData(target, strlen(payload), payload);
    pollIndex = (pollIndex + 1) % 3;
    lastPollTime = millis();
  }

  // Process incoming data and trigger callbacks
  ryuw122.loop();

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

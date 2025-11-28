// Master_Mobile_Anchor_Heltec.ino
// Example: RYUW122 Master/Mobile Anchor for Heltec WiFi Kit 32 V2
// This example polls three TAGs to obtain distances and computes its position
// using trilateration. Display output on SSD1306 OLED.

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RYUW122.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     RST_OLED
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define RX_PIN 18
#define TX_PIN 17
#define RESET_PIN 13

const char* NETWORK_ID = "AABBCCDD";
const char* MASTER_ADDRESS = "MOB00001";
const char* targetTagAddresses[3] = { "T1T1T1T1", "T2T2T2T2", "T3T3T3T3" };

struct Point { double x; double y; };
Point anchorPositions[3] = { {0.0,0.0}, {5.0,0.0}, {0.0,3.0} };

RYUW122 ryuw122(TX_PIN, RX_PIN, &Serial2, RESET_PIN);

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_OLED, SCL_OLED);
  display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0,0);
  display.println(F("RYUW122 Master Anchor"));
  display.display();

  if (!ryuw122.begin()) {
    Serial.println(F("UWB init failed"));
    while(1);
  }
  ryuw122.setMode(RYUW122Mode::ANCHOR);
  ryuw122.setNetworkId(NETWORK_ID);
  ryuw122.setAddress(MASTER_ADDRESS);

  Serial.println(F("Master configured"));
}

void loop() {
  // Basic demo: poll TAGs one by one
  const char* payload = "POLL";
  for (int i=0;i<3;i++) {
    ryuw122.anchorSendData(targetTagAddresses[i], strlen(payload), payload);
    delay(300);
    ryuw122.loop();
  }
  delay(1000);
}


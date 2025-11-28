/*
  Master_Mobile_Anchor.ino
  RYUW122 library example: Heltec WiFi Kit 32 V2 as a mobile MASTER/ANCHOR
  Polls three fixed anchors, collects distances and computes 2D position
  (trilateration). Shows distances and computed position on OLED and Serial.
  Language: English
*/

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <RYUW122.h>

// Fallback defines for Heltec boards (override from board variant if present)
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

// RYUW122 UWB Module pins (use Serial2)
#define RX_PIN 18
#define TX_PIN 17
#define RESET_PIN 13  // Connect to RYUW122 NRST (active LOW)

// Network and addresses
const char* NETWORK_ID = "AABBCCDD";
const char* MASTER_ADDRESS = "MOB00001";   // Mobile master address

// Fixed anchors to poll (their AT addresses)
const char* anchorAddresses[3] = { "A1A1A1A1", "A2A2A2A2", "A3A3A3A3" };

// Anchor coordinates (meters) - change to match your installation
struct Point { double x; double y; };
Point anchorPositions[3] = { {0.0, 0.0}, {5.0, 0.0}, {0.0, 3.0} };

// Initialize RYUW122 with Serial2
RYUW122 ryuw122(TX_PIN, RX_PIN, &Serial2, RESET_PIN);

// State
double anchorDistances[3] = {0,0,0};
bool anchorHave[3] = {false,false,false};
unsigned long anchorTimestamps[3] = {0,0,0};

double estimatedX=0, estimatedY=0; bool havePosition=false;
int pollIndex=0; unsigned long lastPollTime=0; const unsigned long SEND_INTERVAL=1000;
unsigned long packetsReceived = 0;

void updateDisplay();
void onAnchorDataReceived(const char* tagAddress, int payloadLength, const char* data, int distanceCm, int rssi);
bool tryTrilateration();

void onAnchorDataReceived(const char* tagAddress, int payloadLength, const char* data, int distanceCm, int rssi) {
  String addr = String(tagAddress);
  int idx=-1;
  for (int i=0;i<3;i++) if (addr.equalsIgnoreCase(String(anchorAddresses[i]))) { idx=i; break; }
  if (idx<0) { Serial.print(F("Received from unknown address: ")); Serial.println(addr); return; }

  double d = (double)distanceCm/100.0;
  anchorDistances[idx]=d; anchorHave[idx]=true; anchorTimestamps[idx]=millis(); packetsReceived++;

  Serial.println(F("\n--- Callback Triggered (Data Received) ---"));
  Serial.print(F("From Anchor: ")); Serial.println(anchorAddresses[idx]);
  Serial.print(F("Payload: ")); Serial.println(data);
  Serial.print(F("Distance: ")); Serial.print(d,3); Serial.println(F(" m"));
  Serial.print(F("RSSI: ")); Serial.print(rssi); Serial.println(F(" dBm"));

  if (anchorHave[0] && anchorHave[1] && anchorHave[2]) {
    if (tryTrilateration()) { havePosition=true; Serial.print(F("Estimated position: x=")); Serial.print(estimatedX,3); Serial.print(F(" m, y=")); Serial.println(estimatedY,3); }
    else { havePosition=false; Serial.println(F("Trilateration failed")); }
  }

  updateDisplay();
}

void setup(){
  Serial.begin(115200);
  delay(50);
  Serial.println(F("Master Mobile Anchor example"));

  // OLED init
  Wire.begin(SDA_OLED, SCL_OLED);
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)){
    Serial.println(F("SSD1306 allocation failed")); while(1) delay(1000);
  }
  display.clearDisplay(); display.setTextSize(1); display.setTextColor(SSD1306_WHITE); display.setCursor(0,0);
  display.println(F("RYUW122 MASTER ANCHOR")); display.println(F("Initializing...")); display.display();
  delay(200);

  for (int i=0;i<3;i++){ anchorHave[i]=false; anchorDistances[i]=0; anchorTimestamps[i]=0; }
  havePosition=false;

  display.clearDisplay(); display.setCursor(0,0); display.println(F("Init UWB module...")); display.display();
  if (!ryuw122.begin()){
    Serial.println(F("Failed to initialize RYUW122 module")); display.clearDisplay(); display.println(F("UWB Init FAILED")); display.display(); while(1) delay(1000);
  }
  Serial.println(F("RYUW122 OK")); delay(100);

  display.clearDisplay(); display.setCursor(0,0); display.println(F("Configuring...")); display.display();
  ryuw122.setMode(RYUW122Mode::ANCHOR);
  ryuw122.setNetworkId(NETWORK_ID);
  ryuw122.setAddress(MASTER_ADDRESS);
  ryuw122.onAnchorReceive(onAnchorDataReceived);

  display.clearDisplay(); display.setCursor(0,0);
  display.println(F("MASTER ANCHOR READY")); display.print(F("Net: ")); display.println(NETWORK_ID);
  display.print(F("Addr: ")); display.println(MASTER_ADDRESS); display.display();

  lastPollTime=millis();
}

void loop(){
  if (millis()-lastPollTime>=SEND_INTERVAL){
    const char* payload = "POLL";
    const char* target = anchorAddresses[pollIndex];
    Serial.print(F("\n[MASTER] Polling Anchor: ")); Serial.println(target);
    ryuw122.anchorSendData(target, strlen(payload), payload);
    pollIndex = (pollIndex+1)%3; lastPollTime=millis();
  }
  ryuw122.loop();
  delay(10);
}

bool tryTrilateration(){
  double x1=anchorPositions[0].x,y1=anchorPositions[0].y,r1=anchorDistances[0];
  double x2=anchorPositions[1].x,y2=anchorPositions[1].y,r2=anchorDistances[1];
  double x3=anchorPositions[2].x,y3=anchorPositions[2].y,r3=anchorDistances[2];
  if (r1<=0||r2<=0||r3<=0) return false;
  double A=2*(x2-x1), B=2*(y2-y1), C=r1*r1-r2*r2 - x1*x1 + x2*x2 - y1*y1 + y2*y2;
  double D=2*(x3-x2), E=2*(y3-y2), F=r2*r2-r3*r3 - x2*x2 + x3*x3 - y2*y2 + y3*y3;
  double denom = A*E - B*D; if (fabs(denom)<1e-9) return false;
  double x=(C*E - B*F)/denom; double y=(A*F - C*D)/denom; estimatedX=x; estimatedY=y; return true;
}

void updateDisplay(){
  display.clearDisplay(); display.setTextSize(1); display.setCursor(0,0);
  display.println(F("MASTER ANCHOR")); display.print(F("Addr: ")); display.println(MASTER_ADDRESS);
  display.drawLine(0,12,SCREEN_WIDTH,12,SSD1306_WHITE);
  for (int i=0;i<3;i++){
    display.setCursor(0,14+i*12);
    display.print(F("A")); display.print(i+1); display.print(F(":")); display.print(anchorAddresses[i]); display.print(F(" "));
    if (anchorHave[i]){ display.print(anchorDistances[i],2); display.print(F("m")); } else display.print(F("--"));
  }
  display.setCursor(0,52);
  if (havePosition) { display.print(F("Pos: ")); display.print(estimatedX,2); display.print(F(",")); display.print(estimatedY,2)); }
  else display.print(F("Pos: waiting..."));
  display.display();
}


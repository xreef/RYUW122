// Basic_Tag_ESP32C3.ino
// Simple TAG example for RYUW122 library (ESP32-C3)

#include <RYUW122.h>

#define RX_PIN 5
#define TX_PIN 4
#define RESET_PIN 6

const char* NETWORK_ID = "AABBCCDD";
const char* TAG_ADDRESS = "T1T1T1T1";

RYUW122 ryuw122(TX_PIN, RX_PIN, &Serial1, RESET_PIN);

void onTagDataReceived(int payloadLength, const char* data, int rssi) {
  Serial.println(F("Tag received data:"));
  Serial.println(data);
  const char* response = "PONG";
  ryuw122.tagSendData(strlen(response), response);
}

void setup() {
  Serial.begin(115200);
  while (!Serial) { delay(10); }

  if (!ryuw122.begin()) while(1);

  ryuw122.setMode(RYUW122Mode::TAG);
  ryuw122.setNetworkId(NETWORK_ID);
  ryuw122.setAddress(TAG_ADDRESS);
  ryuw122.onTagReceive(onTagDataReceived);
}

void loop() {
  ryuw122.loop();
  delay(10);
}


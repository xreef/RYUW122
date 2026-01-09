/**
 * @file 01_Master_Anchor_esp32devkitv4.ino
 * @author Renzo Mischianti
 * @brief UWB Master Anchor for ESP32, initiating polling via MQTT.
 * @version 1.0.0
 * @date 2024-05-23
 *
 * This sketch configures an ESP32 as the "Master" Anchor in a standard positioning architecture.
 * More info and reference projects at https://mischianti.org/category/my-libraries/ryuw122-uwb/
 *
 * Its roles are:
 * 1. Connect to Wi-Fi and an MQTT broker.
 * 2. Periodically publish a JSON "poll request" to a shared MQTT topic to initiate the ranging process.
 * 3. As an anchor, it also listens for its own poll request and polls the Tag.
 * 4. Publish the measured distance to a dedicated MQTT topic.
 *
 * @copyright Copyright (c) 2024 Renzo Mischianti - www.mischianti.org
 *
 */

#include <RYUW122.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#define PUBLISH_JSON_POLL_REQUEST

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

const char* NETWORK_ID = "AABBCCDD";
const char* ANCHOR_ADDRESS = "MA";
const char* TARGET_TAG = "T1T1T1T1"; // The Tag to be polled

// --- WiFi Configuration ---
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// --- MQTT Configuration ---
const char* MQTT_USER = "your_mqtt_user";
const char* MQTT_PASSWORD = "your_mqtt_password";
const char* mqtt_server = "your_mqtt_broker_ip"; // e.g., "192.168.1.100"
const int MQTT_PORT = 1883;

const char* MQTT_CLIENT_ID = "Anchor_MA";
const char* MQTT_TOPIC_POLL_REQUEST = "uwb/trilateration/poll_request";
const char* MQTT_TOPIC_DISTANCE = "uwb/trilateration/distance";

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

// --- Synchronization ---
const unsigned long POLLING_INTERVAL_MS = 2000;
unsigned long lastPollRequestTime = 0;
unsigned long currentPollId = 0;

void pollTag(const char* tagAddress) {
    Serial.print(F("\nReceived poll request. Polling Tag: "));
    Serial.println(tagAddress);
    const char* pollMessage = "POLL";
    uwb.anchorSendData(tagAddress, strlen(pollMessage), pollMessage);
}

/**
 * @brief MQTT callback. Parses the JSON poll request.
 */
void mqttCallback(char* topic, byte* payload, unsigned int length) {
    if (strcmp(topic, MQTT_TOPIC_POLL_REQUEST) == 0) {
        Serial.println("Poll request received.");
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, payload, length);

        if (error) {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            return;
        }

        currentPollId = doc["poll_id"];
        const char* tagAddress = doc["tag_address"];

        if (tagAddress) {
            pollTag(tagAddress);
        } else {
            Serial.println("Error: tag_address not found in poll request.");
        }
    }
}

/**
 * @brief UWB callback. Publishes results with the poll_id.
 */
void onAnchorDataReceived(const char* tagAddress, int payloadLength, const char* data, int distance, int rssi) {
    Serial.println(F("--- Distance Response Received ---"));
    Serial.print(F("From Tag: ")); Serial.println(tagAddress);
    Serial.print(F("Distance: ")); Serial.print(distance); Serial.println(F(" cm"));
    Serial.print(F("RSSI: ")); Serial.print(rssi); Serial.println(F(" dBm"));

    char jsonPayload[150];
    snprintf(jsonPayload, sizeof(jsonPayload),
             "{\"tag\":\"%s\", \"anchor\":\"%s\", \"distance_cm\":%d, \"rssi\":%d, \"poll_id\":%lu}",
             tagAddress, ANCHOR_ADDRESS, distance, rssi, currentPollId);

    if (mqttClient.publish(MQTT_TOPIC_DISTANCE, jsonPayload)) {
        Serial.print(F("Published to MQTT: ")); Serial.println(jsonPayload);
    } else {
        Serial.println(F("Failed to publish distance to MQTT."));
    }
}

void setupWifi() {
    Serial.print(F("Connecting to WiFi: ")); Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(F("."));
    }
    Serial.println(F("\nWiFi connected."));
    Serial.print(F("IP Address: ")); Serial.println(WiFi.localIP());
}

void reconnectMqtt() {
    while (!mqttClient.connected()) {
        Serial.print(F("Attempting MQTT connection..."));
        if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD)) {
            Serial.println(F("connected"));
            mqttClient.subscribe(MQTT_TOPIC_POLL_REQUEST);
            Serial.print(F("Subscribed to: ")); Serial.println(MQTT_TOPIC_POLL_REQUEST);
        } else {
            Serial.print(F("failed, rc="));
            Serial.print(mqttClient.state());
            Serial.println(F(" try again in 5 seconds"));
            delay(5000);
        }
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println(F("RYUW122 Dynamic ANCHOR (MASTER)"));

    if (!uwb.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }
    uwb.setMode(RYUW122Mode::ANCHOR);
    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(ANCHOR_ADDRESS);
    uwb.onAnchorReceive(onAnchorDataReceived);

    setupWifi();
    mqttClient.setServer(mqtt_server, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
}

void loop() {
    if (!mqttClient.connected()) {
        reconnectMqtt();
    }
    mqttClient.loop();

#ifdef PUBLISH_JSON_POLL_REQUEST
    // As the Master Anchor, periodically publish the JSON poll request
    if (millis() - lastPollRequestTime >= POLLING_INTERVAL_MS) {
        lastPollRequestTime = millis();

        JsonDocument doc;
        doc["poll_id"] = lastPollRequestTime;
        doc["tag_address"] = TARGET_TAG;

        char jsonBuffer[128];
        serializeJson(doc, jsonBuffer);

        Serial.print("\nSending JSON Poll Request: ");
        Serial.println(jsonBuffer);
        mqttClient.publish(MQTT_TOPIC_POLL_REQUEST, jsonBuffer);
    }
#endif

    uwb.loop();
}

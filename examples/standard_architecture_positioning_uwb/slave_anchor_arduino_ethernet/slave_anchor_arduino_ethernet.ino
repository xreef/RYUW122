/**
 * @file slave_anchor_arduino_ethernet.ino
 * @author Renzo Mischianti
 * @brief UWB Slave Anchor for Arduino Mega with Ethernet.
 * @version 1.0.0
 * @date 2024-05-23
 *
 * This sketch configures an Arduino Mega as a "Slave" Anchor.
 * More info and reference projects at https://mischianti.org/category/my-libraries/ryuw122-uwb/
 *
 * Its roles are:
 * 1. Connect to the network via an Ethernet shield.
 * 2. Subscribe to a shared MQTT topic to listen for "poll requests" from the Master Anchor.
 * 3. When a request is received, it parses the JSON, polls the specified Tag, and publishes the result.
 *
 * @copyright Copyright (c) 2024 Renzo Mischianti - www.mischianti.org
 *
 */

#include <RYUW122.h>
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// --- UWB Configuration ---
RYUW122 uwb(&Serial1); // Using Serial1 on Arduino Mega

const char* NETWORK_ID = "AABBCCDD";
const char* ANCHOR_ADDRESS = "SA1"; // Unique address for this Slave Anchor

// --- Ethernet & MQTT Configuration ---
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED }; // Must be unique on your network
// IPAddress ip(192, 168, 1, 177); // Optional: uncomment for a static IP

const char* MQTT_USER = "your_mqtt_user";
const char* MQTT_PASSWORD = "your_mqtt_password";
const char* mqtt_server = "your_mqtt_broker_ip"; // e.g., "192.168.1.100"
const int MQTT_PORT = 1883;

const char* MQTT_CLIENT_ID = "Anchor_SA1";
const char* MQTT_TOPIC_POLL_REQUEST = "uwb/trilateration/poll_request";
const char* MQTT_TOPIC_DISTANCE = "uwb/trilateration/distance";

EthernetClient ethClient;
PubSubClient mqttClient(ethClient);

// --- Synchronization ---
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
    Serial.println(F("RYUW122 Dynamic ANCHOR (Slave, Ethernet)"));

    if (!uwb.begin()) {
        Serial.println(F("Failed to initialize RYUW122 module. Halting."));
        while (1);
    }
    uwb.setMode(RYUW122Mode::ANCHOR);
    uwb.setNetworkId(NETWORK_ID);
    uwb.setAddress(ANCHOR_ADDRESS);
    uwb.onAnchorReceive(onAnchorDataReceived);

    Ethernet.init(10); // CS Pin for Ethernet Shield, e.g., 10 for official shield
    Serial.println(F("Initializing Ethernet..."));
    if (Ethernet.begin(mac) == 0) {
        Serial.println(F("Failed to configure Ethernet using DHCP."));
        // If DHCP fails, you might need to use a static IP.
        // Ethernet.begin(mac, ip);
    }
    delay(2000);

    if (Ethernet.hardwareStatus() == EthernetNoHardware || Ethernet.linkStatus() == LinkOFF) {
        Serial.println(F("Ethernet hardware not found or cable not connected. Halting."));
        while (true);
    }
    Serial.print(F("Ethernet IP: ")); Serial.println(Ethernet.localIP());

    mqttClient.setServer(mqtt_server, MQTT_PORT);
    mqttClient.setCallback(mqttCallback);
}

void loop() {
    if (!mqttClient.connected()) {
        reconnectMqtt();
    }
    mqttClient.loop(); // Handles incoming MQTT messages
    uwb.loop();        // Handles incoming UWB messages
}

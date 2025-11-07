/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * RYUW122 Arduino library implementation
 */

#include "RYUW122.h"


RYUW122::RYUW122(Stream* serial) : st(serial) {}

#ifdef ACTIVATE_SOFTWARE_SERIAL
RYUW122::RYUW122(byte txModulePin, byte rxModulePin, RYUW122BaudRate bpsRate){
    this->txModulePin = txModulePin;
    this->rxModulePin = rxModulePin;
    SoftwareSerial* mySerial = new SoftwareSerial((uint8_t)this->txModulePin, (uint8_t)this->rxModulePin); // "RX TX" // @suppress("Abstract class cannot be instantiated")
    this->ss = mySerial;
    this->hs = nullptr;

    this->bpsRate = bpsRate;
}
RYUW122::RYUW122(byte txModulePin, byte rxModulePin, byte nodeIndicatorPin, RYUW122BaudRate bpsRate){
    this->txModulePin = txModulePin;
    this->rxModulePin = rxModulePin;
    this->nodeIndicatorPin = nodeIndicatorPin;
    SoftwareSerial* mySerial = new SoftwareSerial((uint8_t)this->txModulePin, (uint8_t)this->rxModulePin); // "RX TX" // @suppress("Abstract class cannot be instantiated")
    this->ss = mySerial;
    this->hs = nullptr;

    this->bpsRate = bpsRate;
}
RYUW122::RYUW122(byte txModulePin, byte rxModulePin, byte nodeIndicatorPin, byte lowResetTriggerInputPin,   RYUW122BaudRate bpsRate){
    this->txModulePin = txModulePin;
    this->rxModulePin = rxModulePin;

    this->nodeIndicatorPin = nodeIndicatorPin;

    this->lowResetTriggerInputPin = lowResetTriggerInputPin;

    SoftwareSerial* mySerial = new SoftwareSerial((uint8_t)this->txModulePin, (uint8_t)this->rxModulePin); // "RX TX" // @suppress("Abstract class cannot be instantiated")
    this->ss = mySerial;
    this->hs = nullptr;

    this->bpsRate = bpsRate;
}
#endif

RYUW122::RYUW122(HardwareSerial* serial, RYUW122BaudRate bpsRate){ //, uint32_t serialConfig
    // leave pins as default (-1) unless provided by other ctors
#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif
    this->hs = serial;
    this->bpsRate = bpsRate;
}
RYUW122::RYUW122(HardwareSerial* serial, byte nodeIndicatorPin, RYUW122BaudRate bpsRate){ // , uint32_t serialConfig
    this->nodeIndicatorPin = nodeIndicatorPin;
#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif
    this->hs = serial;
    this->bpsRate = bpsRate;
}
RYUW122::RYUW122(HardwareSerial* serial, byte nodeIndicatorPin, byte lowResetTriggerInputPin,   RYUW122BaudRate bpsRate){ //, uint32_t serialConfig
    this->nodeIndicatorPin = nodeIndicatorPin;
    this->lowResetTriggerInputPin = lowResetTriggerInputPin;

#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif
    this->hs = serial;
    this->bpsRate = bpsRate;
}

#ifdef HARDWARE_SERIAL_SELECTABLE_PIN
RYUW122::RYUW122(byte txModulePin, byte rxModulePin, HardwareSerial* serial, RYUW122BaudRate bpsRate, uint32_t serialConfig){
    this->txModulePin = txModulePin;
    this->rxModulePin = rxModulePin;

#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif

    this->serialConfig = serialConfig;

    this->hs = serial;

    this->bpsRate = bpsRate;
}
RYUW122::RYUW122(byte txModulePin, byte rxModulePin, HardwareSerial* serial, byte nodeIndicatorPin, RYUW122BaudRate bpsRate, uint32_t serialConfig){
    this->txModulePin = txModulePin;
    this->rxModulePin = rxModulePin;
    this->nodeIndicatorPin = nodeIndicatorPin;

#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif

    this->serialConfig = serialConfig;

    this->hs = serial;

    this->bpsRate = bpsRate;
}
RYUW122::RYUW122(byte txModulePin, byte rxModulePin, HardwareSerial* serial, byte nodeIndicatorPin, byte lowResetTriggerInputPin,   RYUW122BaudRate bpsRate, uint32_t serialConfig){
    this->txModulePin = txModulePin;
    this->rxModulePin = rxModulePin;

    this->nodeIndicatorPin = nodeIndicatorPin;

    this->lowResetTriggerInputPin = lowResetTriggerInputPin;

#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif

    this->serialConfig = serialConfig;

    this->hs = serial;

    this->bpsRate = bpsRate;
}
#endif

#ifdef ACTIVATE_SOFTWARE_SERIAL

RYUW122::RYUW122(SoftwareSerial* serial, RYUW122BaudRate bpsRate){
    this->ss = serial;
    this->hs = nullptr;
    this->bpsRate = bpsRate;
}
RYUW122::RYUW122(SoftwareSerial* serial, byte nodeIndicatorPin, RYUW122BaudRate bpsRate){
    this->nodeIndicatorPin = nodeIndicatorPin;
    this->ss = serial;
    this->hs = nullptr;
    this->bpsRate = bpsRate;
}
RYUW122::RYUW122(SoftwareSerial* serial, byte nodeIndicatorPin, byte lowResetTriggerInputPin,   RYUW122BaudRate bpsRate){
    this->nodeIndicatorPin = nodeIndicatorPin;
    this->lowResetTriggerInputPin = lowResetTriggerInputPin;
    this->ss = serial;
    this->hs = nullptr;
    this->bpsRate = bpsRate;
}
#endif

bool RYUW122::begin(){
    // Display pin configuration for debugging
    DEBUG_PRINT(F("RX Module Pin ---> "));
    DEBUG_PRINTLN(this->txModulePin);
    DEBUG_PRINT(F("TX Module Pin ---> "));
    DEBUG_PRINTLN(this->rxModulePin);
    DEBUG_PRINT(F("Node Indicator Pin (AUX) ---> "));
    DEBUG_PRINTLN(this->nodeIndicatorPin);
    DEBUG_PRINT(F("Low Reset Trigger Pin (M0) ---> "));
    DEBUG_PRINTLN(this->lowResetTriggerInputPin);
    DEBUG_PRINT(F("M1 Pin ---> "));
    DEBUG_PRINTLN(this->m1Pin);
    DEBUG_PRINT(F("M2 Pin ---> "));
    DEBUG_PRINTLN(this->m2Pin);

    // Initialize node indicator pin (AUX) if configured
    if (this->nodeIndicatorPin != -1) {
        pinMode(this->nodeIndicatorPin, INPUT);
        DEBUG_PRINTLN(F("Initialized node indicator (AUX) pin"));
    }

    // Initialize low reset trigger pin (M0) if configured
    if (this->lowResetTriggerInputPin != -1) {
        pinMode(this->lowResetTriggerInputPin, OUTPUT);
        DEBUG_PRINTLN(F("Initialized low reset trigger (M0) pin"));
        digitalWrite(this->lowResetTriggerInputPin, HIGH);
    }

    // Initialize serial communication based on configured type
    if (this->st){
        // Using generic Stream interface (already initialized) - assign directly
        // Stream does not provide a begin() method, so just use the provided stream
        this->serialDef.stream = this->st;
    } else {
        DEBUG_PRINTLN(F("Beginning serial initialization"));
    }

    if (this->hs){
        DEBUG_PRINTLN(F("Using Hardware Serial"));

#ifdef HARDWARE_SERIAL_SELECTABLE_PIN
        if(this->txModulePin != -1 && this->rxModulePin != -1) {
            DEBUG_PRINTLN(F("Hardware Serial with custom TX/RX pins"));
            this->serialDef.begin(*this->hs, (uint32_t)this->bpsRate, this->serialConfig, this->txModulePin, this->rxModulePin);
        }else{
            this->serialDef.begin(*this->hs, (uint32_t)this->bpsRate, this->serialConfig);
        }
#endif
#ifndef HARDWARE_SERIAL_SELECTABLE_PIN
        this->serialDef.begin(*this->hs, (uint32_t)this->bpsRate);
#endif
        // Wait for serial port to connect (needed for native USB)
        while (!this->hs) {
          ;
        }

#ifdef ACTIVATE_SOFTWARE_SERIAL
    }else if (this->ss){
        DEBUG_PRINTLN(F("Using Software Serial (pre-configured instance)"));

        this->serialDef.begin(*this->ss, (uint32_t)this->bpsRate);
    }    else{
        DEBUG_PRINTLN(F("Using Software Serial (creating new instance with pins)"));
        SoftwareSerial* mySerial = new SoftwareSerial((int)this->txModulePin, (int)this->rxModulePin);
        this->ss = mySerial;

        DEBUG_PRINT(F("Software Serial RX Pin: "));
        DEBUG_PRINTLN((int)this->txModulePin);
        DEBUG_PRINT(F("Software Serial TX Pin: "));
        DEBUG_PRINTLN((int)this->rxModulePin);

        this->serialDef.begin(*this->ss, (uint32_t)this->bpsRate);
#endif
    }

    // Set serial timeout for AT command responses
    this->serialDef.stream->setTimeout(100);

    // Return success if serial stream is initialized
    return this->serialDef.stream != nullptr;
}

/**
 * @brief Waits for the module to complete transmission.
 * Uses nodeIndicatorPin (AUX) if available, otherwise uses a delay.
 * Provides timeout protection to avoid infinite loops.
 *
 * @param timeout Maximum time to wait in milliseconds
 * @param waitNoAux Delay time in ms if no AUX pin is configured
 * @return True if completed successfully, false if timeout occurred
 */
bool RYUW122::waitCompleteResponse(unsigned long timeout, unsigned int waitNoAux) {
    unsigned long t = millis();

    // Protect against millis() overflow
    if (((unsigned long) (t + timeout)) == 0){
        t = 0;
    }

    // If AUX pin is configured, wait for it to go HIGH
    if (this->nodeIndicatorPin != -1) {
        while (digitalRead(this->nodeIndicatorPin) == LOW) {
            if ((millis() - t) > timeout){
                DEBUG_PRINTLN(F("Timeout error while waiting for AUX pin!"));
                return false;
            }
        }
        DEBUG_PRINTLN(F("AUX pin is HIGH - transmission complete"));
    }
    else {
        // If no AUX pin available, use fixed delay
        this->managedDelay(waitNoAux);
        DEBUG_PRINTLN(F("No AUX pin - using fixed delay"));
    }

    // Per datasheet, wait additional 20ms after AUX goes high
    this->managedDelay(20);
    DEBUG_PRINTLN(F("Module ready"));
    return true;
}

/**
 * @brief Non-blocking delay implementation.
 * Using delay() in a library can interfere with interrupts,
 * so we poll millis() until the timeout is reached.
 *
 * @param timeout Time to wait in milliseconds
 */
void RYUW122::managedDelay(unsigned long timeout) {
    unsigned long t = millis();

    // Protect against millis() overflow
    if (((unsigned long) (t + timeout)) == 0){
        t = 0;
    }

    // Poll until timeout is reached
    while ((millis() - t) < timeout)     {     }
}

/**
 * @brief Checks for available data on the serial stream.
 *
 * @return Number of bytes available to read, or 0 if stream is not initialized
 */
int RYUW122::available() {
    return this->serialDef.stream ? this->serialDef.stream->available() : 0;
}

/**
 * @brief Reads a single byte from the serial stream.
 *
 * @return The byte read, or -1 if no data available or stream not initialized
 */
int RYUW122::read() {
    return this->serialDef.stream ? this->serialDef.stream->read() : -1;
}

void RYUW122::loop() {
    if (this->serialDef.stream && this->serialDef.stream->available()) {
        String response = this->serialDef.stream->readStringUntil('\n');
        response.trim();
        if (response.length() > 0) {
            DEBUG_PRINT(F("AT< "));
            DEBUG_PRINTLN(response);
            if (response.startsWith(F("+ANCHOR_RCV="))) {
                char buf[64];
                strncpy(buf, response.c_str(), sizeof(buf)-1);
                buf[sizeof(buf)-1] = '\0';
                parseAnchorReceive(buf);
            } else if (response.startsWith(F("+TAG_RCV="))) {
                char buf[64];
                strncpy(buf, response.c_str(), sizeof(buf)-1);
                buf[sizeof(buf)-1] = '\0';
                parseTagReceive(buf);
            }
        }
    }
}

bool RYUW122::setMode(RYUW122Mode mode) {
    char command[20];
    snprintf(command, sizeof(command), "AT+MODE=%d", (int)mode);
    return sendCommand(command, F("+OK"));
}

RYUW122Mode RYUW122::getMode() {
    char response[64];
    if (sendCommandAndGetResponse("AT+MODE?", response, sizeof(response))) {
        if (strstr(response, "+MODE=") != nullptr) {
            return (RYUW122Mode)atoi(response + 6);
        }
    }
    return RYUW122Mode::TAG; // Default value
}

bool RYUW122::setBaudRate(RYUW122BaudRate baudRate) {
    char command[30];
    snprintf(command, sizeof(command), "AT+IPR=%u", (unsigned)baudRate);
    return sendCommand(command, F("+OK"));
}

RYUW122BaudRate RYUW122::getBaudRate() {
    char response[64];
    if (sendCommandAndGetResponse("AT+IPR?", response, sizeof(response))) {
        if (strstr(response, "+IPR=") != nullptr) {
            return (RYUW122BaudRate)atoi(response + 5);
        }
    }
    return RYUW122BaudRate::B_115200; // Default value
}

bool RYUW122::setRfChannel(RYUW122RFChannel channel) {
    char command[20];
    snprintf(command, sizeof(command), "AT+CHANNEL=%d", (int)channel);
    return sendCommand(command, F("+OK"));
}

RYUW122RFChannel RYUW122::getRfChannel() {
    char response[64];
    if (sendCommandAndGetResponse("AT+CHANNEL?", response, sizeof(response))) {
        if (strstr(response, "+CHANNEL=") != nullptr) {
            return (RYUW122RFChannel)atoi(response + 9);
        }
    }
    return RYUW122RFChannel::CH_5; // Default value
}

bool RYUW122::setBandwidth(RYUW122Bandwidth bandwidth) {
    char command[25];
    snprintf(command, sizeof(command), "AT+BANDWIDTH=%d", (int)bandwidth);
    return sendCommand(command, F("+OK"));
}

RYUW122Bandwidth RYUW122::getBandwidth() {
    char response[64];
    if (sendCommandAndGetResponse("AT+BANDWIDTH?", response, sizeof(response))) {
        if (strstr(response, "+BANDWIDTH=") != nullptr) {
            return (RYUW122Bandwidth)atoi(response + 11);
        }
    }
    return RYUW122Bandwidth::BW_850K; // Default value
}

bool RYUW122::setNetworkId(const char* networkId) {
    char command[64];
    snprintf(command, sizeof(command), "AT+NETWORKID=%s", networkId);
    return sendCommand(command, F("+OK"));
}

bool RYUW122::getNetworkId(char* networkId) {
    if (sendCommandAndGetResponse("AT+NETWORKID?", _buffer, sizeof(_buffer))) {
        if (strstr(_buffer, "+NETWORKID=") != nullptr) {
            strncpy(networkId, _buffer + 11, 8);
            networkId[8] = '\0';
            return true;
        }
    }
    return false;
}

bool RYUW122::setAddress(const char* address) {
    char command[64];
    snprintf(command, sizeof(command), "AT+ADDRESS=%s", address);
    return sendCommand(command, F("+OK"));
}

bool RYUW122::getAddress(char* address) {
    if (sendCommandAndGetResponse("AT+ADDRESS?", _buffer, sizeof(_buffer))) {
        if (strstr(_buffer, "+ADDRESS=") != nullptr) {
            strncpy(address, _buffer + 9, 8);
            address[8] = '\0';
            return true;
        }
    }
    return false;
}

bool RYUW122::getUid(char* uid) {
    if (sendCommandAndGetResponse("AT+UID?", _buffer, sizeof(_buffer))) {
        if (strstr(_buffer, "+UID=") != nullptr) {
            strncpy(uid, _buffer + 5, 16);
            uid[16] = '\0';
            return true;
        }
    }
    return false;
}

bool RYUW122::setPassword(const char* password) {
    char command[64];
    snprintf(command, sizeof(command), "AT+CPIN=%s", password);
    return sendCommand(command, F("+OK"));
}

bool RYUW122::getPassword(char* password) {
    if (sendCommandAndGetResponse("AT+CPIN?", _buffer, sizeof(_buffer))) {
        if (strstr(_buffer, "+CPIN=") != nullptr) {
            strncpy(password, _buffer + 6, 32);
            password[32] = '\0';
            return true;
        }
    }
    return false;
}

bool RYUW122::setTagRfDutyCycle(int rfEnableTime, int rfDisableTime) {
    char command[32];
    snprintf(command, sizeof(command), "AT+TAGD=%d,%d", rfEnableTime, rfDisableTime);
    return sendCommand(command, F("+OK"));
}

bool RYUW122::getTagRfDutyCycle(int& rfEnableTime, int& rfDisableTime) {
    char response[64];
    if (sendCommandAndGetResponse("AT+TAGD?", response, sizeof(response))) {
        if (strstr(response, "+TAGD=") != nullptr) {
            sscanf(response + 6, "%d,%d", &rfEnableTime, &rfDisableTime);
            return true;
        }
    }
    return false;
}

bool RYUW122::setRfPower(RYUW122RFPower power) {
    char command[20];
    snprintf(command, sizeof(command), "AT+CRFOP=%d", (int)power);
    return sendCommand(command, F("+OK"));
}

RYUW122RFPower RYUW122::getRfPower() {
    char response[64];
    if (sendCommandAndGetResponse("AT+CRFOP?", response, sizeof(response))) {
        if (strstr(response, "+CRFOP=") != nullptr) {
            return (RYUW122RFPower)atoi(response + 7);
        }
    }
    return RYUW122RFPower::N32dBm; // Default value
}

bool RYUW122::anchorSendData(const char* tagAddress, int payloadLength, const char* data) {
    char command[64];
    snprintf(command, sizeof(command), "AT+ANCHOR_SEND=%s,%d,%s", tagAddress, payloadLength, data);
    return sendCommand(command, F("+OK"));
}

bool RYUW122::anchorSendDataSync(const char* tagAddress, int payloadLength, const char* data, char* responseData, int* distance, int* rssi, unsigned long timeout) {
    // Validate parameters according to AT command documentation
    if (!tagAddress || strlen(tagAddress) != 8) {
        DEBUG_PRINTLN(F("Error: TAG Address must be 8 bytes ASCII"));
        return false;
    }
    if (payloadLength < 0 || payloadLength > RYUW122_MAX_PAYLOAD_LENGTH) {
        DEBUG_PRINTLN(F("Error: Payload length must be 0-12 bytes"));
        return false;
    }
    if (!data && payloadLength > 0) {
        DEBUG_PRINTLN(F("Error: Data cannot be nullptr with positive payload length"));
        return false;
    }

    // Send the command
    char command[64];
    snprintf(command, sizeof(command), "AT+ANCHOR_SEND=%s,%d,%s", tagAddress, payloadLength, data ? data : "");

    if (!this->serialDef.stream) return false;

    // Debug: show the command being sent
    DEBUG_PRINT(F("AT> "));
    DEBUG_PRINTLN(command);

    this->serialDef.stream->println(command);

    unsigned long startTime = millis();
    bool receivedOk = false;
    bool receivedData = false;

    // Wait for +OK and +ANCHOR_RCV response
    while ((millis() - startTime) < timeout) {
        if (this->serialDef.stream->available()) {
            String response = this->serialDef.stream->readStringUntil('\n');
            response.trim();
            DEBUG_PRINT(F("AT< "));
            DEBUG_PRINTLN(response);

            if (response.startsWith(F("+OK"))) {
                receivedOk = true;
            } else if (response.startsWith(F("+ANCHOR_RCV="))) {
                // Parse the response: +ANCHOR_RCV=<TAG Address>,<PAYLOAD LENGTH>,<TAG DATA>,<DISTANCE>,<RSSI>
                char buf[64];
                strncpy(buf, response.c_str(), sizeof(buf)-1);
                buf[sizeof(buf)-1] = '\0';
                /* use compile-time length to avoid runtime strlen on literal */
                char* ptr = buf + (sizeof("+ANCHOR_RCV=") - 1);
                char* recvTagAddr = strtok(ptr, ",");
                strtok(nullptr, ","); // Skip payload length
                char* recvData = strtok(nullptr, ",");
                char* recvDistanceStr = strtok(nullptr, ",");
                char* recvRssiStr = strtok(nullptr, ",\r\n");

                // Verify it's from the correct TAG
                if (recvTagAddr && strcmp(recvTagAddr, tagAddress) == 0) {
                    if (responseData && recvData) {
                        strncpy(responseData, recvData, RYUW122_MAX_PAYLOAD_LENGTH);
                        responseData[RYUW122_MAX_PAYLOAD_LENGTH] = '\0';
                    }
                    if (distance && recvDistanceStr) {
                        *distance = atoi(recvDistanceStr);
                    }
                    if (rssi && recvRssiStr) {
                        *rssi = atoi(recvRssiStr);
                    }
                    receivedData = true;
                    break;
                }
            }
        }
    }

    return receivedOk && receivedData;
}

bool RYUW122::tagSendData(int payloadLength, const char* data) {
    char command[64];
    snprintf(command, sizeof(command), "AT+TAG_SEND=%d,%s", payloadLength, data);
    return sendCommand(command, F("+OK"));
}

bool RYUW122::tagSendDataSync(int payloadLength, const char* data, unsigned long timeout) {
    // Validate parameters according to AT command documentation
    if (payloadLength < 0 || payloadLength > RYUW122_MAX_PAYLOAD_LENGTH) {
        DEBUG_PRINTLN(F("Error: Payload length must be 0-12 bytes"));
        return false;
    }
    if (!data && payloadLength > 0) {
        DEBUG_PRINTLN(F("Error: Data cannot be nullptr with positive payload length"));
        return false;
    }

    // Send the command
    char command[64];
    snprintf(command, sizeof(command), "AT+TAG_SEND=%d,%s", payloadLength, data ? data : "");

    if (!this->serialDef.stream) return false;

    // Debug: show the command being sent
    DEBUG_PRINT(F("AT> "));
    DEBUG_PRINTLN(command);

    this->serialDef.stream->println(command);

    unsigned long startTime = millis();

    // Wait for +OK response
    while ((millis() - startTime) < timeout) {
        if (this->serialDef.stream->available()) {
            String response = this->serialDef.stream->readStringUntil('\n');
            response.trim();
            DEBUG_PRINT(F("AT< "));
            DEBUG_PRINTLN(response);

            if (response.startsWith(F("+OK"))) {
                return true;
            }
        }
    }

    return false;
}

bool RYUW122::setRssiDisplay(RYUW122RSSI rssi) {
    char command[20];
    snprintf(command, sizeof(command), "AT+RSSI=%d", (int)rssi);
    return sendCommand(command, F("+OK"));
}

RYUW122RSSI RYUW122::getRssiDisplay() {
    char response[64];
    if (sendCommandAndGetResponse("AT+RSSI?", response, sizeof(response))) {
        if (strstr(response, "+RSSI=") != nullptr) {
            return (RYUW122RSSI)atoi(response + 6);
        }
    }
    return RYUW122RSSI::DISABLE; // Default value
}

bool RYUW122::setDistanceCalibration(int calibrationValue) {
    char command[20];
    snprintf(command, sizeof(command), "AT+CAL=%d", calibrationValue);
    return sendCommand(command, F("+OK"));
}

int RYUW122::getDistanceCalibration() {
    char response[64];
    if (sendCommandAndGetResponse("AT+CAL?", response, sizeof(response))) {
        if (strstr(response, "+CAL=") != nullptr) {
            return atoi(response + 5);
        }
    }
    return 0; // Default value
}

bool RYUW122::getFirmwareVersion(char* version) {
    if (sendCommandAndGetResponse("AT+VER?", _buffer, sizeof(_buffer))) {
        if (strstr(_buffer, "+VER=") != nullptr) {
            strncpy(version, _buffer + 5, 16);
            version[16] = '\0';
            return true;
        }
    }
    return false;
}

bool RYUW122::factoryReset() {
    return sendCommand("AT+FACTORY", F("+FACTORY"));
}

bool RYUW122::reset() {
    return sendCommand("AT+RESET", F("+RESET"));
}

bool RYUW122::test() {
    return sendCommand("AT", F("+OK"));
}

void RYUW122::onAnchorReceive(AnchorReceiveCallback callback) {
    _anchorReceiveCallback = callback;
}

void RYUW122::onTagReceive(TagReceiveCallback callback) {
    _tagReceiveCallback = callback;
}

bool RYUW122::sendCommand(const char* command, const char* expectedResponse, int timeout) {
    if (!this->serialDef.stream) return false;

    // Debug: show the command being sent
    DEBUG_PRINT(F("AT> "));
    DEBUG_PRINTLN(command);

    this->serialDef.stream->println(command);
    unsigned long start = millis();
    String response;
    while ((millis() - start) < (unsigned long)timeout) {
        if (this->serialDef.stream->available()) {
            response = this->serialDef.stream->readStringUntil('\n');
            response.trim();
            break;
        }
    }
    DEBUG_PRINT(F("AT< "));
    DEBUG_PRINTLN(response);
    return response.startsWith(expectedResponse);
}

// Overload: accept flash string (F("...")) as expected response
bool RYUW122::sendCommand(const char* command, const __FlashStringHelper* expectedResponse, int timeout) {
    if (!this->serialDef.stream) return false;

    // Debug: show the command being sent
    DEBUG_PRINT(F("AT> "));
    DEBUG_PRINTLN(command);

    this->serialDef.stream->println(command);
    unsigned long start = millis();
    String response;
    while ((millis() - start) < (unsigned long)timeout) {
        if (this->serialDef.stream->available()) {
            response = this->serialDef.stream->readStringUntil('\n');
            response.trim();
            break;
        }
    }
    DEBUG_PRINT(F("AT< "));
    DEBUG_PRINTLN(response);
    return response.startsWith(expectedResponse);
}

bool RYUW122::sendCommandAndGetResponse(const char* command, char* response, int responseSize, int timeout) {
    if (!this->serialDef.stream) return false;

    // Debug: show the command being sent
    DEBUG_PRINT(F("AT> "));
    DEBUG_PRINTLN(command);

    this->serialDef.stream->println(command);
    unsigned long start = millis();
    String res;
    while ((millis() - start) < (unsigned long)timeout) {
        if (this->serialDef.stream->available()) {
            res = this->serialDef.stream->readStringUntil('\n');
            res.trim();
            break;
        }
    }
    DEBUG_PRINT(F("AT< "));
    DEBUG_PRINTLN(res);
    if (res.length() > 0) {
        strncpy(response, res.c_str(), responseSize - 1);
        response[responseSize - 1] = '\0';
        return true;
    }
    return false;
}

void RYUW122::parseAnchorReceive(char* response) {
    // Parse the response once
    /* use compile-time length to avoid runtime strlen on literal */
    char* ptr = response + (sizeof("+ANCHOR_RCV=") - 1);
    char* tagAddress = strtok(ptr, ",");
    char* payloadStr = strtok(nullptr, ",");
    char* tagData = strtok(nullptr, ",");
    char* distanceStr = strtok(nullptr, ",");
    char* rssiStr = strtok(nullptr, ",\r\n");
    int payloadLength = payloadStr ? atoi(payloadStr) : 0;
    int distance = distanceStr ? atoi(distanceStr) : 0;
    int rssi = rssiStr ? atoi(rssiStr) : 0;

    // Trigger original callback if registered
    if (_anchorReceiveCallback) {
        _anchorReceiveCallback(tagAddress ? tagAddress : "", payloadLength, tagData ? tagData : "", distance, rssi);
    }

    // Trigger simplified callbacks if registered
    if (_simpleMessageCallback && tagData && strlen(tagData) > 0) {
        _simpleMessageCallback(tagAddress ? tagAddress : "", tagData, rssi);
    }

    if (_simpleDistanceCallback) {
        _simpleDistanceCallback(tagAddress ? tagAddress : "",
                               convertDistance(distance, _preferredUnit),
                               _preferredUnit, rssi);
    }
}

void RYUW122::parseTagReceive(char* response) {
    if (_tagReceiveCallback) {
        // +TAG_RCV=<payloadLength>,<data>,<rssi>
        /* use compile-time length to avoid runtime strlen on literal */
        char* ptr = response + (sizeof("+TAG_RCV=") - 1);
        char* payloadStr = strtok(ptr, ",");
        char* data = strtok(nullptr, ",");
        char* rssiStr = strtok(nullptr, ",\r\n");
        int payloadLength = payloadStr ? atoi(payloadStr) : 0;
        int rssi = rssiStr ? atoi(rssiStr) : 0;
        _tagReceiveCallback(payloadLength, data ? data : "", rssi);
    }

    // Also trigger simplified callback if registered
    if (_simpleMessageCallback) {
        /* use compile-time length to avoid runtime strlen on literal */
        char* ptr = response + (sizeof("+TAG_RCV=") - 1);
        strtok(ptr, ","); // Skip payload length
        char* data = strtok(nullptr, ",");
        char* rssiStr = strtok(nullptr, ",\r\n");
        int rssi = rssiStr ? atoi(rssiStr) : 0;
        _simpleMessageCallback("ANCHOR", data ? data : "", rssi);
    }
}

// ========================================
// SIMPLIFIED HIGH-LEVEL API IMPLEMENTATION
// ========================================

float RYUW122::convertDistance(int distanceCm, MeasureUnit unit) {
    switch (unit) {
        case MeasureUnit::CENTIMETERS:
            return (float)distanceCm;
        case MeasureUnit::INCHES:
            return distanceCm / 2.54f;
        case MeasureUnit::METERS:
            return distanceCm / 100.0f;
        case MeasureUnit::FEET:
            return distanceCm / 30.48f;
        default:
            return (float)distanceCm;
    }
}

float RYUW122::getDistanceFrom(const char* tagAddress, MeasureUnit unit, unsigned long timeout) {
    // Validate TAG address
    if (!tagAddress || strlen(tagAddress) != 8) {
        DEBUG_PRINTLN(F("Error: TAG Address must be 8 bytes ASCII"));
        return -1.0f;
    }

    // Send empty payload to get distance
    char responseData[RYUW122_MAX_PAYLOAD_LENGTH + 1];
    int distance = 0;
    int rssi = 0;

    bool success = anchorSendDataSync(tagAddress, 0, "", responseData, &distance, &rssi, timeout);

    if (success) {
        // Trigger distance callback if registered
        if (_simpleDistanceCallback) {
            _simpleDistanceCallback(tagAddress, convertDistance(distance, _preferredUnit), _preferredUnit, rssi);
        }
        return convertDistance(distance, unit);
    }

    return -1.0f;
}

bool RYUW122::sendMessageToTag(const char* tagAddress, const char* message, unsigned long timeout) {
    // Validate inputs
    if (!tagAddress || strlen(tagAddress) != 8) {
        DEBUG_PRINTLN(F("Error: TAG Address must be 8 bytes ASCII"));
        return false;
    }
    if (!message) {
        DEBUG_PRINTLN(F("Error: Message cannot be nullptr"));
        return false;
    }

    int messageLen = strlen(message);
    if (messageLen > RYUW122_MAX_PAYLOAD_LENGTH) {
        DEBUG_PRINTLN(F("Error: Message too long (max 12 characters)"));
        return false;
    }

    // Send message and get response
    char responseData[RYUW122_MAX_PAYLOAD_LENGTH + 1];
    int distance = 0;
    int rssi = 0;

    bool success = anchorSendDataSync(tagAddress, messageLen, message, responseData, &distance, &rssi, timeout);

    if (success && _simpleDistanceCallback) {
        _simpleDistanceCallback(tagAddress, convertDistance(distance, _preferredUnit), _preferredUnit, rssi);
    }

    return success;
}

bool RYUW122::sendMessageFromTag(const char* message, unsigned long timeout) {
    // Validate input
    if (!message) {
        DEBUG_PRINTLN(F("Error: Message cannot be nullptr"));
        return false;
    }

    int messageLen = strlen(message);
    if (messageLen > RYUW122_MAX_PAYLOAD_LENGTH) {
        DEBUG_PRINTLN(F("Error: Message too long (max 12 characters)"));
        return false;
    }

    return tagSendDataSync(messageLen, message, timeout);
}

void RYUW122::processMessages() {
    // This is an alias for loop() with clearer naming
    loop();
}

void RYUW122::onMessageReceived(SimpleMessageCallback callback) {
    _simpleMessageCallback = callback;
}

void RYUW122::onDistanceMeasured(SimpleDistanceCallback callback, MeasureUnit unit) {
    _simpleDistanceCallback = callback;
    _preferredUnit = unit;
}

int RYUW122::getMultipleDistances(const char** tagAddresses, int numTags, float* distances,
                                   MeasureUnit unit, unsigned long timeout) {
    if (!tagAddresses || !distances || numTags <= 0) {
        DEBUG_PRINTLN(F("Error: Invalid parameters for getMultipleDistances"));
        return 0;
    }

    int successCount = 0;

    for (int i = 0; i < numTags; i++) {
        DEBUG_PRINT(F("Measuring distance to TAG: "));
        DEBUG_PRINTLN(tagAddresses[i]);

        float distance = getDistanceFrom(tagAddresses[i], unit, timeout);

        if (distance >= 0.0f) {
            distances[i] = distance;
            successCount++;
            DEBUG_PRINT(F("Distance: "));
            DEBUG_PRINT(distance);
            DEBUG_PRINTLN(unit == MeasureUnit::CENTIMETERS ? F(" cm") :
                         unit == MeasureUnit::INCHES ? F(" in") :
                         unit == MeasureUnit::METERS ? F(" m") : F(" ft"));
        } else {
            distances[i] = -1.0f;
            DEBUG_PRINT(F("Failed to measure distance to "));
            DEBUG_PRINTLN(tagAddresses[i]);
        }

        // Small delay between measurements
        delay(100);
    }

    return successCount;
}

bool RYUW122::begin(RYUW122BaudRate baudRate) {
    this->bpsRate = baudRate;
    return begin();
}


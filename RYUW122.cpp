/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * RYUW122 Arduino library implementation
 */

#include "RYUW122.h"
#include <stdlib.h>

// Helper: safe integer parse using strtol. Returns defVal on null/invalid input.
static int safeAtoi(const char* s, int defVal = 0) {
    if (!s) return defVal;
    char* endptr = nullptr;
    long val = strtol(s, &endptr, 10);
    if (endptr == s) return defVal; // no conversion
    return (int)val;
}

// Helper: read a line from stream into a char buffer, handling timeout and buffer size.
// Returns true if a line was read, false on timeout or error.
static bool readLine(Stream& stream, char* buffer, size_t bufferSize, unsigned long timeout) {
    unsigned long start = millis();
    size_t pos = 0;
    while ((millis() - start) < timeout) {
        if (stream.available()) {
            char c = stream.read();
            if (c == '\n') {
                buffer[pos] = '\0';
                return true;
            }
            if (pos < bufferSize - 1) {
                buffer[pos++] = c;
            }
        }
    }
    buffer[pos] = '\0'; // Null-terminate even on timeout
    return false; // Timeout
}


RYUW122::RYUW122(Stream* serial) : st(serial) {
    // Quando viene passato un Stream generico, assumiamo hardware-like stream
    this->hs = nullptr;
#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif
    this->isSoftwareSerial = false;
}

#ifdef ACTIVATE_SOFTWARE_SERIAL
RYUW122::RYUW122(byte mcuTxPin, byte mcuRxPin, RYUW122BaudRate bpsRate){
    this->mcuTxPin = mcuTxPin;
    this->mcuRxPin = mcuRxPin;
    this->ss = nullptr;
    this->hs = nullptr;
    this->st = nullptr;
    this->isSoftwareSerial = true;
    this->bpsRate = bpsRate;
}
// New overload: tx, rx, lowResetTriggerInputPin
RYUW122::RYUW122(byte mcuTxPin, byte mcuRxPin, byte lowResetTriggerInputPin, RYUW122BaudRate bpsRate){
    this->mcuTxPin = mcuTxPin;
    this->mcuRxPin = mcuRxPin;

    this->lowResetTriggerInputPin = lowResetTriggerInputPin;

    this->ss = nullptr;
    this->hs = nullptr;
    this->st = nullptr;
    this->isSoftwareSerial = true;

    this->bpsRate = bpsRate;
}
// New overload: tx, rx, lowResetTriggerInputPin, nodeIndicatorPin
RYUW122::RYUW122(byte mcuTxPin, byte mcuRxPin, byte lowResetTriggerInputPin, byte nodeIndicatorPin, RYUW122BaudRate bpsRate){
    this->mcuTxPin = mcuTxPin;
    this->mcuRxPin = mcuRxPin;

    this->lowResetTriggerInputPin = lowResetTriggerInputPin;
    this->nodeIndicatorPin = nodeIndicatorPin;

    this->ss = nullptr;
    this->hs = nullptr;
    this->st = nullptr;
    this->isSoftwareSerial = true;

    this->bpsRate = bpsRate;
}
#endif

RYUW122::RYUW122(HardwareSerial* serial, RYUW122BaudRate bpsRate){ //, uint32_t serialConfig
    // leave pins as default (-1) unless provided by other ctors
#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif
    this->hs = serial;
    this->st = nullptr;
    this->isSoftwareSerial = false;
    this->bpsRate = bpsRate;
}
// allow passing only reset pin as second argument
RYUW122::RYUW122(HardwareSerial* serial, byte lowResetTriggerInputPin, RYUW122BaudRate bpsRate){ // , uint32_t serialConfig
    this->lowResetTriggerInputPin = lowResetTriggerInputPin;
#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif
    this->hs = serial;
    this->st = nullptr;
    this->isSoftwareSerial = false;
    this->bpsRate = bpsRate;
}
// overload with reset pin then node indicator
RYUW122::RYUW122(HardwareSerial* serial, byte lowResetTriggerInputPin, byte nodeIndicatorPin, RYUW122BaudRate bpsRate){ //, uint32_t serialConfig
    this->lowResetTriggerInputPin = lowResetTriggerInputPin;
    this->nodeIndicatorPin = nodeIndicatorPin;

#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif
    this->hs = serial;
    this->st = nullptr;
    this->isSoftwareSerial = false;
    this->bpsRate = bpsRate;
}

#ifdef HARDWARE_SERIAL_SELECTABLE_PIN
RYUW122::RYUW122(byte mcuTxPin, byte mcuRxPin, HardwareSerial* serial, RYUW122BaudRate bpsRate){
    this->mcuTxPin = mcuTxPin;
    this->mcuRxPin = mcuRxPin;

#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif

    // use default member serialConfig from header

    this->hs = serial;

    this->st = nullptr;
    this->bpsRate = bpsRate;
}
RYUW122::RYUW122(byte mcuTxPin, byte mcuRxPin, HardwareSerial* serial, byte lowResetTriggerInputPin, RYUW122BaudRate bpsRate){
    this->mcuTxPin = mcuTxPin;
    this->mcuRxPin = mcuRxPin;
    this->lowResetTriggerInputPin = lowResetTriggerInputPin;

#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif

    // use default member serialConfig from header

    this->hs = serial;

    this->st = nullptr;
    this->bpsRate = bpsRate;
}
RYUW122::RYUW122(byte mcuTxPin, byte mcuRxPin, HardwareSerial* serial, byte lowResetTriggerInputPin, byte nodeIndicatorPin, RYUW122BaudRate bpsRate){
    this->mcuTxPin = mcuTxPin;
    this->mcuRxPin = mcuRxPin;

    this->lowResetTriggerInputPin = lowResetTriggerInputPin;
    this->nodeIndicatorPin = nodeIndicatorPin;

#ifdef ACTIVATE_SOFTWARE_SERIAL
    this->ss = nullptr;
#endif

    // use default member serialConfig from header

    this->hs = serial;

    this->st = nullptr;
    this->bpsRate = bpsRate;
}
#endif

#ifdef ACTIVATE_SOFTWARE_SERIAL

RYUW122::RYUW122(SoftwareSerial* serial, RYUW122BaudRate bpsRate){
    this->ss = serial;
    this->hs = nullptr;
    this->st = nullptr;
    this->isSoftwareSerial = true;
    this->bpsRate = bpsRate;
}
RYUW122::RYUW122(SoftwareSerial* serial, byte lowResetTriggerInputPin, RYUW122BaudRate bpsRate){
    this->lowResetTriggerInputPin = lowResetTriggerInputPin;
    this->ss = serial;
    this->hs = nullptr;
    this->st = nullptr;
    this->isSoftwareSerial = true;
    this->bpsRate = bpsRate;
}
RYUW122::RYUW122(SoftwareSerial* serial, byte lowResetTriggerInputPin, byte nodeIndicatorPin, RYUW122BaudRate bpsRate){
    this->lowResetTriggerInputPin = lowResetTriggerInputPin;
    this->nodeIndicatorPin = nodeIndicatorPin;
    this->ss = serial;
    this->hs = nullptr;
    this->st = nullptr;
    this->isSoftwareSerial = true;
    this->bpsRate = bpsRate;
}
#endif

bool RYUW122::begin(){
    // Display pin configuration for debugging
    // mcuTxPin: microcontroller TX pin (connects to module RX)
    // mcuRxPin: microcontroller RX pin (connects to module TX)
    DEBUG_PRINT(F("MC TX Pin (to module RX) ---> "));
    DEBUG_PRINTLN(this->mcuTxPin);
    DEBUG_PRINT(F("MC RX Pin (from module TX) ---> "));
    DEBUG_PRINTLN(this->mcuRxPin);
    DEBUG_PRINT(F("Node Indicator Pin  ---> "));
    DEBUG_PRINTLN(this->nodeIndicatorPin);
    DEBUG_PRINT(F("Low Reset Trigger Pin  ---> "));
    DEBUG_PRINTLN(this->lowResetTriggerInputPin);

    // Initialize node indicator pin  if configured
    if (this->nodeIndicatorPin != -1) {
        pinMode(this->nodeIndicatorPin, INPUT);
        DEBUG_PRINTLN(F("Initialized node indicator  pin"));
    }

    // Initialize low reset trigger pin  if configured
    if (this->lowResetTriggerInputPin != -1) {
        pinMode(this->lowResetTriggerInputPin, OUTPUT);
        // DEBUG_PRINTLN(F("Initialized low reset trigger  pin"));
        // digitalWrite(this->lowResetTriggerInputPin, HIGH);
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
        if(this->mcuTxPin != -1 && this->mcuRxPin != -1) {
            DEBUG_PRINTLN(F("Hardware Serial with custom TX/RX pins"));
            this->serialDef.begin(*this->hs, (uint32_t)this->bpsRate, this->serialConfig, this->mcuRxPin, this->mcuTxPin);
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
        // SoftwareSerial constructor expects (rxPin, txPin)
        SoftwareSerial* mySerial = new SoftwareSerial((int)this->mcuRxPin, (int)this->mcuTxPin);
        this->ss = mySerial;

        DEBUG_PRINT(F("Software Serial RX Pin: "));
        DEBUG_PRINTLN((int)this->mcuRxPin);
        DEBUG_PRINT(F("Software Serial TX Pin: "));
        DEBUG_PRINTLN((int)this->mcuTxPin);

        this->serialDef.begin(*this->ss, (uint32_t)this->bpsRate);
#endif
    }

    // If a hardware reset pin is provided, perform a hardware reset of the module
    // to ensure it starts in a known state. This mirrors the behaviour used in
    // example sketches where the NRST pin is toggled LOW for a few ms.
    if (this->lowResetTriggerInputPin != -1) {
        DEBUG_PRINTLN(F("Performing hardware reset via reset pin"));
        // small delay to ensure pin mode settled
        // delay(5);
        hardwareResetPin();
        // allow module to boot
        // delay(100);
        // drain any initial messages if present
        DEBUG_PRINTLN(F("Draining initial messages"));
        // Use a short timeout so we don't block forever if the module keeps sending data
        unsigned long _drainStart = millis();
        const unsigned long _drainIdleTimeout = 200; // ms without data to consider draining complete
        while (this->serialDef.stream) {
            // Read all available bytes and reset the idle timer
            while (this->serialDef.stream->available()) {
                (void)this->serialDef.stream->read();
                _drainStart = millis();
            }
            // If no data arrived for _drainIdleTimeout ms, we're done
            if ((millis() - _drainStart) > _drainIdleTimeout) break;
        }
        DEBUG_PRINTLN(F("Complete!"));
    }

    // Set serial timeout for AT command responses
    this->serialDef.stream->setTimeout((unsigned long)this->_streamTimeoutMs);

    // Warn if using SoftwareSerial at 115200 baud, common issue on UNO
    #ifdef ACTIVATE_SOFTWARE_SERIAL
    if (this->bpsRate == RYUW122BaudRate::B_115200) {
        DEBUG_PRINTLN(F("Warning: Using SoftwareSerial at 115200 baud, may not be reliable on UNO"));
    }
    #endif

    // Return success if serial stream is initialized
    return this->serialDef.stream != nullptr;
}

/**
 * @brief Non-blocking delay implementation.
 * Using delay() in a library can interfere with interrupts,
 * so we poll millis() until the timeout is reached.
 *
 * @param timeout Time to wait in milliseconds
 */
void RYUW122::managedDelay(unsigned long timeout) const {
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
        char response[64];
        if (readLine(*this->serialDef.stream, response, sizeof(response), 1000)) {
            // Trim whitespace
            char* p = response;
            while (isspace(*p)) p++;
            char* end = p + strlen(p) - 1;
            while (end > p && isspace(*end)) *end-- = '\0';

            if (strlen(p) > 0) {
                DEBUG_PRINT(F("AT< "));
                DEBUG_PRINTLN(p);
                if (strncmp_P(p, PSTR("+ANCHOR_RCV="), 12) == 0) {
                    parseAnchorReceive(p);
                } else if (strncmp_P(p, PSTR("+TAG_RCV="), 9) == 0) {
                    parseTagReceive(p);
                }
            }
        }
    }
}

bool RYUW122::setMode(RYUW122Mode mode) {
    char command[20];
    snprintf_P(command, sizeof(command), PSTR("AT+MODE=%d"), (int)mode);
    bool result = sendCommand(command, F("+OK"));
    if (result) managedDelay(100);
    return result;
}

RYUW122Mode RYUW122::getMode() {
    char response[64];
    if (sendCommandAndGetResponse(F("AT+MODE?"), response, sizeof(response))) {
        if (strstr_P(response, PSTR("+MODE=")) != nullptr) {
            int modeVal = safeAtoi(response + 6, -1);
            switch(modeVal) {
                case 0: return RYUW122Mode::TAG;
                case 1: return RYUW122Mode::ANCHOR;
                case 2: return RYUW122Mode::SLEEP;
                default: return RYUW122Mode::UNKNOWN;
            }
        }
    }
    return RYUW122Mode::UNKNOWN;
}

bool RYUW122::setBaudRate(RYUW122BaudRate baudRate) {
    char command[30];
    snprintf_P(command, sizeof(command), PSTR("AT+IPR=%u"), (unsigned)baudRate);
    bool result = sendCommand(command, F("+OK"));
    if (result) managedDelay(100);
    return result;
}

RYUW122BaudRate RYUW122::getBaudRate() {
    char response[64];
    if (sendCommandAndGetResponse(F("AT+IPR?"), response, sizeof(response))) {
        if (strstr_P(response, PSTR("+IPR=")) != nullptr) {
            long baud = strtol(response + 5, nullptr, 10);
            switch(baud) {
                case 9600: return RYUW122BaudRate::B_9600;
                case 57600: return RYUW122BaudRate::B_57600;
                case 115200: return RYUW122BaudRate::B_115200;
                default: return RYUW122BaudRate::UNKNOWN;
            }
        }
    }
    return RYUW122BaudRate::UNKNOWN;
}

bool RYUW122::setRfChannel(RYUW122RFChannel channel) {
    char command[20];
    snprintf_P(command, sizeof(command), PSTR("AT+CHANNEL=%d"), (int)channel);
    bool result = sendCommand(command, F("+OK"));
    if (result) managedDelay(100);
    return result;
}

RYUW122RFChannel RYUW122::getRfChannel() {
    char response[64];
    if (sendCommandAndGetResponse(F("AT+CHANNEL?"), response, sizeof(response))) {
        if (strstr_P(response, PSTR("+CHANNEL=")) != nullptr) {
            int channelVal = safeAtoi(response + 9, -1);
            switch(channelVal) {
                case 5: return RYUW122RFChannel::CH_5;
                case 9: return RYUW122RFChannel::CH_9;
                default: return RYUW122RFChannel::UNKNOWN;
            }
        }
    }
    return RYUW122RFChannel::UNKNOWN;
}

bool RYUW122::setBandwidth(RYUW122Bandwidth bandwidth) {
    char command[25];
    snprintf_P(command, sizeof(command), PSTR("AT+BANDWIDTH=%d"), (int)bandwidth);
    bool result = sendCommand(command, F("+OK"));
    if (result) managedDelay(100);
    return result;
}

RYUW122Bandwidth RYUW122::getBandwidth() {
    char response[64];
    if (sendCommandAndGetResponse(F("AT+BANDWIDTH?"), response, sizeof(response))) {
        if (strstr_P(response, PSTR("+BANDWIDTH=")) != nullptr) {
            int bwVal = safeAtoi(response + 11, -1);
            switch(bwVal) {
                case 0: return RYUW122Bandwidth::BW_850K;
                case 1: return RYUW122Bandwidth::BW_6_8M;
                default: return RYUW122Bandwidth::UNKNOWN;
            }
        }
    }
    return RYUW122Bandwidth::UNKNOWN;
}

bool RYUW122::setNetworkId(const char* networkId) {
    char command[64];
    snprintf_P(command, sizeof(command), PSTR("AT+NETWORKID=%s"), networkId);
    bool result = sendCommand(command, F("+OK"));
    if (result) managedDelay(100);
    return result;
}

bool RYUW122::getNetworkId(char* networkId) {
    if (sendCommandAndGetResponse(F("AT+NETWORKID?"), _buffer, sizeof(_buffer))) {
        if (strstr_P(_buffer, PSTR("+NETWORKID=")) != nullptr) {
            strncpy(networkId, _buffer + 11, 8);
            networkId[8] = '\0';
            return true;
        }
    }
    return false;
}

bool RYUW122::setAddress(const char* address) {
    char command[64];
    snprintf_P(command, sizeof(command), PSTR("AT+ADDRESS=%s"), address);
    bool result = sendCommand(command, F("+OK"));
    if (result) managedDelay(100);
    return result;
}

bool RYUW122::getAddress(char* address) {
    if (sendCommandAndGetResponse(F("AT+ADDRESS?"), _buffer, sizeof(_buffer))) {
        if (strstr_P(_buffer, PSTR("+ADDRESS=")) != nullptr) {
            strncpy(address, _buffer + 9, 8);
            address[8] = '\0';
            return true;
        }
    }
    return false;
}

bool RYUW122::getUid(char* uid) {
    if (sendCommandAndGetResponse(F("AT+UID?"), _buffer, sizeof(_buffer))) {
        if (strstr_P(_buffer, PSTR("+UID=")) != nullptr) {
            strncpy(uid, _buffer + 5, 16);
            uid[16] = '\0';
            return true;
        }
    }
    return false;
}

bool RYUW122::setPassword(const char* password) {
    char command[64];
    snprintf_P(command, sizeof(command), PSTR("AT+CPIN=%s"), password);
    bool result = sendCommand(command, F("+OK"));
    if (result) managedDelay(100);
    return result;
}

bool RYUW122::getPassword(char* password) {
    if (sendCommandAndGetResponse(F("AT+CPIN?"), _buffer, sizeof(_buffer))) {
        if (strstr_P(_buffer, PSTR("+CPIN=")) != nullptr) {
            strncpy(password, _buffer + 6, 32);
            password[32] = '\0';
            return true;
        }
    }
    return false;
}

bool RYUW122::setTagRfDutyCycle(int rfEnableTime, int rfDisableTime) {
    char command[32];
    snprintf_P(command, sizeof(command), PSTR("AT+TAGD=%d,%d"), rfEnableTime, rfDisableTime);
    bool result = sendCommand(command, F("+OK"));
    if (result) managedDelay(100);
    return result;
}

bool RYUW122::getTagRfDutyCycle(int& rfEnableTime, int& rfDisableTime) {
    char response[64];
    if (sendCommandAndGetResponse(F("AT+TAGD?"), response, sizeof(response))) {
        if (strstr_P(response, PSTR("+TAGD=")) != nullptr) {
            // parse two ints from response+6
            char* p = response + 6;
            char* a = strtok(p, ",");
            char* b = strtok(nullptr, ",");
            if (a) rfEnableTime = safeAtoi(a, rfEnableTime);
            if (b) rfDisableTime = safeAtoi(b, rfDisableTime);
            return true;
        }
    }
    return false;
}

bool RYUW122::setRfPower(RYUW122RFPower power) {
    char command[20];
    snprintf_P(command, sizeof(command), PSTR("AT+CRFOP=%d"), (int)power);
    bool result = sendCommand(command, F("+OK"));
    if (result) managedDelay(100);
    return result;
}

RYUW122RFPower RYUW122::getRfPower() {
    char response[64];
    if (sendCommandAndGetResponse(F("AT+CRFOP?"), response, sizeof(response))) {
        if (strstr_P(response, PSTR("+CRFOP=")) != nullptr) {
            int powerVal = safeAtoi(response + 7, -1);
            switch(powerVal) {
                case 0: return RYUW122RFPower::N65dBm;
                case 1: return RYUW122RFPower::N50dBm;
                case 2: return RYUW122RFPower::N45dBm;
                case 3: return RYUW122RFPower::N40dBm;
                case 4: return RYUW122RFPower::N35dBm;
                case 5: return RYUW122RFPower::N32dBm;
                default: return RYUW122RFPower::UNKNOWN;
            }
        }
    }
    return RYUW122RFPower::UNKNOWN;
}

bool RYUW122::anchorSendData(const char* tagAddress, int payloadLength, const char* data) {
    char command[64];
    snprintf_P(command, sizeof(command), PSTR("AT+ANCHOR_SEND=%s,%d,%s"), tagAddress, payloadLength, data);
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
    snprintf_P(command, sizeof(command), PSTR("AT+ANCHOR_SEND=%s,%d,%s"), tagAddress, payloadLength, data ? data : "");

    if (!this->serialDef.stream) return false;

    // Debug: show the command being sent
    DEBUG_PRINT(F("AT> "));
    DEBUG_PRINTLN(command);

    this->serialDef.stream->println(command);
    if (isSoftwareSerial) managedDelay(10); // Give SoftwareSerial time to switch

    unsigned long startTime = millis();
    bool receivedOk = false;
    bool receivedData = false;
    char response[64];

    // Wait for +OK and +ANCHOR_RCV response
    while ((millis() - startTime) < timeout) {
        if (readLine(*this->serialDef.stream, response, sizeof(response), timeout - (millis() - startTime))) {
            DEBUG_PRINT(F("AT< "));
            DEBUG_PRINTLN(response);

            if (strncmp_P(response, PSTR("+OK"), 3) == 0) {
                receivedOk = true;
            } else if (strncmp_P(response, PSTR("+ANCHOR_RCV="), 12) == 0) {
                // Parse the response: +ANCHOR_RCV=<TAG Address>,<PAYLOAD LENGTH>,<TAG DATA>,<DISTANCE>,<RSSI>
                char* ptr = response + 12;
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
                        *distance = safeAtoi(recvDistanceStr, *distance);
                    }
                    if (rssi && recvRssiStr) {
                        *rssi = safeAtoi(recvRssiStr, *rssi);
                    }
                    receivedData = true;
                    break;
                }
            }
        }
    }

    return receivedOk && receivedData;
}

// bool RYUW122::tagSendData(int payloadLength, const char* data) {
//     // Validate parameters
//     if (payloadLength < 0 || payloadLength > RYUW122_MAX_PAYLOAD_LENGTH) {
//         DEBUG_PRINTLN(F("Error: Invalid payload length"));
//         return false;
//     }
//     if (!data && payloadLength > 0) {
//         DEBUG_PRINTLN(F("Error: Data is null"));
//         return false;
//     }
//     if (!this->serialDef.stream) return false;
//
//     // Clear any pending data
//     while (this->serialDef.stream->available()) {
//         (void)this->serialDef.stream->read();
//     }
//
//     // Build and send command using write() for binary-safe transmission
//     DEBUG_PRINT(F("AT> AT+TAG_SEND="));
//     DEBUG_PRINT(payloadLength);
//     DEBUG_PRINT(F(","));
//     DEBUG_PRINTLN(data);
//
//     this->serialDef.stream->print("AT+TAG_SEND=");
//     this->serialDef.stream->print(payloadLength);
//     this->serialDef.stream->print(",");
//     // Use write() to send exact bytes, not snprintf which stops at null terminators
//     this->serialDef.stream->write((const uint8_t*)data, payloadLength);
//     this->serialDef.stream->print("\r\n");
//
//     if (isSoftwareSerial) managedDelay(10);
//
//     // Wait for +OK response
//     char response[64];
//     if (readLine(*this->serialDef.stream, response, sizeof(response), this->_commandTimeoutMs)) {
//         DEBUG_PRINT(F("AT< "));
//         DEBUG_PRINTLN(response);
//         return strncmp(response, "+OK", 3) == 0;
//     }
//
//     DEBUG_PRINTLN(F("AT< <no response>"));
//     return false;
// }

bool RYUW122::tagSendData(int payloadLength, const char* data) {
    char command[64];
    snprintf_P(command, sizeof(command), PSTR("AT+TAG_SEND=%d,%s"), payloadLength, data);
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
    snprintf_P(command, sizeof(command), PSTR("AT+TAG_SEND=%d,%s"), payloadLength, data ? data : "");

    if (!this->serialDef.stream) return false;

    // Debug: show the command being sent
    DEBUG_PRINT(F("AT> "));
    DEBUG_PRINTLN(command);

    this->serialDef.stream->println(command);
    if (isSoftwareSerial) managedDelay(10); // Give SoftwareSerial time to switch

    unsigned long startTime = millis();
    char response[64];

    // Wait for +OK response
    while ((millis() - startTime) < timeout) {
        if (readLine(*this->serialDef.stream, response, sizeof(response), timeout - (millis() - startTime))) {
            DEBUG_PRINT(F("AT< "));
            DEBUG_PRINTLN(response);

            if (strncmp_P(response, PSTR("+OK"), 3) == 0) {
                return true;
            }
        }
    }

    return false;
}

bool RYUW122::setRssiDisplay(RYUW122RSSI rssi) {
    char command[20];
    snprintf_P(command, sizeof(command), PSTR("AT+RSSI=%d"), (int)rssi);
    bool result = sendCommand(command, F("+OK"));
    if (result) managedDelay(100);
    return result;
}

RYUW122RSSI RYUW122::getRssiDisplay() {
    char response[64];
    if (sendCommandAndGetResponse(F("AT+RSSI?"), response, sizeof(response))) {
        if (strstr_P(response, PSTR("+RSSI=")) != nullptr) {
            int rssiVal = safeAtoi(response + 6, -1);
            switch(rssiVal) {
                case 0: return RYUW122RSSI::DISABLE;
                case 1: return RYUW122RSSI::ENABLE;
                default: return RYUW122RSSI::UNKNOWN;
            }
        }
    }
    return RYUW122RSSI::UNKNOWN;
}

bool RYUW122::setDistanceCalibration(int calibrationValue) {
    char command[20];
    snprintf_P(command, sizeof(command), PSTR("AT+CAL=%d"), calibrationValue);
    bool result = sendCommand(command, F("+OK"));
    if (result) managedDelay(100);
    return result;
}

int RYUW122::getDistanceCalibration() {
    char response[64];
    if (sendCommandAndGetResponse(F("AT+CAL?"), response, sizeof(response))) {
        if (strstr_P(response, PSTR("+CAL=")) != nullptr) {
            return safeAtoi(response + 5, 0);
        }
    }
    return 0; // Default value
}

bool RYUW122::getFirmwareVersion(char* version) {
    if (sendCommandAndGetResponse(F("AT+VER?"), _buffer, sizeof(_buffer))) {
        if (strstr_P(_buffer, PSTR("+VER=")) != nullptr) {
            strncpy(version, _buffer + 5, 16);
            version[16] = '\0';
            return true;
        }
    }
    return false;
}

bool RYUW122::factoryReset() {
    return sendCommand(F("AT+FACTORY"), F("+FACTORY"));
}

bool RYUW122::reset() {
    return sendCommand(F("AT+RESET"), F("+RESET"));
}

bool RYUW122::test() {
    return sendCommand(F("AT"), F("+OK"));
}

void RYUW122::onAnchorReceive(AnchorReceiveCallback callback) {
    _anchorReceiveCallback = callback;
}

void RYUW122::onTagReceive(TagReceiveCallback callback) {
    _tagReceiveCallback = callback;
}

bool RYUW122::sendCommand(const __FlashStringHelper* command, const __FlashStringHelper* expectedResponse, int timeout) {
    if (!this->serialDef.stream) return false;

    if (timeout == 0) timeout = (int)this->_commandTimeoutMs;

    while (this->serialDef.stream->available()) {
        (void)this->serialDef.stream->read();
    }

    DEBUG_PRINT(F("AT> "));
    DEBUG_PRINTLN(command);

    this->serialDef.stream->println(command);
    if (isSoftwareSerial) managedDelay(10);

    char response[64];
    if (readLine(*this->serialDef.stream, response, sizeof(response), timeout)) {
        DEBUG_PRINT(F("AT< "));
        DEBUG_PRINTLN(response);
        
        char expected[32];
        strncpy_P(expected, (const char*)expectedResponse, sizeof(expected));
        expected[sizeof(expected)-1] = '\0';
        
        return strncmp(response, expected, strlen(expected)) == 0;
    }
    
    DEBUG_PRINTLN(F("AT< <no response> (timeout)"));
    return false;
}

bool RYUW122::sendCommand(const char* command, const __FlashStringHelper* expectedResponse, int timeout) {
    if (!this->serialDef.stream) return false;

    if (timeout == 0) timeout = (int)this->_commandTimeoutMs;

    while (this->serialDef.stream->available()) {
        (void)this->serialDef.stream->read();
    }

    DEBUG_PRINT(F("AT> "));
    DEBUG_PRINTLN(command);

    this->serialDef.stream->println(command);
    if (isSoftwareSerial) managedDelay(10);

    char response[64];
    if (readLine(*this->serialDef.stream, response, sizeof(response), timeout)) {
        DEBUG_PRINT(F("AT< "));
        DEBUG_PRINTLN(response);
        
        char expected[32];
        strncpy_P(expected, (const char*)expectedResponse, sizeof(expected));
        expected[sizeof(expected)-1] = '\0';
        
        return strncmp(response, expected, strlen(expected)) == 0;
    }
    
    DEBUG_PRINTLN(F("AT< <no response> (timeout)"));
    return false;
}

bool RYUW122::sendCommandAndGetResponse(const __FlashStringHelper* command, char* response, int responseSize, int timeout) {
    if (!this->serialDef.stream) return false;

    if (timeout == 0) timeout = (int)this->_commandTimeoutMs;

    while (this->serialDef.stream->available()) {
        (void)this->serialDef.stream->read();
    }

    DEBUG_PRINT(F("AT> "));
    DEBUG_PRINTLN(command);

    this->serialDef.stream->println(command);
    if (isSoftwareSerial) managedDelay(10);

    if (readLine(*this->serialDef.stream, response, responseSize, timeout)) {
        DEBUG_PRINT(F("AT< "));
        DEBUG_PRINTLN(response);
        return true;
    }
    
    DEBUG_PRINTLN(F("AT< <no response> (timeout)"));
    return false;
}

bool RYUW122::sendCommandAndGetResponse(const char* command, char* response, int responseSize, int timeout) {
    if (!this->serialDef.stream) return false;

    if (timeout == 0) timeout = (int)this->_commandTimeoutMs;

    while (this->serialDef.stream->available()) {
        (void)this->serialDef.stream->read();
    }

    DEBUG_PRINT(F("AT> "));
    DEBUG_PRINTLN(command);

    this->serialDef.stream->println(command);
    if (isSoftwareSerial) managedDelay(10);

    if (readLine(*this->serialDef.stream, response, responseSize, timeout)) {
        DEBUG_PRINT(F("AT< "));
        DEBUG_PRINTLN(response);
        return true;
    }
    
    DEBUG_PRINTLN(F("AT< <no response> (timeout)"));
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
    int payloadLength = payloadStr ? safeAtoi(payloadStr, 0) : 0;
    int distance = distanceStr ? safeAtoi(distanceStr, 0) : 0;
    int rssi = rssiStr ? safeAtoi(rssiStr, 0) : 0;

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
        int payloadLength = payloadStr ? safeAtoi(payloadStr, 0) : 0;
        int rssi = rssiStr ? safeAtoi(rssiStr, 0) : 0;
        _tagReceiveCallback(payloadLength, data ? data : "", rssi);
    }

    // Also trigger simplified callback if registered
    if (_simpleMessageCallback) {
        /* use compile-time length to avoid runtime strlen on literal */
        char* ptr = response + (sizeof("+TAG_RCV=") - 1);
        strtok(ptr, ","); // Skip payload length
        char* data = strtok(nullptr, ",");
        char* rssiStr = strtok(nullptr, ",\r\n");
        int rssi = rssiStr ? safeAtoi(rssiStr, 0) : 0;
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
        managedDelay(100);
    }

    return successCount;
}

bool RYUW122::begin(RYUW122BaudRate baudRate) {
    this->bpsRate = baudRate;
    return begin();
}

// Toggle the configured reset pin: LOW for 5ms then HIGH
void RYUW122::hardwareResetPin() const {
    digitalWrite(this->lowResetTriggerInputPin, LOW);
    managedDelay(5);
    digitalWrite(this->lowResetTriggerInputPin, HIGH);
    managedDelay(5);
}

// Implement timeout setters/getters
void RYUW122::setCommandTimeout(unsigned long ms) {
    this->_commandTimeoutMs = ms;
}

unsigned long RYUW122::getCommandTimeout() const {
    return this->_commandTimeoutMs;
}

void RYUW122::setStreamTimeout(unsigned long ms) {
    this->_streamTimeoutMs = ms;
    if (this->serialDef.stream) this->serialDef.stream->setTimeout(ms);
}

unsigned long RYUW122::getStreamTimeout() const {
    return this->_streamTimeoutMs;
}

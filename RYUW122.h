/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * RYUW122 Arduino library header
 */

#ifndef RYUW122_H
#define RYUW122_H

#include "Arduino.h"
#include "includes/RYUW122_enums.h"
#include <Stream.h>

#if !defined(ARDUINO_ARCH_STM32) && !defined(ESP32) && !defined(ARDUINO_ARCH_SAMD) && !defined(ARDUINO_ARCH_MBED) && !defined(__STM32F1__) && !defined(__STM32F4__)
    #define ACTIVATE_SOFTWARE_SERIAL
#endif
#if defined(ESP32) || defined(ESP32C3)
    #define HARDWARE_SERIAL_SELECTABLE_PIN
#endif

#ifdef ACTIVATE_SOFTWARE_SERIAL
    #include <SoftwareSerial.h>
#endif

// Uncomment to enable printing out nice debug messages.
#define RYUW122_DEBUG

// Define where debug output will be printed.
#define DEBUG_PRINTER Serial

// Setup debug printing macros.
#ifdef RYUW122_DEBUG
    #define DEBUG_PRINT(...) { DEBUG_PRINTER.print(__VA_ARGS__); }
    #define DEBUG_PRINTLN(...) { DEBUG_PRINTER.println(__VA_ARGS__); }
#else
    #define DEBUG_PRINT(...) {}
    #define DEBUG_PRINTLN(...) {}
#endif

// Define the maximum payload length
#define RYUW122_MAX_PAYLOAD_LENGTH 12

// Measurement units for distance
enum class MeasureUnit {
    CENTIMETERS,
    INCHES,
    METERS,
    FEET
};

// Callback function types
typedef void (*AnchorReceiveCallback)(const char* tagAddress, int payloadLength, const char* tagData, int distance, int rssi);
typedef void (*TagReceiveCallback)(int payloadLength, const char* data, int rssi);
typedef void (*SimpleMessageCallback)(const char* fromAddress, const char* message, int rssi);
typedef void (*SimpleDistanceCallback)(const char* fromAddress, float distance, MeasureUnit unit, int rssi);

class RYUW122 {
public:
#ifdef ACTIVATE_SOFTWARE_SERIAL
    RYUW122(byte txModulePin, byte rxModulePin, RYUW122BaudRate bpsRate = RYUW122BaudRate::B_115200);
    RYUW122(byte txModulePin, byte rxModulePin, byte nodeIndicatorPin, RYUW122BaudRate bpsRate = RYUW122BaudRate::B_115200);
    RYUW122(byte txModulePin, byte rxModulePin, byte nodeIndicatorPin, byte lowResetTriggerInputPin,   RYUW122BaudRate bpsRate = RYUW122BaudRate::B_115200);
#endif

    RYUW122(HardwareSerial* serial, RYUW122BaudRate bpsRate = RYUW122BaudRate::B_115200);
    RYUW122(HardwareSerial* serial, byte nodeIndicatorPin, RYUW122BaudRate bpsRate = RYUW122BaudRate::B_115200);
    RYUW122(HardwareSerial* serial, byte nodeIndicatorPin, byte lowResetTriggerInputPin,   RYUW122BaudRate bpsRate = RYUW122BaudRate::B_115200);

#ifdef HARDWARE_SERIAL_SELECTABLE_PIN
    RYUW122(byte txModulePin, byte rxModulePin, HardwareSerial* serial, RYUW122BaudRate bpsRate);
    RYUW122(byte txModulePin, byte rxModulePin, HardwareSerial* serial, byte nodeIndicatorPin, RYUW122BaudRate bpsRate);
    RYUW122(byte txModulePin, byte rxModulePin, HardwareSerial* serial, byte nodeIndicatorPin, byte lowResetTriggerInputPin,   RYUW122BaudRate bpsRate);
#endif

#ifdef ACTIVATE_SOFTWARE_SERIAL
    RYUW122(SoftwareSerial* serial, RYUW122BaudRate bpsRate = RYUW122BaudRate::B_115200);
    RYUW122(SoftwareSerial* serial, byte nodeIndicatorPin, RYUW122BaudRate bpsRate = RYUW122BaudRate::B_115200);
    RYUW122(SoftwareSerial* serial, byte nodeIndicatorPin, byte lowResetTriggerInputPin,   RYUW122BaudRate bpsRate = RYUW122BaudRate::B_115200);
#endif

    /**
     * @brief Constructor for the RYUW122 library.
     * @param serial A pointer to the HardwareSerial or SoftwareSerial instance.
     */
    RYUW122(Stream* serial);

        /**
     * @brief Initializes the RYUW122 module.
     * @param baudRate The baud rate for the serial communication.
     * @return True if initialization is successful, false otherwise.
     */
    bool begin();

    /**
     * @brief Initializes the RYUW122 module.
     * @param baudRate The baud rate for the serial communication.
     * @return True if initialization is successful, false otherwise.
     */
    bool begin(RYUW122BaudRate baudRate);

    /**
     * @brief Checks for incoming data and processes it. This should be called in the main loop.
     */
    void loop();

    /**
     * @brief Sets the operating mode of the module.
     * @param mode The desired operating mode (TAG, ANCHOR, or SLEEP).
     * @return True if the mode was set successfully, false otherwise.
     */
    bool setMode(RYUW122Mode mode);

    /**
     * @brief Gets the current operating mode of the module.
     * @return The current operating mode.
     */
    RYUW122Mode getMode();

    /**
     * @brief Sets the UART baud rate.
     * @param baudRate The desired baud rate.
     * @return True if the baud rate was set successfully, false otherwise.
     */
    bool setBaudRate(RYUW122BaudRate baudRate);

    /**
     * @brief Gets the current UART baud rate.
     * @return The current baud rate.
     */
    RYUW122BaudRate getBaudRate();

    /**
     * @brief Sets the RF channel.
     * @param channel The desired RF channel.
     * @return True if the channel was set successfully, false otherwise.
     */
    bool setRfChannel(RYUW122RFChannel channel);

    /**
     * @brief Gets the current RF channel.
     * @return The current RF channel.
     */
    RYUW122RFChannel getRfChannel();

    /**
     * @brief Sets the data rate (bandwidth).
     * @param bandwidth The desired data rate.
     * @return True if the data rate was set successfully, false otherwise.
     */
    bool setBandwidth(RYUW122Bandwidth bandwidth);

    /**
     * @brief Gets the current data rate (bandwidth).
     * @return The current data rate.
     */
    RYUW122Bandwidth getBandwidth();

    /**
     * @brief Sets the network ID.
     * @param networkId The desired network ID (8 bytes ASCII).
     * @return True if the network ID was set successfully, false otherwise.
     */
    bool setNetworkId(const char* networkId);

    /**
     * @brief Gets the current network ID.
     * @param networkId A buffer to store the network ID.
     * @return True if the network ID was retrieved successfully, false otherwise.
     */
    bool getNetworkId(char* networkId);

    /**
     * @brief Sets the address of the module.
     * @param address The desired address (8 bytes ASCII).
     * @return True if the address was set successfully, false otherwise.
     */
    bool setAddress(const char* address);

    /**
     * @brief Gets the current address of the module.
     * @param address A buffer to store the address.
     * @return True if the address was retrieved successfully, false otherwise.
     */
    bool getAddress(char* address);

    /**
     * @brief Gets the unique ID of the module.
     * @param uid A buffer to store the unique ID.
     * @return True if the unique ID was retrieved successfully, false otherwise.
     */
    bool getUid(char* uid);

    /**
     * @brief Sets the AES128 password for the network.
     * @param password The desired password (32 characters hex string).
     * @return True if the password was set successfully, false otherwise.
     */
    bool setPassword(const char* password);

    /**
     * @brief Gets the current AES128 password for the network.
     * @param password A buffer to store the password.
     * @return True if the password was retrieved successfully, false otherwise.
     */
    bool getPassword(char* password);

    /**
     * @brief Sets the RF duty cycle for the TAG.
     * @param rfEnableTime The time in ms for RF to be enabled (10-28000ms).
     * @param rfDisableTime The time in ms for RF to be disabled (10-28000ms).
     * @return True if the duty cycle was set successfully, false otherwise.
     */
    bool setTagRfDutyCycle(int rfEnableTime, int rfDisableTime);

    /**
     * @brief Gets the current RF duty cycle for the TAG.
     * @param rfEnableTime A variable to store the RF enable time.
     * @param rfDisableTime A variable to store the RF disable time.
     * @return True if the duty cycle was retrieved successfully, false otherwise.
     */
    bool getTagRfDutyCycle(int& rfEnableTime, int& rfDisableTime);

    /**
     * @brief Sets the RF output power.
     * @param power The desired RF output power.
     * @return True if the power was set successfully, false otherwise.
     */
    bool setRfPower(RYUW122RFPower power);

    /**
     * @brief Gets the current RF output power.
     * @return The current RF output power.
     */
    RYUW122RFPower getRfPower();

    /**
     * @brief Sends data from an ANCHOR to a TAG.
     * @param tagAddress The address of the target TAG (must be 8 bytes ASCII).
     * @param payloadLength The length of the data to send (0-12 bytes maximum).
     * @param data The data to send (ASCII format).
     * @return True if the data was sent successfully, false otherwise.
     */
    bool anchorSendData(const char* tagAddress, int payloadLength, const char* data);

    /**
     * @brief Sends data from an ANCHOR to a TAG and waits synchronously for response with distance.
     * @param tagAddress The address of the target TAG (must be 8 bytes ASCII).
     * @param payloadLength The length of the data to send (0-12 bytes maximum).
     * @param data The data to send (ASCII format).
     * @param responseData Buffer to store the received data from TAG (minimum 13 bytes).
     * @param distance Pointer to store the calculated distance in cm (can be NULL).
     * @param rssi Pointer to store the RSSI value (can be NULL).
     * @param timeout Timeout in milliseconds (default 2000ms).
     * @return True if data was sent and response received successfully, false otherwise.
     * @note This is a blocking call that waits for both +OK and +ANCHOR_RCV responses.
     * @note The payload difference between Anchor and TAG cannot exceed 3 bytes for accurate distance calculation.
     */
    bool anchorSendDataSync(const char* tagAddress, int payloadLength, const char* data, char* responseData, int* distance = nullptr, int* rssi = nullptr, unsigned long timeout = 2000);

    /**
     * @brief Sends data from a TAG.
     * @param payloadLength The length of the data to send (0-12 bytes maximum).
     * @param data The data to send (ASCII format).
     * @return True if the data was sent successfully, false otherwise.
     */
    bool tagSendData(int payloadLength, const char* data);

    /**
     * @brief Sends data from a TAG and waits synchronously for confirmation.
     * @param payloadLength The length of the data to send (0-12 bytes maximum).
     * @param data The data to send (ASCII format).
     * @param timeout Timeout in milliseconds (default 1000ms).
     * @return True if data was sent and acknowledged successfully, false otherwise.
     * @note This is a blocking call that waits for +OK response.
     * @note The data is stored in the TAG module and will be transmitted when ANCHOR requests it.
     * @note The payload difference between Anchor and TAG cannot exceed 3 bytes for accurate distance calculation.
     */
    bool tagSendDataSync(int payloadLength, const char* data, unsigned long timeout = 1000);

    /**
     * @brief Enables or disables the RSSI display.
     * @param rssi The desired RSSI display setting.
     * @return True if the setting was changed successfully, false otherwise.
     */
    bool setRssiDisplay(RYUW122RSSI rssi);

    /**
     * @brief Gets the current RSSI display setting.
     * @return The current RSSI display setting.
     */
    RYUW122RSSI getRssiDisplay();

    /**
     * @brief Calibrates the distance measurement.
     * @param calibrationValue The calibration value in cm (-100 to +100).
     * @return True if the calibration was set successfully, false otherwise.
     */
    bool setDistanceCalibration(int calibrationValue);

    /**
     * @brief Gets the current distance calibration value.
     * @return The current distance calibration value.
     */
    int getDistanceCalibration();

    /**
     * @brief Gets the firmware version of the module.
     * @param version A buffer to store the firmware version.
     * @return True if the version was retrieved successfully, false otherwise.
     */
    bool getFirmwareVersion(char* version);

    /**
     * @brief Resets the module to its factory settings.
     * @return True if the reset was successful, false otherwise.
     */
    bool factoryReset();

    /**
     * @brief Performs a software reset of the module.
     * @return True if the reset was successful, false otherwise.
     */
    bool reset();

    /**
     * @brief Checks if the module is responding to AT commands.
     * @return True if the module is responding, false otherwise.
     */
    bool test();

    /**
     * @brief Registers a callback function for when an ANCHOR receives data.
     * @param callback The function to call.
     */
    void onAnchorReceive(AnchorReceiveCallback callback);

    /**
     * @brief Registers a callback function for when a TAG receives data.
     * @param callback The function to call.
     */
    void onTagReceive(TagReceiveCallback callback);

    // ========================================
    // SIMPLIFIED HIGH-LEVEL API
    // ========================================

    /**
     * @brief Gets distance from a TAG in the specified unit (ANCHOR mode only).
     * @param tagAddress The address of the target TAG (8 bytes ASCII).
     * @param unit The desired measurement unit (default: CENTIMETERS).
     * @param timeout Timeout in milliseconds (default 2000ms).
     * @return Distance in the specified unit, or -1.0 on error.
     * @note This is a blocking call. The module must be in ANCHOR mode.
     */
    float getDistanceFrom(const char* tagAddress, MeasureUnit unit = MeasureUnit::CENTIMETERS, unsigned long timeout = 2000);

    /**
     * @brief Sends a text message to a TAG from ANCHOR.
     * @param tagAddress The target TAG address (8 bytes ASCII).
     * @param message The message to send (max 12 characters).
     * @param timeout Timeout in milliseconds (default 2000ms).
     * @return True if message was sent and acknowledged, false otherwise.
     * @note This is a blocking call. The module must be in ANCHOR mode.
     */
    bool sendMessageToTag(const char* tagAddress, const char* message, unsigned long timeout = 2000);

    /**
     * @brief Sends a text message from TAG (will be transmitted when ANCHOR requests).
     * @param message The message to send (max 12 characters).
     * @param timeout Timeout in milliseconds (default 1000ms).
     * @return True if message was stored successfully, false otherwise.
     * @note This is a blocking call. The module must be in TAG mode.
     */
    bool sendMessageFromTag(const char* message, unsigned long timeout = 1000);

    /**
     * @brief Processes incoming messages and calls simplified callbacks.
     * @note Call this in your main loop() function for simplified message handling.
     */
    void processMessages();

    /**
     * @brief Registers a callback for simple message reception.
     * @param callback The function to call when a text message is received.
     */
    void onMessageReceived(SimpleMessageCallback callback);

    /**
     * @brief Registers a callback for distance measurements.
     * @param callback The function to call when a distance is measured.
     * @param unit The unit for distance reporting (default: CENTIMETERS).
     */
    void onDistanceMeasured(SimpleDistanceCallback callback, MeasureUnit unit = MeasureUnit::CENTIMETERS);

    /**
     * @brief Gets distance from multiple TAGs for trilateration (ANCHOR mode only).
     * @param tagAddresses Array of TAG addresses (each 8 bytes ASCII).
     * @param numTags Number of TAGs to query (typically 3 for 2D positioning).
     * @param distances Array to store the measured distances.
     * @param unit The desired measurement unit (default: CENTIMETERS).
     * @param timeout Timeout in milliseconds per TAG (default 2000ms).
     * @return Number of successful distance measurements.
     */
    int getMultipleDistances(const char** tagAddresses, int numTags, float* distances,
                            MeasureUnit unit = MeasureUnit::CENTIMETERS, unsigned long timeout = 2000);

private:
    HardwareSerial* hs;

#ifdef ACTIVATE_SOFTWARE_SERIAL
    SoftwareSerial* ss;
#endif

    Stream* st;

    bool isSoftwareSerial = true;

    int16_t txModulePin = -1;
    int16_t rxModulePin = -1;
    int16_t nodeIndicatorPin = -1;
    int16_t m1Pin = -1;
    int16_t m2Pin = -1;

#ifdef HARDWARE_SERIAL_SELECTABLE_PIN
    uint32_t serialConfig = SERIAL_8N1;
#endif

    int16_t lowResetTriggerInputPin = -1;

    RYUW122BaudRate bpsRate = RYUW122BaudRate::B_115200;

    struct NeedsStream {
        template<typename T>
        void begin(T &t, uint32_t baud) {
            DEBUG_PRINTLN(F("Begin "));
            t.setTimeout(500);
            t.begin(baud);
            stream = &t;
        }

#ifdef HARDWARE_SERIAL_SELECTABLE_PIN
        //		  template< typename T >
        //		  void begin( T &t, uint32_t baud, SerialConfig config ){
        //			  DEBUG_PRINTLN("Begin ");
        //			  t.setTimeout(500);
        //			  t.begin(baud, config);
        //			  stream = &t;
        //		  }
        //
        template< typename T >
        void begin( T &t, uint32_t baud, uint32_t config ) {
            DEBUG_PRINTLN(F("Begin "));
            t.setTimeout(500);
            t.begin(baud, config);
            stream = &t;
        }

        template< typename T >
        void begin( T &t, uint32_t baud, uint32_t config, int8_t txModulePin, int8_t rxModulePin ) {
            DEBUG_PRINTLN(F("Begin "));
            DEBUG_PRINT(F("TX: "));
            DEBUG_PRINT(txModulePin);
            DEBUG_PRINT(F("RX: "));
            DEBUG_PRINT(rxModulePin);

            t.setTimeout(500);
            t.begin(baud, config, txModulePin, rxModulePin);
            stream = &t;
        }
#endif

        void listen() {}

        Stream *stream;
    };
    NeedsStream serialDef;

    char _buffer[64];
    AnchorReceiveCallback _anchorReceiveCallback = nullptr;
    TagReceiveCallback _tagReceiveCallback = nullptr;
    SimpleMessageCallback _simpleMessageCallback = nullptr;
    SimpleDistanceCallback _simpleDistanceCallback = nullptr;
    MeasureUnit _preferredUnit = MeasureUnit::CENTIMETERS;

    /**
     * @brief Sends an AT command and checks for expected response.
     * @param command The AT command string to send.
     * @param expectedResponse The expected response string.
     * @param timeout Timeout in milliseconds.
     * @return True if expected response was received, false otherwise.
     */
    bool sendCommand(const char* command, const char* expectedResponse, int timeout = 1000);

    // Overload that accepts flash (F("...")) strings to save RAM
    bool sendCommand(const char* command, const __FlashStringHelper* expectedResponse, int timeout = 1000);

    /**
     * @brief Sends an AT command and retrieves the response.
     * @param command The AT command string to send.
     * @param response Buffer to store the response.
     * @param responseSize Size of the response buffer.
     * @param timeout Timeout in milliseconds.
     * @return True if response was received, false otherwise.
     */
    bool sendCommandAndGetResponse(const char* command, char* response, int responseSize, int timeout = 1000);

    /**
     * @brief Parses incoming ANCHOR_RCV messages and triggers callback.
     * @param response The response string to parse.
     */
    void parseAnchorReceive(char* response);

    /**
     * @brief Parses incoming TAG_RCV messages and triggers callback.
     * @param response The response string to parse.
     */
    void parseTagReceive(char* response);

    /**
     * @brief Checks for available data on the serial stream.
     * @return Number of bytes available to read.
     */
    int available();

    /**
     * @brief Reads a single byte from the serial stream.
     * @return The byte read, or -1 if no data available.
     */
    int read();

    /**
     * @brief Waits for module to complete transmission using nodeIndicatorPin or timeout.
     * @param timeout Maximum time to wait in milliseconds.
     * @param waitNoAux Delay time if no nodeIndicatorPin is configured.
     * @return True if completed successfully, false if timeout occurred.
     */
    bool waitCompleteResponse(unsigned long timeout, unsigned int waitNoAux = 100);

    /**
     * @brief Non-blocking delay implementation using millis().
     * @param timeout Time to wait in milliseconds.
     */
    void managedDelay(unsigned long timeout);

    /**
     * @brief Converts distance from centimeters to the specified unit.
     * @param distanceCm Distance in centimeters.
     * @param unit Target measurement unit.
     * @return Converted distance value.
     */
    float convertDistance(int distanceCm, MeasureUnit unit);
};

#endif // RYUW122_H

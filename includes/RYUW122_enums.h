/*
 * Author: Renzo Mischianti
 * Website: https://mischianti.org
 * Copyright (c) 2025 Renzo Mischianti
 * RYUW122 enums header
 */

#ifndef RYUW122_ENUMS_H
#define RYUW122_ENUMS_H

/**
 * @brief Defines the operating mode of the RYUW122 module.
 */
enum class RYUW122Mode {
    TAG = 0,    ///< TAG mode
    ANCHOR = 1, ///< ANCHOR mode
    SLEEP = 2   ///< Sleep mode
};

/**
 * @brief Defines the UART baud rates for communication with the RYUW122 module.
 */
enum class RYUW122BaudRate : uint32_t {
    B_9600 = 9600u,     ///< 9600 bps
    B_57600 = 57600u,   ///< 57600 bps
    B_115200 = 115200u  ///< 115200 bps
};

/**
 * @brief Defines the RF channel for the RYUW122 module.
 */
enum class RYUW122RFChannel {
    CH_5 = 5, ///< 6489.6MHz
    CH_9 = 9  ///< 7987.2 MHz
};

/**
 * @brief Defines the data rate (bandwidth) for the RYUW122 module.
 */
enum class RYUW122Bandwidth {
    BW_850K = 0, ///< 850 Kbps
    BW_6_8M = 1  ///< 6.8 Mbps
};

/**
 * @brief Defines the RF output power of the RYUW122 module.
 */
enum class RYUW122RFPower {
    N65dBm = 0, ///< -65dBm
    N50dBm = 1, ///< -50dBm
    N45dBm = 2, ///< -45dBm
    N40dBm = 3, ///< -40dBm
    N35dBm = 4, ///< -35dBm
    N32dBm = 5  ///< -32dBm
};

/**
 * @brief Defines the RSSI display setting.
 */
enum class RYUW122RSSI {
    DISABLE = 0, ///< Disable RSSI display
    ENABLE = 1   ///< Enable RSSI display
};

/**
 * @brief Defines the error codes returned by the RYUW122 module.
 */
enum class RYUW122ErrorCode {
    MISSING_CARRIAGE_RETURN = 1, ///< Missing carriage return or line feed
    INVALID_COMMAND_HEADER   = 2, ///< Command does not start with "AT"
    PARAMETER_FAILURE        = 3, ///< Parameter failure
    COMMAND_FAILURE          = 4, ///< Command failure
    UNKNOWN_COMMAND          = 5  ///< Unknown command
};

/**
 * @brief Return a human-readable description for a RYUW122ErrorCode value.
 * Acts like a static helper tied to the enum.
 */
static inline String RYUW122ErrorCode_description(RYUW122ErrorCode code) {
    switch (code) {
        case RYUW122ErrorCode::MISSING_CARRIAGE_RETURN:
            return String(F("Missing carriage return or line feed"));
        case RYUW122ErrorCode::INVALID_COMMAND_HEADER:
            return String(F("Command does not start with 'AT'"));
        case RYUW122ErrorCode::PARAMETER_FAILURE:
            return String(F("Parameter failure"));
        case RYUW122ErrorCode::COMMAND_FAILURE:
            return String(F("Command failure"));
        case RYUW122ErrorCode::UNKNOWN_COMMAND:
            return String(F("Unknown command"));
        default:
            return String(F("Invalid error code"));
    }
}

/**
 * @brief Parse a numeric error code (e.g. from "+ERR=n") and return description.
 * @param code Numeric error code as received from module.
 */
static inline String RYUW122ErrorCode_descriptionFromValue(uint8_t code) {
    if (code >= 1 && code <= 5) {
        return RYUW122ErrorCode_description(static_cast<RYUW122ErrorCode>(code));
    }
    return String(F("Unknown error code"));
}

// -----------------------------------------
// Additional enum description helpers
// -----------------------------------------

static inline String RYUW122Mode_description(RYUW122Mode mode) {
    switch (mode) {
        case RYUW122Mode::TAG: return String(F("TAG (mobile) mode"));
        case RYUW122Mode::ANCHOR: return String(F("ANCHOR (fixed) mode"));
        case RYUW122Mode::SLEEP: return String(F("SLEEP mode"));
        default: return String(F("Unknown mode"));
    }
}

static inline String RYUW122BaudRate_description(RYUW122BaudRate br) {
    switch (br) {
        case RYUW122BaudRate::B_9600: return String(F("9600 bps"));
        case RYUW122BaudRate::B_57600: return String(F("57600 bps"));
        case RYUW122BaudRate::B_115200: return String(F("115200 bps"));
        default: return String(F("Unknown baud rate"));
    }
}

static inline String RYUW122BaudRate_descriptionFromValue(uint32_t val) {
    switch (val) {
        case 9600: return String(F("9600 bps"));
        case 57600: return String(F("57600 bps"));
        case 115200: return String(F("115200 bps"));
        default: return String(F("Unknown baud rate"));
    }
}

static inline String RYUW122RFChannel_description(RYUW122RFChannel ch) {
    switch (ch) {
        case RYUW122RFChannel::CH_5: return String(F("Channel 5 (6489.6 MHz)"));
        case RYUW122RFChannel::CH_9: return String(F("Channel 9 (7987.2 MHz)"));
        default: return String(F("Unknown RF channel"));
    }
}

static inline String RYUW122Bandwidth_description(RYUW122Bandwidth bw) {
    switch (bw) {
        case RYUW122Bandwidth::BW_850K: return String(F("850 Kbps"));
        case RYUW122Bandwidth::BW_6_8M: return String(F("6.8 Mbps"));
        default: return String(F("Unknown bandwidth"));
    }
}

static inline String RYUW122RFPower_description(RYUW122RFPower p) {
    switch (p) {
        case RYUW122RFPower::N65dBm: return String(F("-65 dBm"));
        case RYUW122RFPower::N50dBm: return String(F("-50 dBm"));
        case RYUW122RFPower::N45dBm: return String(F("-45 dBm"));
        case RYUW122RFPower::N40dBm: return String(F("-40 dBm"));
        case RYUW122RFPower::N35dBm: return String(F("-35 dBm"));
        case RYUW122RFPower::N32dBm: return String(F("-32 dBm"));
        default: return String(F("Unknown RF power"));
    }
}

static inline String RYUW122RSSI_description(RYUW122RSSI r) {
    switch (r) {
        case RYUW122RSSI::DISABLE: return String(F("RSSI display disabled"));
        case RYUW122RSSI::ENABLE: return String(F("RSSI display enabled"));
        default: return String(F("Unknown RSSI setting"));
    }
}

#endif // RYUW122_ENUMS_H

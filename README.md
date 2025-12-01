# RYUW122 UWB Library v1.0.1

![RYUW122 Logo](https://raw.githubusercontent.com/xreef/RYUW122/refs/heads/master/resources/RYUW122_library_logo.png)

[![arduino-library-badge](https://www.ardu-badge.com/badge/RYUW122.svg?)](https://www.ardu-badge.com/RYUW122)

[![](https://img.shields.io/badge/Platform-Arduino%20%7C%20ArduinoSAMD%20%7C%20ESP32%20%7C%20ESP8266%20%7C%20RP2040%20%7C%20STM32-green.svg)]()
[![](https://img.shields.io/badge/License-MIT-lightgrey.svg)](LICENSE.md)

A comprehensive Arduino library to manage the **Reyax RYUW122** Ultra-Wideband (UWB) transceiver module. It simplifies the management of AT commands via UART to configure the device as a Tag or Anchor and read distance data for indoor positioning and ranging applications.

**Author:** Renzo Mischianti
**Website:** [www.mischianti.org](https://www.mischianti.org)
**GitHub:** [@xreef](https://github.com/xreef/RYUW122)

---

## 📚 Documentation & Tutorials

Complete tutorials and articles available on [mischianti.org](https://www.mischianti.org):

- 🌐 **[RYUW122 Library Home](https://www.mischianti.org/category/my-libraries/)** - Main library page with documentation
- 📖 **[Getting Started with UWB and Arduino](https://www.mischianti.org)** - Wiring and basic usage guide
- 🛠️ **[Library Forum & Support](https://www.mischianti.org/forums/)** - Community support

---

## 📋 Table of Contents
- [Features](#-features)
- [Supported Platforms](#-supported-platforms)
- [Installation](#-installation)
- [Wiring](#-wiring)
- [Basic Usage](#-basic-usage)
    - [Anchor: Read Distance](#anchor-read-distance)
    - [Tag: Receive Data](#tag-receive-data)
    - [Indoor Positioning (Trilateration)](#indoor-positioning-trilateration)
- [API Overview](#-api-overview)
- [Changelog](#-changelog)
- [License](#-license)

## ✨ Features

- ✅ **Tag & Anchor Modes**: Easily switch between operating modes.
- ✅ **High-Level API**: Simple methods like `getDistanceFrom()` and `sendMessageToTag()`.
- ✅ **Ranging**: Precise distance measurement (cm, m, inches, feet).
- ✅ **Data Transmission**: Send short data payloads (up to 12 bytes) between devices.
- ✅ **Async Callbacks**: Non-blocking data reception using `onTagReceive` and `onAnchorReceive`.
- ✅ **Network Configuration**: Manage Network ID, Device Address, and Encryption (AES128).
- ✅ **Sleep Mode**: Power management support.
- ✅ **Flexible Serial**: Support for HardwareSerial (ESP32, STM32, Mega) and SoftwareSerial (AVR, ESP8266).

## 🎯 Supported Platforms

| Platform | Hardware Serial | Software Serial | Notes |
|----------|-----------------|-----------------|-------|
| **ESP32** | ✅ | ✅ | Recommended (Hardware Serial 2) |
| **ESP8266** | ✅ | ✅ | SoftwareSerial recommended |
| **Arduino Mega** | ✅ | ✅ | Use Serial1, Serial2, etc. |
| **Arduino Uno/Nano**| ❌ | ✅ | Use SoftwareSerial |
| **Arduino SAMD** | ✅ | ❌ | Use Serial1 |
| **STM32** | ✅ | ❌ | Hardware Serial recommended |
| **RP2040 (Pico)** | ✅ | ✅ | GP0/GP1 or others |

## 📦 Installation

### Arduino IDE
1. Open Arduino IDE
2. Go to **Sketch > Include Library > Manage Libraries**
3. Search for "**RYUW122**"
4. Click **Install**

### PlatformIO
Add to your `platformio.ini`:
```ini
lib_deps = 
    xreef/RYUW122@^1.0.0
```

## 🔌 Wiring

The RYUW122 module communicates via UART (TX/RX).

| RYUW122 Pin | Arduino/ESP Pin | Note                                  |
|-------------|-----------------|---------------------------------------|
| VCC         | 3.3V            | Module is strictly 3.3V!              |
| GND         | GND             | Common ground                         |
| TX          | RX              | Connect to MCU RX                     |
| RX          | TX              | Connect to MCU TX                     |
| NRST        | GPIO (Optional) | Active LOW Reset pin                  |

⚠️ **5V MCU ATTENTION**: The RYUW122 module is **strictly 3.3V**. When using a 5V microcontroller (like Arduino Uno/Mega), you **must** use a logic level converter for the RX/TX lines to prevent damage. Alternatively, a [voltage divider](https://mischianti.org/voltage-divider-calculator-and-application/) can be used on the MCU's TX line.

⚠️ **ESP32 RESET PIN**: For ESP32 boards, connecting the `NRST` pin to a GPIO is **highly recommended**. The module may fail to initialize or work unreliably without a proper hardware reset managed by the library.


### Anchor: Read Distance (High Level API)

This example shows how to set up an Anchor and read the distance from a Tag.

```cpp
#include <RYUW122.h>

// ESP32 Wiring: TX=17, RX=16, RST=4
#define RX_PIN 16
#define TX_PIN 17
#define RESET_PIN 4

// Initialize: TX_PIN, RX_PIN, Serial Interface, Reset Pin
RYUW122 uwb(TX_PIN, RX_PIN, &Serial2, RESET_PIN);

const char* TAG_ADDRESS = "T1T1T1T1"; // Address of the target Tag

void setup() {
  Serial.begin(115200);
  
  // Start the module
  if (!uwb.begin()) {
    Serial.println("Initialization failed!");
    while(1);
  }

  // Set as Anchor
  uwb.setMode(RYUW122Mode::ANCHOR);
  uwb.setNetworkId("AABBCCDD");
  uwb.setAddress("ANCHOR01");
}

void loop() {
  // Get distance in Meters
  float distance = uwb.getDistanceFrom(TAG_ADDRESS, MeasureUnit::METERS);

  if (distance >= 0) {
    Serial.print("Distance to Tag: ");
    Serial.print(distance);
    Serial.println(" m");
  } else {
    Serial.println("Tag not reachable");
  }
  
  delay(500);
}
```

### Tag: Receive Data (Callback)

This example sets up a Tag that listens for messages.

```cpp
#include <RYUW122.h>

// Arduino Uno Wiring with SoftwareSerial
// RYUW122 TX -> Pin 10, RYUW122 RX -> Pin 11
RYUW122 uwb(11, 10, RYUW122BaudRate::B_9600);

void onMessageReceived(const char* fromAddress, const char* message, int rssi) {
  Serial.print("Message from: "); Serial.println(fromAddress);
  Serial.print("Content: "); Serial.println(message);
  Serial.print("RSSI: "); Serial.println(rssi);
}

void setup() {
  Serial.begin(115200);
  
  if (!uwb.begin()) {
    Serial.println("Init failed!");
    while(1);
  }

  uwb.setMode(RYUW122Mode::TAG);
  uwb.setNetworkId("AABBCCDD");
  uwb.setAddress("T1T1T1T1");

  // Register callback
  uwb.onMessageReceived(onMessageReceived);
}

void loop() {
  // Keep the listener active
  uwb.loop();
}
```

### Indoor Positioning (Trilateration)

The library supports getting distances from multiple tags/anchors to perform trilateration. See the example `anchor_esp32_position_oled.ino` included in the library for a full implementation of a mobile anchor calculating its position relative to fixed nodes.

## 🛠 API Overview

Here is a comprehensive overview of the public methods available in the library.

### Initialization & Main Loop
```cpp
// Constructors for SoftwareSerial, HardwareSerial, and Stream
RYUW122(byte tx, byte rx, ...);
RYUW122(HardwareSerial* serial, ...);
RYUW122(Stream* serial);

// Initialize the module
bool begin();
bool begin(RYUW122BaudRate baudRate);

// Process incoming data (must be in loop())
void loop();
```

### Module Configuration
```cpp
// Set operating mode
bool setMode(RYUW122Mode mode);
RYUW122Mode getMode();

// Set network parameters
bool setNetworkId(const char* networkId);
bool getNetworkId(char* networkId);
bool setAddress(const char* address);
bool getAddress(char* address);
bool getUid(char* uid);

// Set encryption password (AES128)
bool setPassword(const char* password);
bool getPassword(char* password);

// Hardware reset
void reset();
```

### RF & Communication Parameters
```cpp
// Set UART baud rate
bool setBaudRate(RYUW122BaudRate baudRate);
RYUW122BaudRate getBaudRate();

// Set RF channel
bool setRfChannel(RYUW122RFChannel channel);
RYUW122RFChannel getRfChannel();

// Set RF bandwidth (data rate)
bool setBandwidth(RYUW122Bandwidth bandwidth);
RYUW122Bandwidth getBandwidth();

// Set RF power
bool setRfPower(RYUW122RFPower power);
RYUW122RFPower getRfPower();

// Set Tag's RF duty cycle
bool setTagRfDutyCycle(int rfEnableTime, int rfDisableTime);
bool getTagRfDutyCycle(int& rfEnableTime, int& rfDisableTime);
TagDutyCycleResponse getTagRfDutyCycle();
```

### High-Level Ranging & Messaging
```cpp
// Get distance from a single Tag (blocking)
float getDistanceFrom(const char* tagAddress, MeasureUnit unit = MeasureUnit::CENTIMETERS);

// Get distances from multiple Tags (blocking)
int getMultipleDistances(const char** tags, int numTags, float* distances, MeasureUnit unit = MeasureUnit::CENTIMETERS);

// Send a message to a Tag (blocking)
bool sendMessageToTag(const char* tagAddress, const char* message);

// Send data and get a response with distance (blocking struct version)
AnchorResponse anchorSendDataSync(const char* tagAddress, int payloadLength, const char* data, unsigned long timeout = 2000);

// Send data from a Tag (non-blocking, stores data for Anchor to poll)
bool tagSendData(int payloadLength, const char* data);
```

### Asynchronous Callbacks
```cpp
// Register a callback for when an Anchor receives data from a Tag
void onAnchorReceive(AnchorReceiveCallback callback);

// Register a callback for when a Tag receives data from an Anchor
void onTagReceive(TagReceiveCallback callback);

// Simplified message callback
void onMessageReceived(SimpleMessageCallback callback);

// Simplified distance callback
void onDistanceReceived(SimpleDistanceCallback callback);
```

### Low-Level AT Command
```cpp
// Send a raw AT command and get the response
String sendCommand(const char* command, unsigned long timeout = 1000);
```

## 📝 Changelog

 - v1.0.1 2025-12-01: Fix examples and update documentation 
 - v1.0.0 2025-10-10):
   - Initial release
   - Support for Tag and Anchor modes
   - High-level API for distance and messaging
   - Trilateration helper functions
   - Sync and Async operation modes

## 📄 License

MIT License - See LICENSE file for details

Copyright (c) 2025 Renzo Mischianti

## 🤝 Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## 📞 Support

- **Website**: https://www.mischianti.org
- **GitHub Issues**: https://github.com/xreef/RYUW122/issues
- **Forum**: https://www.mischianti.org/forums/

## 👤 Author

**Renzo Mischianti**
- **Website**: https://www.mischianti.org
- **Email**: renzo.mischianti@gmail.com
- **GitHub**: @xreef

Made with ❤️ by Renzo Mischianti

https://downloads.arduino.cc/libraries/logs/github.com/xreef/RYUW122/
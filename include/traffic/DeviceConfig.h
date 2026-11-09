#pragma once

// Device-specific identity, GPIOs and display geometry. No wiring changes in this extraction.
#define OTA_Title "Gate Counter"
#include "FirmwareVersion.h" // Version is edited at the top of src/main.cpp.
#define THIS_MQTT_CLIENT "espGateCounter"
#define beamAPin 32 // Beam A: existing wiring retained
#define beamBPin 33
#define magneticSensorPin 25 // Independent EL817 diagnostic input, 3.3 V MCU side
#define DHTPIN 4
#define DHTTYPE DHT22
#define PIN_SPI_CS 5
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
// Both baselines use 128x64 logical geometry; physical screen sizes differ.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define MSG_BUFFER_SIZE (500)

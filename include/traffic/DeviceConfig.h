#pragma once

// Device-specific identity, GPIOs and display geometry. No wiring changes in this extraction.
#define OTA_Title "Gate Counter"
#define FWVersion "25.12.28.0-modular-test.1"
#define THIS_MQTT_CLIENT "espGateCounter"
#define magSensorPin 32 // Legacy name: Beam A in production dual-beam firmware
#define beamSensorPin 33
#define DHTPIN 4
#define DHTTYPE DHT22
#define PIN_SPI_CS 5
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C
// Both baselines use 128x64 logical geometry; physical screen sizes differ.
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define MSG_BUFFER_SIZE (500)

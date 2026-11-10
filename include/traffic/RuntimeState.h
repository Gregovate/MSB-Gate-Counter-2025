#pragma once
#include "Platform.h"
#include "DeviceConfig.h"
#include "MqttTopics.h"

// Transitional shared state. Defined exactly once in RuntimeState.cpp.
// Service ownership is documented in Modular_Firmware.md.
enum GateDetectState {
    WAITING_FOR_CAR,
    BEAM_A_HIGH,
    BOTH_BEAMS_HIGH,
    CAR_DETECTED
};

extern String showStartDate;
extern bool showStartDateValid;
extern int showStartYear;
extern int showStartMonth;
extern int showStartDay;
extern const int showStartMin;
extern const int showEndMin;
extern bool rtcReady;
extern String bootTimestamp;
extern int mqttKeepAlive;
extern unsigned long lastKeepAliveMillis;
extern char topic[60];
extern char topicBase[60];
extern const unsigned long debounceDelay;
extern bool stableRawA;
extern bool stableRawB;
extern unsigned long lastRawAChangeMs;
extern unsigned long lastRawBChangeMs;
extern GateDetectState gateDetectState;
extern unsigned long beamATripTime_ms;
extern unsigned long bothBeamsBroken_ms;
extern unsigned long lastCarDetected_ms;
extern unsigned long firstBeamHealth_ms;
extern unsigned long secondBeamHealth_ms;
extern unsigned long timeBetweenCars_ms;
extern bool carPresentFlag;
extern bool gateStuckAlarmActive;
extern unsigned long abFollow_ms;
extern unsigned long timeToPassMS;
extern bool abTimeoutLogged;
extern const unsigned long minActivationDuration;
extern const unsigned long maxABFollow_ms;
extern unsigned long carDetectMS;
extern unsigned long gateCounterTimeout;
extern AsyncWebServer server;
extern String currentDirectory;
extern unsigned long ota_progress_millis;
extern RTC_DS3231 rtc;
extern const char* ampm;
extern const char* ntpServer;
extern const long gmtOffset_sec;
extern const int daylightOffset_sec;
extern DHT dht;
extern float tempF;
extern float humidity;
extern Adafruit_SSD1306 display;
extern const int line1;
extern const int line2;
extern const int line3;
extern const int line4;
extern const int line5;
extern const int line6;
extern const int line7;
extern WiFiMulti wifiMulti;
extern WiFiClient espGateCounter;
extern uint16_t connectTimeOutPerAP;
extern PubSubClient mqtt_client;
extern unsigned long lastMsg;
extern char msg[MSG_BUFFER_SIZE];
extern bool loggingEnabled;
extern bool mqtt_connected;
extern bool wifi_connected;
extern int wifi_connect_attempts;
extern bool hasRun;
extern unsigned int dayOfMonth;
extern unsigned int lastDayOfMonth;
extern unsigned int currentHr12;
extern unsigned int currentHr24;
extern unsigned int currentMin;
extern unsigned int currentSec;
extern unsigned int daysRunning;
extern unsigned int currentTimeMinute;
extern int totalDailyCars;
extern int totalShowCars;
extern int inParkCars;
extern int carCounterCars;
extern int lastcarCounterCars;
extern int beamAState;
extern int lastBeamAState;
extern int beamBState;
extern int lastBeamBState;
extern unsigned long triggerTime;
extern unsigned long beamSensorAlarm;
extern const unsigned long wifi_connectioncheckMillis;
extern const unsigned long mqtt_connectionCheckMillis;
extern unsigned long start_MqttMillis;
extern unsigned long start_WiFiMillis;
extern char buf2[25];
extern bool flagDaysRunningReset;
extern bool flagMidnightReset;
extern bool flagDailyShowStartReset;
extern bool flagDailySummarySaved;
extern bool flagDailyShowSummarySaved;
extern bool flagHourlyReset;
extern bool showTime;
extern bool resetFlagsOnce;
extern File myFile;
extern String seasonFolder;
extern String fileName1;
extern String fileName2;
extern String fileName3;
extern String fileName4;
extern String fileName5;
extern String fileName6;
extern String fileName7;
extern String fileName8;
extern String fileName9;
extern String fileName10;
extern String fileNameShowStart;
extern unsigned int hourlyCount[24];
extern float hourlyTemp[24];
extern char days[7][4];
extern char months[12][4];
extern std::queue<String> publishQueue;
extern const size_t MAX_QUEUE;
extern const size_t MAX_FLUSH_PER_CALL;

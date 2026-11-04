/*
Gate Counter by Greg Liebig greg@engrinnovations.com
Initial Build 12/5/2023 12:15 pm
Counts vehicles as they exit the park
Connects to WiFi and updates RTC on Boot
Uses an Optocoupler to read buried vehicle sensor for Ghost Controls Gate operating at 12V
Purpose: supplements Car Counter to improve traffic control and determine park capacity
Uses an Optocoupler to read buried vehicle sensor for Ghost Controls Gate operating at 12V
DOIT DevKit V1 ESP32 with built-in WiFi & Bluetooth
*/
#define OTA_Title "Gate Counter" // OTA Title
#define FWVersion "25.12.28.0"   // Remove A->B timeout reset (diagnostic only) to prevent undercount during congestion/platooning
#define THIS_MQTT_CLIENT "espGateCounter" // This MQTT Client Name

/*  ## BEGIN CHANGELOG GATE COUNTER ##
25.12.28.0  - Critical: Removed A->B timeout as a hard reset in BEAM_A_HIGH. Timeout is now diagnostic-only (logs once per event).
                Purpose: prevent undercount on high-volume nights where congestion/platooning delays Beam B.
            - Added per-event reset of abTimeoutLogged at event start and on BEAM_A_HIGH exit paths (prevents log carryover/spam).
25.12.05.0   Removed beamA trip time and replaced with timeBetweenCars_ms in ExitLog.csv
25.12.03.3   GateCounter UI and SD file manager alignment with CarCounter 2025:
             - Added visible "View Show Summary" button in index.html and applied
               Gate theme styling so the element renders correctly on te lighter
               Gate UI background.
             - Updated GateCounter CSS for .btn elements (blue background, white text,
               proper hover transitions) to ensure consistent appearance across UI.
             - Confirmed showSummary.html integration and CSV viewer loading through
               /ShowSummary.csv with appropriate error handling.
             - Added HTTP /delete and /rename routes using the unified buildPath()
               helper so SD file operations behave identically to CarCounter 2025.
             - Both routes honor currentDirectory and return proper HTTP status codes
               for invalid parameters, missing files, and SD unavailability.
             - No changes to car-detection logic, MQTT topics, or timing behavior.
25.12.03.1   Added CarCounter-style idle beam-health monitoring:
             - If Beam A or Beam B remains HIGH (broken) for >= gateCounterTimeout
               while in WAITING_FOR_CAR, publish ALARM_GATE_STUCK (retained).
             - Added debug entries: "Beam A stuck HIGH (idle)" and
               "Beam B stuck HIGH (idle)".
             - Alarm auto-clears when both beams return LOW in WAITING_FOR_CAR.
             No changes to BEAM_A_HIGH, BOTH_BEAMS_HIGH, or CAR_DETECTED state logic.
25.12.03.0   Hardened GateCounter alarm model for 2025 season:
             - Stuck-vehicle alarm in BOTH_BEAMS_HIGH is now properly latched:
               publishes "ALARM_GATE_STUCK" only once when gateCounterTimeout
               is exceeded (guarded by !gateStuckAlarmActive).
             - All ALARM and CLEAR publishes are now retained to ensure Home
               Assistant always receives the current alarm state after reconnect.
             - Added boot-time retained CLEAR publish immediately after
               MQTTreconnect() so HA never starts in an undefined alarm state.
             - Updated CLEAR behavior in BOTH_BEAMS_HIGH to use retained publish
               and reset the latch cleanly.
             - No changes made to the detection state machine logic (WAITING_FOR_CAR,
               BEAM_A_HIGH, BOTH_BEAMS_HIGH, CAR_DETECTED) beyond alarm handling.
             - No beam-health timers or FIRST_BEAM stuck logic added yet; these
               will be implemented in upcoming versions.
25.11.30.2   Replaced rtc.toString(buf2) with explicit timestamp formatting in
             countTheCar() to fix frozen timestamps in ExitLog.csv and MQTT
             time publish. Now uses a local timeBuf built with snprintf so each
             logged car gets a correct, advancing DateTime. Changed Rest time to 5:08PM
25.11.30.1   Fixed TimeToPass_ms logging order so correct TTP is written for the
             current car. Removed tempF from ExitLog.csv output. Added
             AB_Follow_ms to ExitLog.csv. Updated CSV write order and created
             new header: "DateTime, TimeToPass_ms, ExitDailyTotal, InParkCars,
             AB_Follow_ms, BeamA_Trip_ms".
25.11.30.0   Added /reboot for webserver. Changed variable name abFollow to
            abFollow_ms for clarity.
25.11.29.0   Fixed SD File Manager path handling across all operations 
                (download, upload, delete, and directory changes) by applying 
                unified buildPath() helper. Eliminated malformed paths such as 
                //gc/2025 and restored proper file access.
             Corrected header creation logic in checkAndCreateFile() to use 
                println(), preventing header/data merge issues in ExitLog.csv 
                 and ShowSummary.csv.
             Increased maxABFollow_ms from 750ms to 900ms to prevent valid 
                 slow A→B beam transitions from being discarded. Addresses 
                undercounts observed during stopped/slow traffic.
             Aligned upload/download path behavior with directory navigation 
                to ensure consistent SD card file management.
             No functional OTA or MQTT changes in this version; all updates 
                limited to SD file logic and beam timing.
25.11.28.1   Increased DHT sensor read interval from 10 seconds to 60 seconds to 
                 reduce sensor polling frequency and added a warm-up period on boot.
                 mimics Car Counter behavior.
25.11.28.0  - Standardized stuck/alarm behavior for Gate Counter:
                * When BOTH_BEAMS_HIGH exceeds gateCounterTimeout, now publish:
                    publishMQTT(MQTT_PUB_ALARM, "ALARM_GATE_STUCK", false);
                    publishMQTT(MQTT_COUNTER_LOG, "Sensor blocked", false);
                This matches the Car Counter pattern and unifies the stuck message text.
            - Cleaned up System/hello usage:
                * Replaced all publishHelloEvent("...") calls in the MQTT callback with
                publishDebugLog("...") so that config/status updates go to:
                    msb/traffic/GateCounter/System/debug
                and System/hello remains a retained ONLINE JSON status only.
                * Removed the unused publishHelloEvent() helper wrapper.
            - No changes to MQTT topics, unique IDs, timeout tuning behavior, or heartbeat payload.
25.11.27.0  Added CarCounter calendar mirroring support:
                - Added subscriptions for CarCounter dayOfMonth and daysRunning.
                - Added callback handlers to mirror CarCounter calendar values into GateCounter.
                - GateCounter now republishes mirrored values under its own Calendar topics
                  (msb/traffic/GateCounter/Calendar/DayOfMonth and DaysRunning).
                - Prevents overnight drift between devices; CarCounter is now authoritative.
             Restored publication of timing configuration values:
                - gateCounterTimeout (ms) now republished as retained.
                - carDetectMS (ms) republished as retained.
                - Ensures visibility in HA and MQTT Explorer even after reboot.
             Minor cleanup of callback ordering for new config topics.
25.11.26.1  GAL Standardized System/hello payload to JSON object:
               { "device":..., "status":..., "fw":..., "boot":..., "msg":... }
             Added publishHello(status,msg,retain) helper for retained
               connection/update events.
             Added publishHelloEvent() for non-retained config/reset events.
             Removed legacy string HELLO publishes.
             Updated MQTTreconnect() to publish retained JSON ONLINE status.
             Aligned Home Assistant integration with new JSON format.
25.11.26.0  Updated MQTT topic tree + dual-beam telemetry refinements
            - Adopted 2025 clean topic structure:
                /System /Env /Cars /Calendar /Sensors /Config
            - Standardized WiFi diagnostics under /System/wifi/
            - Standardized hourly publish topic to /Cars/Hour
            - Added retained season metadata publishes:
                System/seasonFolder, System/seasonYear
            - Added A→B follow-time telemetry:
                Sensors/beamAB_ms
            - Added Beam B broken-duration timing:
                Sensors/beamB_broken_ms
            - Updated dual-beam state machine logic for parity with CarCounter
            - Unified timeToPass (TTP) and timeBetweenCars_ms telemetry
25.11.24.2  Added separate keep-alive timer in KeepMqttAlive() to publish
             select MQTT state values every 30 seconds if no cars are counted,
             ensuring remote dashboards stay updated during low traffic periods.
             Independent of publishMQTT() resets. Added Retained flag to beam states
             and betweenCars publishes.
25.11.24.1  Synced GateCounter MQTT sensor definitions with updated HA
                sensor files. Removed all year-based `_2025` unique_id
                suffixes and standardized entity IDs for long-term stability.
             Rebuilt mqtt_configs/sensors/gatecounter.yaml to match the
                2025 topic tree (`/System`, `/Env`, `/Cars`, `/Calendar`,
                `/Sensors`), ensuring consistent alignment with firmware.
             Cleaned up legacy/duplicate HA entities (_2, _2025, _3 ghosts)
                and restored stable entity naming for dashboards.
             No firmware logic changes, no heartbeat modifications included
                in this commit.
25.11.24.0  Aligned Gate Counter with Car Counter 2025 MQTT/state model.
             • Converted all state MQTT publishes to retained (ExitTotal, InParkCars,
               ShowTotal, DayOfMonth, DaysRunning, HourlyCounts).
             • Fixed multiple duplicate publishes and missing retain flags.
             • Reworked get* /save* SD routines for deterministic state reload on reboot.
             • Implemented retained hourly bucket publishes after reboot.
             • Standardized all dynamic MQTT topics to 2025 tree (…/Cars/Hourly/HH).
             • Corrected MQTT queue behavior and removed non-retained state leaks.
             • Cleaned up saveDailyTotal(), getDailyTotal(), saveDaysRunning(),
               getDaysRunning(), saveShowTotal(), getShowTotal(), saveDayOfMonth(),
               getDayOfMonth() and saveHourlyCounts().
             • Fixed broken brace in saveDailyShowSummary() that caused
               downstream function nesting compile errors.

25.11.22.5  Removed heatbeat MQTT topic ccountcar() to KeepMqttAlive() function to publish current counts
             every 30 seconds if no car is counted, ensuring remote dashboards
             stay updated during low traffic periods.
25.11.22.4  Added retained “online” MQTT debug event on connect with timestamp,
             SSID, RSSI, and IP for remote diagnostics (publishDebugEvent("online", ...)).
             Updated HELLO banner to include boot timestamp (“Gate Counter ONLINE @ …”).
             Standardized JSON temp/humidity publish on connect.
25.11.22.3  Added retained “online” MQTT debug event on connect with timestamp,
             SSID, RSSI, and IP for remote diagnostics (publishDebugEvent("online", ...)).
             Updated HELLO banner to include boot timestamp (“Gate Counter ONLINE @ …”).
             Standardized JSON temp/humidity publish on connect.
25.11.22.2  Cleaned up 2025 dual-beam MQTT topics to remove legacy Mag/Beam naming.
             New topics: beamAState, beamBState, beamAB_ms (A→B follow),
             and beamB_broken_ms (Beam B broken duration).
             Added A→B follow timing publish at Beam-B break edge for diagnostics.
             Enabled temporary dual-publishing of legacy topics (beam-high_ms and old state topics)
             for backward HA/Grafana compatibility. No changes to car-detection logic.
25.11.22.1  Fixed build break after CarCounter parity merge by relocating new
             dual-beam state-machine globals back to the main globals section.
             Ensured beamATripTime_ms is globally visible for SD logging in countTheCar().
             No logic changes from 25.11.22.0 — compile/order fix only.
25.11.22.0  Replaced legacy timing-based car detection with CarCounter 2025 state-machine logic.
             Added debounced raw beam sampling (50 ms) for stable A/B transitions.
             Implemented 4-state detection flow (WAITING_FOR_CAR → A_BROKEN → BOTH_BROKEN → CAR_DETECTED)
             matching CarCounter beam spacing and event timing.
             Preserved all GateCounter MQTT topics, payloads, and TTP/BetweenCars reporting.
             Integrated 750 ms A→B follow rule and retained carDetectMS duration threshold.
             Eliminated early re-arm behavior responsible for double counts on long vehicles.
             Beam polarity mapping unchanged (HIGH=broken, LOW=clear).

25.11.21.1  Inverted beam polarity mapping to match 2025 optocoupler outputs
             (LOW=clear, HIGH=broken). Boot-state and detectCar() logic now align
             with field-tested behavior.
25.11.21.0  Renamed all legacy MagSensor identifiers to Beam A / Beam B for 2025 dual-beam hardware.
             Updated all related state variables, timers, and debug messages to match new naming.
             Removed obsolete mag-sensor debug events no longer valid with dual IR beams.
             Verified mapping for normally-HIGH IR optocoupler beams and preserved 2024-proven logic.
             Ensured initial beam states publish correctly on reboot to avoid stale MQTT data.
25.11.20.5  Debugging Inpark Cars
25.11.20.4  Restored the proven 2024 car-detection state machine and added proper
             input-polarity mapping for the new normally-HIGH IR beams via optocoupler.
             Corrected multi-count behavior by adding per-event locking to prevent
             additional counts when Beam B cycles while Beam A remains broken
             (common in heavy, stop-and-go exit traffic). Car detection now triggers
             only once per true Beam-A event and resets only after both beams clear.
             Retains all legacy timing features: 750 ms Beam-A pre-trigger window,
             carDetectMS duration threshold, between-car timing, and TTP reporting.

25.11.20.3  Corrected beam logic inversion in dual-beam detection. Inputs from the
             new optocoupler IR receivers are normally-HIGH, LOW when broken.
             Restored 2024 state-machine semantics (LOW = beam broken) to match
             proven car-counter behavior. Eliminated false state transitions and
             corrected CAR_DETECTED trigger to fire only after B clears. System
             now mirrors the stable 2024 timing and detection reliability.
25.11.20.2  Updated dual-beam integration to use the proven 2024 state machine.
             Re-mapped magSensorPin to Beam A and aligned raw pin reads to the
             state machine’s expected logic (1 = broken, 0 = clear).
             Restored all timing behavior from last season, including
             carDetectMS threshold, 750 ms pre-trigger window, and B-duration
             validation. Removed experimental dual-beam function and corrected
             pinMode configuration for optocoupler inputs.
25.11.20.1  Repurposed magSensorPin as Beam A; implemented dual-beam detection;
            removed inverted magSensor logic and aligned inputs to normally-HIGH beams.
25.11.18.1  Removed SD/RTC infinite while(1) loops to prevent hard lockups;
            Added MQTT firmware publish on connect.
24.12.28.1 Bug in alarm logic removed.
24.12.26.1 Added alarm if beam sensor remains high for more than 3 minutes
24.12.19.6 Removed temperature array from average procedure an used the global declared array
24.12.19.5 Accidentally removed mqtt_client.setCallback(callback) Fixed with a forward declaration
24.12.19.4 Added save saveHourlyCounts() to countTheCar() and removed save from OTA update
24.12.19.3 File comparrison between Car Counter and Gate Counter. Synced shared code
24.12.19.2 fixed warnings, removed unused variables, Changed waitduration to carDetectMS Only errors are with html. Changed carDetectMS default 1200
24.12.19.1 Included saving hourly data before uploading new firmware, Set hostname, MultiMQTT, modified secrets.h
24.12.18.6 Changed flags to save certain data hourly flagHourlyReset
24.12.18.5 Fixed MQTT dymanic topic in CountTheCar for hourly totals 
24.12.18.4 added  pinMode(DHTPIN, INPUT_PULLUP) for DHT Sensor getting bad readings.  77.1% (used 1010645 bytes from 1310720 bytes)
24.12.18.3 Added new stat Time Between Cars. Put back BeamSensor High Time
24.12.18.2 put publishing state changes beamBState and beamAState and timeToPassMS. detectCar() finally working reliably!
24.12.18.1 Renamed hourlyCarCount[] to hourlyCount[] and finished comparison to Car Counter Code
24.12.17.4 added new topic MQTT_COUNTER_LOG "msb/traffic/GateCounter/CounterLog"
24.12.17.3 more tweaks to detectCar() revised averageHourlyTemp() & readTempandRH() removed averageHourlyTemp() from Loop
24.12.17.2 modified detectCar() based on logging data increased predetect mag sensor to 750ms
24.12.17.1 modified downloadSDFile(AsyncWebServerRequest *request) to save file with correct filename
24.12.16.5 Added logging function to plot sensor data. Changes getHourlyData() & saveHoulyCounts()
24.12.16.4 Reset counts was incorrect due to syncing code from car counter to gate counter should be 5:10 pm for gate counter
24.12.16.3 was not incrementing hourly car counts and writing multiple rows for the same date
24.12.16.2 removed all references to tempF from RTC sensor moved to DHT22 sensor publish every 10 min publish temp & RH json format
24.12.16.1 revised saveDailyShowSummary() times to 9:20 for gate counter
24.12.15.4 revised saveDailyShowSummary() to average temps during show
24.12.15.3 after debugging in field after 15.2
24.12.15.2 Matched Car Counter procedures, Changed Names, Added webserver
24.12.14.2 added getdayofmonth() after updatingdayofmonth()
24.12.14.1 Added DHT22 sensor to gate counter. Revised carDetect to weight beamSensor higher and used magSensor for additional Confirmation
24.12.12.3 Tweaks to state machine for improving accuracy. Added timers for analysis topics 16 & 18
24.12.12.2 Revising detectCars() State machine logic for new sensor
24.12.12.1 BREAKING CHANGE Replaced reflective sensor with through beam nomally closed invert reading
24.12.02.3 added timer for timeDiff between magSensor HIGH and beamSensor HIGH and timer between cars
24.12.02.1 revised state machine again for through-beam sensor and removed bounce detection for beam
24.12.01.3 revised state machine again for beam sensor bouncing during car detection
24.12.01.2 revised state machine for beam sensor bouncing
24.12.01.1 converted car dection to state machine in separate branch
24.11.30.3 Totally Reworked Car Detection
24.11.30.2 Refactored publishMQTT and File Checking and Creation and for loop hour counts
24.11.30.1 Updated MQTT callback section to clean up potential memory leak
24.11.27.1 Changed mqtt publish outside File writes, changed daily totals to string pointer, tempF to float
24.11.26.1 Changed Update to days running since they were doubling on date change
24.11.25.3 Cleaned up MQTT Topics
24.11.25.2 Added in state change in loop to publish mag sensor states. Missed 5 cars during dog show.
24.11.25.1 Added MQTT Topics for Remote Reset to match Car Counter. Added Alarm for blocked beam sensor
24.11.19.1 Replace Current Day & Calday with DayOfMonth. Added boolean to print daily summary once
24.11.18.1 Added publishing totals on manual reset
24.11.15.1 Removed Sensor Bounces, updated MQTT Topics
24.11.14.1 Eliminated mqtt timeout, debug topic, added TTP to mqtt
24.11.10.2 Miscellaneous formatting issues before re-creating JSON branch again
24.11.10.1 Fixed bounce check, changed filename methods merged Arduino json branch
24.11.9.2 Added mqtt publish when car counter cars updates
24.11.9.1 Added mqtt loop to while loop. Working code excluding elegantota update
24.11.8.1 testing beamSensorBoune time
24.11.6.2 Increased carDetectTime from 500 to 750 millis Sensor bounces with my truck
24.11.6.1 Changed MQTT Topics for GateCounter rather than exit
24.11.5.1 Fixed wrong publishing topic for carCounter Counts
24.11.4.1 Removed Bounce Logic for Beam Sensor and associated vatiables added logic for show totals
24.10.28.1 Created proceedure for Updating Car Counts
24.10.27.1 simplified car detect logic, Formatting Changes
24.10.24.1 Fixed Errors in MQTT variables
24.10.23.3 Fixed File creation errors
24.10.23.2 Added update/reset check in loop for date changes. Created initSDCard(). 
24.10.23.1 Updated totals, bug fixes, files ops comparrison to Gate counter  added file ops
24.10.17.2 added #define FWVersion
24.10.15.0 Fixed Pin problem. Beam & mag sensor swapped causing the problems. Purpose: suppliments Car Counter to improve traffic control and determine park capacity
23.12.13.0 Changed time format YYYY-MM-DD hh:mm:ss 12/13/23
*/
// ------ END CHANGELOG  ------

#include <Arduino.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "RTClib.h"
#include "NTPClient.h"
//#include <WiFiClientSecure.h>
#include <WiFiMulti.h>
#include "secrets.h"
#include "time.h"
#include "SD.h"
#include "FS.h"
#include "SPI.h"
#if defined(ESP8266)
  #include <ESP8266WiFi.h>
  #include <ESPAsyncTCP.h>
#elif defined(ESP32)
  #include <WiFi.h>
  #include <AsyncTCP.h>
#endif
#include <ESPmDNS.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTAPro.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <queue>  // Include queue for storing messages

// ******************** CONSTANTS *******************

#define magSensorPin 32 // Pin for Magnotometer Sensor
#define beamSensorPin 33  //Pin for Reflective Beam Sensor
#define DHTPIN 4       // GPIO pin for the DHT22
#define DHTTYPE DHT22  // DHT TYPE
#define PIN_SPI_CS 5   // The ESP32 pin GPIO5
#define OLED_RESET -1  // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

//GATE COUNTER GLOBAL CONSTANTS FOR SHOW TIMES and SAFETY
// ===== Show start date integration (from Car Counter) =====
String showStartDate = "";        // "YYYY-MM-DD"
bool showStartDateValid = false;  // true once parsed
// Show start date (set by CarCounter via MQTT)
static int showStartYear = 0;
static int showStartMonth = 0;
static int showStartDay = 0;
const int showStartMin = 17 * 60 + 10; // 5:10 PM in minutes
const int showEndMin = 21 * 60 + 20;   // 9:20 PM in minutes (including additional checks till 9:20 PM)
bool rtcReady = false;  // GAL 25-11-22: RTC boot-safety guard
String bootTimestamp = "";  // GAL 25-11-22: Store boot timestamp for logging
// **************************************************

/***** MQTT TOPIC DEFINITIONS *****/

int mqttKeepAlive = 30; // publish select values every x seconds to keep MQTT client connected
// Keepalive timer independent of publishMQTT() resets
unsigned long lastKeepAliveMillis = 0;

// Base path
char topic[60];
char topicBase[60];
#define topic_base_path "msb/traffic/GateCounter"

// =====================================================
// NEW 2025 TOPIC TREE (NO LEGACY)
// Buckets: /System /Env /Cars /Calendar /Sensors /Config
// =====================================================

// ---------------- SYSTEM ----------------
#define MQTT_PUB_HELLO          "msb/traffic/GateCounter/System/hello"
#define MQTT_PUB_FIRMWARE       "msb/traffic/GateCounter/System/firmware"
#define MQTT_PUB_TIME           "msb/traffic/GateCounter/System/time"
#define MQTT_DEBUG_LOG          "msb/traffic/GateCounter/System/debug"
#define MQTT_PUB_HEARTBEAT      "msb/traffic/GateCounter/System/heartbeat"

/* Season metadata (shared SD/season logic) */
#define MQTT_PUB_SEASON_FOLDER  "msb/traffic/GateCounter/System/seasonFolder"
#define MQTT_PUB_SEASON_YEAR    "msb/traffic/GateCounter/System/seasonYear"

// WiFi diagnostics (retained)
#define MQTT_PUB_WIFI_SSID   "msb/traffic/GateCounter/System/wifi/ssid"
#define MQTT_PUB_WIFI_RSSI   "msb/traffic/GateCounter/System/wifi/rssi"
#define MQTT_PUB_WIFI_IP     "msb/traffic/GateCounter/System/wifi/ip"

// ---------------- ENV ----------------
// retained JSON: {"tempF": xx.x, "humidity": xx.x}
#define MQTT_PUB_TEMP        "msb/traffic/GateCounter/Env/tempHumidity"

// ---------------- CARS ----------------
#define MQTT_PUB_EXIT_CARS        "msb/traffic/GateCounter/Cars/ExitTotal"
#define MQTT_PUB_INPARK_CARS      "msb/traffic/GateCounter/Cars/InParkCars"
#define MQTT_PUB_SHOWTOTAL        "msb/traffic/GateCounter/Cars/ShowTotal"
#define MQTT_PUB_CARS_HOURLY      "msb/traffic/GateCounter/Cars/hour/"
#define MQTT_PUB_HOURLY_JSON      "msb/traffic/GateCounter/Cars/hour/json"
#define MQTT_PUB_BETWEENCARS_MS   "msb/traffic/GateCounter/Cars/timeBetweenCars"

// ---------------- CALENDAR ----------------
#define MQTT_PUB_DAYOFMONTH  "msb/traffic/GateCounter/Calendar/DayOfMonth"
#define MQTT_PUB_DAYSRUNNING "msb/traffic/GateCounter/Calendar/DaysRunning"
#define MQTT_PUB_SUMMARY     "msb/traffic/GateCounter/Calendar/Summary"

// 2025 dual-beam Sensors and Logs (A upstream, B downstream)
#define MQTT_PUB_BEAM_A_STATE     "msb/traffic/GateCounter/Sensors/beamAState"
#define MQTT_PUB_BEAM_B_STATE     "msb/traffic/GateCounter/Sensors/beamBState"
#define MQTT_PUB_BEAM_AB_MS       "msb/traffic/GateCounter/Sensors/beamAB_ms"
#define MQTT_PUB_BEAM_B_BROKEN_MS "msb/traffic/GateCounter/Sensors/beamB_broken_ms"
#define MQTT_PUB_TTP              "msb/traffic/GateCounter/Sensors/TTP"
#define MQTT_COUNTER_LOG          "msb/traffic/GateCounter/Sensors/CounterLog"
#define MQTT_PUB_ALARM            "msb/traffic/GateCounter/Sensors/Alarm"

// ---------------- CONFIG (subscribed setpoints/toggles) ----------------
#define MQTT_SUB_GATE_RESET_DAILY   "msb/traffic/GateCounter/Config/resetDailyCount"
#define MQTT_SUB_GATE_RESET_SHOW    "msb/traffic/GateCounter/Config/resetShowCount"
#define MQTT_SUB_GATE_RESET_DOM     "msb/traffic/GateCounter/Config/resetDayOfMonth"
#define MQTT_SUB_GATE_RESET_DAYS    "msb/traffic/GateCounter/Config/resetDaysRunning"
#define MQTT_SUB_GATE_TIMEOUT       "msb/traffic/GateCounter/Config/gateCounterTimeout"
#define MQTT_SUB_CARMS              "msb/traffic/GateCounter/Config/carDetectMS"
#define MQTT_SUB_LOGGING            "msb/traffic/GateCounter/Config/loggingEnabled"
#define MQTT_SUB_CC_DAY_OF_MONTH  "msb/traffic/CarCounter/Calendar/dayOfMonth"
#define MQTT_SUB_CC_DAYS_RUNNING  "msb/traffic/CarCounter/Calendar/daysRunning"


// ---------------- CAR COUNTER INPUTS (new tree) ----------------
#define MQTT_SUB_CC_ENTER_TOTAL     "msb/traffic/CarCounter/Cars/EnterTotal"
#define MQTT_SUB_CC_SHOW_TOTAL      "msb/traffic/CarCounter/Cars/ShowTotal"
// Car Counter show calendar (authoritative)
#define MQTT_SUB_CC_SHOW_START_DATE "msb/traffic/CarCounter/Config/showStartDate"

/***** 2025 CarCounter-parity debounced state machine for GateCounter *****/

// Debounce raw beams (defensive, same as CarCounter)
const unsigned long debounceDelay = 50;  // ms; adjust 30–80 if needed
bool stableRawA = HIGH;                 // stable debounced raw for Beam A
bool stableRawB = HIGH;                 // stable debounced raw for Beam B
unsigned long lastRawAChangeMs = 0;
unsigned long lastRawBChangeMs = 0;

// CarCounter-style states
enum GateDetectState {
    WAITING_FOR_CAR,
    BEAM_A_HIGH,
    BOTH_BEAMS_HIGH,
    CAR_DETECTED
};
GateDetectState gateDetectState = WAITING_FOR_CAR;

// Timing / flags
unsigned long beamATripTime_ms = 0;         // when Beam A first broke
unsigned long bothBeamsBroken_ms = 0;       // when both beams confirmed broken
unsigned long lastCarDetected_ms = 0;       // when last car was detected
unsigned long firstBeamHealth_ms  = 0;      // for Beam A
unsigned long secondBeamHealth_ms = 0;      // for Beam B
unsigned long timeBetweenCars_ms = 0;       // time between cars for ExitLog.csv
bool carPresentFlag = false;
static bool gateStuckAlarmActive = false;
unsigned long abFollow_ms = 0;              // A to B follow time
unsigned long timeToPassMS = 0;             // Time from start of detection to confirmation
// Busy nights + platooning can make A->B slow, and we can't throw away real cars.
// Log once per event so we can quantify how often this happens.
static bool abTimeoutLogged = false;
// Filters / windows (match your proven behavior)
const unsigned long minActivationDuration = 150; // ignore tiny blips
const unsigned long maxABFollow_ms = 900;    

// GAL 25-11-27: Minimum time (ms) both beams must be broken
// to qualify as a real vehicle instead of noise.
unsigned long carDetectMS = 1000;           // Minimum wait duration for a vehicle to be confirmed

// GAL 25-11-27: Max time (ms) a car can block the exit path
// before we publish a "Vehicle stuck" alarm.
// Updated live from HA via msb/traffic/GateCounter/Config/gateCounterTimeout.
unsigned long gateCounterTimeout = 60000; // default time for car counter alarm in millis


/***** OTA & WEBSERVER SETUP *****/
AsyncWebServer server(80);     // Define Webserver
String currentDirectory = "/"; // Current working directory

unsigned long ota_progress_millis = 0;

//void saveHourlyCounts();  // forward declaration

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  //saveHourlyCounts();
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
    // Log when OTA has finished
    if (success) {
      Serial.println("OTA update finished successfully!");
    } else {
      Serial.println("There was an error during OTA update!");
    }
    // <Add your own code here>
}

/** REAL TIME Clock & Time Related Variables **/
RTC_DS3231 rtc;
const char* ampm ="AM";
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -21600;
const int   daylightOffset_sec = 3600;

// GAL 25-11-22: DS3231 timestamp helper (use for GateCounter HELLO/debug)
String getRtcTimestamp() {
    if (!rtcReady) return "rtc-not-ready";  // GAL 25-11-22

    DateTime now = rtc.now();

    char buf[32];
    snprintf(buf, sizeof(buf),
             "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());

    return String(buf);
}

// Initialize DHT sensor & Variables for temperature and humidity readTempandRH()
DHT dht(DHTPIN, DHTTYPE);
float tempF = -998.0;  // Temperature
float humidity = -998.0;     // Humidity

/** Display Definitions & variables **/
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
/*Line Numbers used for Display*/
const int line1 =0;
const int line2 =9;
const int line3 = 19;
const int line4 = 30;
const int line5 = 42;
const int line6 = 50;
const int line7 = 53;

//Create Multiple WIFI Object
WiFiMulti wifiMulti;
//WiFiClientSecure espGateCounter;
WiFiClient espGateCounter;

//const uint32_t connectTimeoutMs = 10000;
uint16_t connectTimeOutPerAP=5000;

/***** MQTT Setup Variables  *****/
PubSubClient mqtt_client(espGateCounter);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (500)
char msg[MSG_BUFFER_SIZE];
//char mqtt_server[] = mqtt_Server;
//char mqtt_username[] = mqtt_UserName;
//char mqtt_password[] = mqtt_Password;
//const int mqtt_port = mqtt_Port;
bool loggingEnabled = false; // Default: Logging is OFF
bool mqtt_connected = false;
bool wifi_connected = false;
int wifi_connect_attempts = 5;
bool hasRun = false;

/***** GLOBAL VARIABLES *****/
unsigned int dayOfMonth;      // Current Calendar day
unsigned int lastDayOfMonth;  // Last calendar day used to reset days running
unsigned int currentHr12;     // Current Hour 12 Hour format
unsigned int currentHr24;     // Current Hour 24 Hour Format
unsigned int currentMin;      // Current Minute
unsigned int currentSec;      // Current Second
unsigned int daysRunning;     // Number of days the show is running.
unsigned int currentTimeMinute; // for converting clock time hh:mm to clock time min since midnight
int totalDailyCars; // total cars counted per day 24/7 Needed for debugging
int totalShowCars;  // total cars counted for durning show hours open (5:00 pm to 9:10 pm)
int inParkCars;     // cars in park Enter Cars - Exit Cars
int carCounterCars; // Counts from Car Counter
int lastcarCounterCars; // Used to publish in park cars when car counter increases
int beamAState = 0;
int lastBeamAState = -1;

int beamBState = 0;
int lastBeamBState = -1;
unsigned long triggerTime; // Stores the time when sensor 1 is triggered
unsigned long beamSensorAlarm; // Monitor time Beam Sensor is blocked


/***** TIME VARIABLES *****/
const unsigned long wifi_connectioncheckMillis = 5000; // check for connection every 5 sec
const unsigned long mqtt_connectionCheckMillis = 30000; // check for connection
unsigned long start_MqttMillis; // for Keep Alive Timer
unsigned long start_WiFiMillis; // for keep Alive Timer

char buf2[25] = "YYYY-MM-DD hh:mm:ss"; // time car detected

//***** DAILY RESET FLAGS *****
bool flagDaysRunningReset = false;
bool flagMidnightReset = false;
bool flagDailyShowStartReset = false;
bool flagDailySummarySaved = false;
bool flagDailyShowSummarySaved = false;
bool flagHourlyReset = false;
bool showTime = false;
bool resetFlagsOnce = false;

// ********** SEASONAL FILE NAMES FOR SD CARD *********

File myFile;   // global handle for SD file ops

// Computed on boot after RTC + SD init
String seasonFolder = "";   // Example: "/GC/2025"

// Build full seasonal file path
String sPath(const char *fname) {
    return seasonFolder + "/" + String(fname);
}

// Runtime-assigned filenames (filled in by initSeasonalPaths())
String fileName1;   // ExitTotal.txt       (daily total)
String fileName2;   // ShowTotal.txt       (season total)
String fileName3;   // DayOfMonth.txt
String fileName4;   // RunDays.txt
String fileName5;   // GateHourlyData.csv
String fileName6;   // GateLog.csv
String fileName7;   // GateDailySummary.csv
String fileName8;   // data/index.html     (OTA / UI)
String fileName9;   // data/style.css
String fileName10;  // SensorLog.csv
String fileNameShowStart; // ShowStart.txt   (opening date)

/***** Arrays for Hourly Totals/Averages *****/
static unsigned int hourlyCount[24] = {0}; // Array for Daily total cars per hour
static float hourlyTemp[24] = {0.0};   // Array to store average temperatures for 24 hours

/***** Arrays Used to make display Pretty *****/
static char days[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
static char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// sync Time at REBOOT
void SetLocalTime()  {
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time. Using Compiled Date");
    return;
  }
  //Following used for Debugging and can be commented out
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  Serial.print("Day of week: ");
  Serial.println(&timeinfo, "%A");
  Serial.print("Month: ");
  Serial.println(&timeinfo, "%B");
  Serial.print("Day of Month: ");
  Serial.println(&timeinfo, "%d");
  Serial.print("Year: ");
  Serial.println(&timeinfo, "%Y");
  Serial.print("Hour: ");
  Serial.println(&timeinfo, "%H");
  Serial.print("Hour (12 hour format): ");
  Serial.println(&timeinfo, "%I");
  Serial.print("Minute: ");
  Serial.println(&timeinfo, "%M");
  Serial.print("Second: ");
  Serial.println(&timeinfo, "%S");
  Serial.println("Time variables");
  char timeHour[3];
  strftime(timeHour,3, "%H", &timeinfo);
  Serial.println(timeHour);
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  Serial.println(timeWeekDay);
  Serial.println();

  // Convert NTP time string to set RTC
  char timeStringBuff[50]; //50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.println(timeStringBuff);
  rtc.adjust(DateTime(timeStringBuff));
}

//WIFI Setup
void setup_wifi()  {
    Serial.println("Connecting to WiFi");
    display.println("Connecting to WiFi..");
    display.display();
    while(wifiMulti.run(connectTimeOutPerAP) != WL_CONNECTED) {
        Serial.print(".");
    }
    Serial.println("Connected to the WiFi network");
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.display();
  
    display.setCursor(0, line1);
    display.print("SSID: ");
    display.println(WiFi.SSID());   // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    
    IPAddress ip = WiFi.localIP();  // print your board's IP address:
    Serial.print("IP: ");
    Serial.println(ip);
    display.setCursor(0, line2);
    display.print("IP: ");
    display.println(ip);
    
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
    display.setCursor(0, line3);
    display.print("signal: ");
    display.print(rssi);  // print the received signal strength:
    display.println(" dBm");
    display.display();
 
    delay(1000);
}  // END WiFi Setup

// =====================================================
// GAL 25-11-24: Seasonal SD folder structure (/GC/YYYY/)
// Matches Car Counter season logic.
// SeasonYear: Nov-Dec => current year, Jan-Oct => previous year.
// =====================================================
int determineSeasonYear(const DateTime &now) {
    int y = now.year();
    int m = now.month();
    if (m >= 11) return y;     // Nov/Dec = current year season
    return y - 1;             // Jan–Oct belongs to previous season
}

void ensureSeasonFolderExists(int seasonYear) {
    String base = "/GC";
    String seasonPath = base + "/" + String(seasonYear);

    if (!SD.exists(base)) {
        SD.mkdir(base);
    }
    if (!SD.exists(seasonPath)) {
        SD.mkdir(seasonPath);
        Serial.printf("Created season folder: %s\n", seasonPath.c_str());
    }
}

// Forward Declarations 
void callback(char* topic, byte* payload, unsigned int length);
int computeDaysRunningFromStart();

// MQTT forward declarations for seasonal folder init
void publishMQTT(const char *topic, const String &message, bool retainFlag);
void publishMQTT(const char *topic, const String &message);
void publishDebugEvent(const char* event, const String& details, bool retainFlag);


void initSeasonalPaths() {
    DateTime now = rtc.now();
    int seasonYear = determineSeasonYear(now);

    ensureSeasonFolderExists(seasonYear);

    seasonFolder = "/GC/" + String(seasonYear);

    // Assign full seasonal paths
    fileName1 = sPath("DAILYTOT.txt");
    fileName2 = sPath("SHOWTOT.txt");
    fileName3 = sPath("DAYOFMONTH.txt");
    fileName4 = sPath("DAYSRUNNING.txt");
    fileName5 = sPath("GateHourlyData.csv");
    fileName6 = sPath("ExitLog.csv");
    fileName7 = sPath("ShowSummary.csv");
    fileNameShowStart = sPath("ShowStart.txt");

    Serial.printf("Season folder set to: %s\n", seasonFolder.c_str());
        // ---- MQTT visibility for headless debugging ----
    publishMQTT(MQTT_PUB_SEASON_YEAR,   String(seasonYear), true);
    publishMQTT(MQTT_PUB_SEASON_FOLDER, seasonFolder,      true);

    publishDebugEvent(
        "season_init",
        "seasonYear=" + String(seasonYear) +
        " folder=" + seasonFolder,
        true
    );

}
// ==========================================
// SD Web File Manager Helper Functions
// ==========================================

// Join directory + filename safely
String buildPath(const String &baseDir, const String &fileName) {
    if (baseDir.endsWith("/")) {
        return baseDir + fileName;
    }
    return baseDir + "/" + fileName;
}


// BEGIN OTA SD Card File Operations
void listSDFiles(AsyncWebServerRequest *request) {

    // Header line so the UI output is self-explanatory
    String fileList = "Files in " + currentDirectory + ":\n";
    fileList += "Name\tSize (bytes)\tLast Write\n";

    File root = SD.open(currentDirectory);
    if (!root || !root.isDirectory()) {
        request->send(500, "text/plain", "Failed to open directory: " + currentDirectory);
        return;
    }

    while (true) {
        File file = root.openNextFile();
        if (!file) {
            break;  // no more files
        }

        if (!file.isDirectory()) {
            // Name
            fileList += String(file.name());
            fileList += "\t";

            // Size
            fileList += String(file.size());
            fileList += "\t";

            // Last write time (if available and time is set)
            time_t lw = file.getLastWrite();  // ESP32 SD library usually supports this
            if (lw > 0) {
                struct tm *tmstruct = localtime(&lw);
                char buf[20];
                // YYYY-MM-DD HH:MM
                strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M", tmstruct);
                fileList += buf;
            } else {
                fileList += "unknown";
            }

            fileList += "\n";
        }

        file.close();  // important: close each file before opening the next
    }

    root.close();

    request->send(200, "text/plain", fileList);
}



void downloadSDFile(AsyncWebServerRequest *request) {
    if (!request->hasParam("filename")) {
        request->send(400, "text/plain", "Filename is required");
        return;
    }

    // NEW (correct, uses directory + "/" + filename):
    String filename = buildPath(currentDirectory, request->getParam("filename")->value());

    if (!SD.exists(filename)) {
        request->send(404, "text/plain", "File not found");
        return;
    }

    // Add Content-Disposition header for proper filename handling
    AsyncWebServerResponse *response = request->beginResponse(SD, filename, "application/octet-stream");
    response->addHeader("Content-Disposition", "attachment; filename=\"" + String(request->getParam("filename")->value()) + "\"");

    request->send(response);
}

void uploadSDFile(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
    static File uploadFile; // Keep track of the currently uploading file

    // Use buildPath instead of manual concat
    String fullPath = buildPath(currentDirectory, filename);  // <--- THIS LINE ONLY CHANGE

    // Handle the start of the upload
    if (index == 0) {
        Serial.printf("Upload started: %s\n", fullPath.c_str());
        if (SD.exists(fullPath)) {
            SD.remove(fullPath); // Remove the file if it already exists
        }
        uploadFile = SD.open(fullPath, FILE_WRITE);
        if (!uploadFile) {
            Serial.printf("Failed to open file: %s\n", fullPath.c_str());
            request->send(500, "text/plain", "Failed to open file for writing");
            return;
        }
    }

    // Write data to the file
    if (uploadFile) {
        uploadFile.write(data, len);
    }

    // Handle the end of the upload
    if (final) {
        if (uploadFile) {
            uploadFile.close();
            Serial.printf("Upload completed: %s\n", fullPath.c_str());
            request->send(200, "text/plain", "File uploaded successfully to " + currentDirectory);
        } else {
            Serial.printf("Upload failed: %s\n", fullPath.c_str());
            request->send(500, "text/plain", "Failed to write file");
        }
    }
}
void changeDirectory(AsyncWebServerRequest *request) {
    if (!request->hasParam("dir")) {
        request->send(400, "text/plain", "Directory name is required");
        return;
    }

    String dir = request->getParam("dir")->value();
    String target;

    // Case 1: absolute path (starts with "/")
    if (dir.startsWith("/")) {
        target = dir;
    }
    // Case 2: contains a "/" but no leading slash → treat as root-based
    // e.g. "gc/2025" -> "/gc/2025"
    else if (dir.indexOf('/') != -1) {
        target = "/" + dir;
    }
    // Case 3: simple name (no "/") → treat as relative to currentDirectory
    else {
        target = buildPath(currentDirectory, dir);
    }

    // Normalize accidental leading "//"
    if (target.startsWith("//")) {
        target = target.substring(1);
    }

    // Must exist AND be a directory
    if (SD.exists(target)) {
        File f = SD.open(target);
        if (f && f.isDirectory()) {
            f.close();
            currentDirectory = target;
            request->send(200, "text/plain", "Changed directory to " + currentDirectory);
            return;
        }
        if (f) {
            f.close();
        }
    }

    request->send(404, "text/plain", "Directory not found: " + target);
}

void deleteSDFile(AsyncWebServerRequest *request) {
    if (!request->hasParam("filename")) {
        request->send(400, "text/plain", "Filename is required");
        return;
    }

    String fileName = request->getParam("filename")->value();

    // Build path safely in the current directory
    String fullPath = buildPath(currentDirectory, fileName);

    // Normalize the file path
    if (fullPath.startsWith("//")) {
        fullPath = fullPath.substring(1); // Remove redundant leading slashes
    }

    if (SD.exists(fullPath)) {
        if (SD.remove(fullPath)) {
            Serial.printf("File deleted: %s\n", fullPath.c_str());
            request->send(200, "text/plain", "File deleted successfully: " + fullPath);
        } else {
            Serial.printf("Failed to delete file: %s\n", fullPath.c_str());
            request->send(500, "text/plain", "Failed to delete file: " + fullPath);
        }
    } else {
        Serial.printf("File not found: %s\n", fullPath.c_str());
        request->send(404, "text/plain", "File not found: " + fullPath);
    }
}

void renameSDFile(AsyncWebServerRequest *request) {

    if (!request->hasParam("old") || !request->hasParam("new")) {
        request->send(400, "text/plain", "Parameters 'old' and 'new' are required");
        return;
    }

    String oldName = request->getParam("old")->value();
    String newName = request->getParam("new")->value();

    // Build full paths relative to currentDirectory
    String oldPath = buildPath(currentDirectory, oldName);
    String newPath = buildPath(currentDirectory, newName);

    // Normalize accidental leading //
    if (oldPath.startsWith("//"))  oldPath = oldPath.substring(1);
    if (newPath.startsWith("//")) newPath = newPath.substring(1);

    // Make sure source exists
    if (!SD.exists(oldPath)) {
        request->send(404, "text/plain", "Source file not found: " + oldPath);
        return;
    }

    // Be conservative: refuse to overwrite an existing target
    if (SD.exists(newPath)) {
        request->send(409, "text/plain", "Target already exists: " + newPath);
        return;
    }

    if (SD.rename(oldPath, newPath)) {
        Serial.printf("File renamed: %s -> %s\n", oldPath.c_str(), newPath.c_str());
        request->send(200, "text/plain",
                      "File renamed: " + oldPath + " -> " + newPath);
    } else {
        Serial.printf("Failed to rename: %s -> %s\n", oldPath.c_str(), newPath.c_str());
        request->send(500, "text/plain",
                      "Failed to rename: " + oldPath + " -> " + newPath);
    }
}
//END OTA SD Card File Operations

// Gate Counter HTML Content now served from /data/index.html and /data/style.css
void setupServer() {
    // Serve HTML file
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!SD.exists("/data/index.html")) {
            request->send(500, "text/plain", "index.html not found in /data");
            return;
        }
        request->send(SD, "/data/index.html", "text/html");
    });

    // ----------------------------
    // Enable Reboot via HTTP
    // ----------------------------

    server.on("/reboot", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "Gate Counter rebooting...");
        Serial.println("HTTP /reboot requested – restarting ESP32");
        delay(500);
        ESP.restart();
    });    

    // ----------------------------
    // Serve CSS (fallback = 404)
    // ----------------------------
    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!SD.exists("/data/style.css")) {
            request->send(500, "text/plain", "style.css not found in /data");
            return;
        }
        request->send(SD, "/data/style.css", "text/css");
    });

    // ----------------------------
    // Serve Show Summary webpage
    // Put showSummary.html in /data/
    // ----------------------------
    server.on("/showSummary.html", HTTP_GET, [](AsyncWebServerRequest *request) {

        if (SD.exists("/data/showSummary.html")) {
            request->send(SD, "/data/showSummary.html", "text/html");
            return;
        }

        request->send(404, "text/plain", "showSummary.html not found in /data");
    });

    // ----------------------------
    // Seasonal ShowSummary.csv
    // /GC/YYYY/ShowSummary.csv  (seasonFolder should already be /GC/YYYY)
    // ----------------------------
    server.on("/ShowSummary.csv", HTTP_GET, [](AsyncWebServerRequest *request) {

        String fullPath = String(seasonFolder) + "/ShowSummary.csv";

        if (!SD.exists(fullPath)) {
            request->send(404, "text/plain",
                          "ShowSummary.csv not found in season folder");
            return;
        }

        AsyncWebServerResponse *response =
            request->beginResponse(SD, fullPath, "text/csv");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    });

    // ----------------------------
    // File manager / SD routes
    // ----------------------------
    server.on("/listFiles", HTTP_GET, listSDFiles);
    server.on("/download", HTTP_GET, downloadSDFile);

    // Simple upload page for /uploadToData (so browser GET doesn't 500)
    server.on("/uploadToData", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(200, "text/html",
            "<!doctype html><html><body>"
            "<h3>Upload UI files to /data</h3>"
            "<form method='POST' action='/uploadToData' enctype='multipart/form-data'>"
            "<input type='file' name='file' multiple>"
            "<input type='submit' value='Upload to /data'>"
            "</form>"
            "<p>After upload, go back to <a href='/'>home</a>.</p>"
            "</body></html>"
        );
    });

    // Handle file uploads to currentDirectory (CarCounter-aligned)
    server.on("/upload", HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        uploadSDFile);

    // ----------------------------
    // Handle file uploads to /data directory (CarCounter-aligned, no sdAvailable)
    // ----------------------------
    server.on("/uploadToData", HTTP_POST,
        [](AsyncWebServerRequest *request) {},
        [](AsyncWebServerRequest *request,
        String filename,
        size_t index,
        uint8_t *data,
        size_t len,
        bool final) {

            // Guarantee /data exists (same behavior as Car Counter)
            if (!SD.exists("/data")) {
                SD.mkdir("/data");
            }

            String fullPath = "/data/" + filename;
            static File uploadFile;

            if (index == 0) {  // First chunk
                if (SD.exists(fullPath)) {
                    SD.remove(fullPath);
                }

                uploadFile = SD.open(fullPath, FILE_WRITE);
                if (!uploadFile) {
                    request->send(500, "text/plain", "Failed to open file for writing");
                    return;
                }
            }

            if (uploadFile) {
                uploadFile.write(data, len);
            }

            if (final) {  // Final chunk
                if (uploadFile) {
                    uploadFile.close();
                }
                request->send(200, "text/plain",
                            "File uploaded successfully to /data");
            }
        });

    // ----------------------------
    // Identity endpoint (Changing Directory)
    // ----------------------------
    server.on("/changeDirectory", HTTP_GET, changeDirectory);

    // ----------------------------
    // NEW: delete and rename 25-12-01 GAL
    // ----------------------------
    server.on("/delete", HTTP_ANY, deleteSDFile);
    server.on("/rename", HTTP_ANY, renameSDFile);

    // ----------------------------
    // Identity endpoint (for UI theming) 25-12-01 GAL
    // ----------------------------
    server.on("/identity", HTTP_GET, [](AsyncWebServerRequest *request) {
        // THIS_MQTT_CLIENT is already "CarCounter" or "GateCounter"
        request->send(200, "text/plain", THIS_MQTT_CLIENT);
    });

    // Elegant OTA
    // You can also enable authentication by uncommenting the below line.
    // ElegantOTA.setAuth("admin", "password");
    ElegantOTA.setID(THIS_MQTT_CLIENT);  // Set Hardware ID
    ElegantOTA.setFWVersion(FWVersion);   // Set Firmware Version
    ElegantOTA.setTitle(OTA_Title);  // Set OTA Webpage Title
    //ElegantOTA.setFilesystemMode(true);  // added 10.16.24.4
    // Start ElegantOTA
    ElegantOTA.begin(&server);    // Start ElegantOTA
    // ElegantOTA callbacks
    ElegantOTA.onStart(onOTAStart);
    ElegantOTA.onProgress(onOTAProgress);
    ElegantOTA.onEnd(onOTAEnd);
    server.begin();
    Serial.println("HTTP server started");
}

/***** MQTT SECTION for Gate Counter******/
// Queue for offline MQTT publishes
std::queue<String> publishQueue;

// GAL 25-11-22: Match Car Counter retain-aware queue behavior
const size_t MAX_QUEUE = 3000;
const size_t MAX_FLUSH_PER_CALL = 150;   // prevents WDT resets



// Helper to encode retain flag into queue "topic|message|retain"
String encodeQueuedMessage(const char *topic, const String &msg, bool retainFlag) {
    return String(topic) + "|" + msg + "|" + (retainFlag ? "1" : "0");
}

void saveShowStartDate() {
    File f = SD.open(fileNameShowStart, FILE_WRITE);
    if (f) {
        f.printf("%04d-%02d-%02d\n", showStartYear, showStartMonth, showStartDay);
        f.close();
    }
}

void getShowStartDate() {
    File f = SD.open(fileNameShowStart, FILE_READ);
    if (!f) {
        Serial.println("No saved show start date.");
        return;
    }

    char buf[20];
    int y, m, d;

    if (f.readBytesUntil('\n', buf, sizeof(buf)) > 0) {
        if (sscanf(buf, "%d-%d-%d", &y, &m, &d) == 3) {
            showStartYear = y;
            showStartMonth = m;
            showStartDay = d;
            showStartDateValid = true;
        }
    }
    f.close();
}


// Retain-aware publish (NEW overload)
void publishMQTT(const char *topic, const String &message, bool retainFlag) {
    if (mqtt_client.connected()) {
        mqtt_client.publish(topic, message.c_str(), retainFlag);
    } else {
        if (publishQueue.size() >= MAX_QUEUE) {
            publishQueue.pop();  // drop oldest
        }
        Serial.printf("MQTT not connected. QUEUEING: %s -> %s (retain=%d)\n",
                      topic, message.c_str(), retainFlag);

        publishQueue.push(encodeQueuedMessage(topic, message, retainFlag));
    }
    start_MqttMillis = millis();
}

// Backwards-compatible wrapper = NO retain (all existing calls still use this)
void publishMQTT(const char *topic, const String &message) {
    publishMQTT(topic, message, false);
}

// Retain-aware queue flush (bounded)
void publishQueuedMessages(size_t maxToFlush = MAX_FLUSH_PER_CALL) {
    size_t flushed = 0;

    while (!publishQueue.empty() &&
           mqtt_client.connected() &&
           flushed < maxToFlush) {

        String data = publishQueue.front();
        publishQueue.pop();

        int p1 = data.indexOf('|');
        int p2 = data.indexOf('|', p1 + 1);

        if (p1 != -1) {
            String topic   = data.substring(0, p1);
            String message = data.substring(p1 + 1, p2);
            bool retainFlag = (p2 != -1 && data.substring(p2 + 1) == "1");

            mqtt_client.publish(topic.c_str(), message.c_str(), retainFlag);
        }

        flushed++;
    }

    if (flushed > 0) {
        Serial.printf("MQTT Queue Flush: %u messages flushed, %u remain\n",
                      (unsigned)flushed, (unsigned)publishQueue.size());
    }
}

void publishDebugLog(const String &message) {
    publishMQTT(MQTT_DEBUG_LOG, message);   // never retained
}

// =====================================================
// GAL 25-11-22: MQTT Debug Event Publisher (remote console)
// Uses MQTT_DEBUG_LOG topic you already have
// =====================================================
void publishDebugEvent(const char* event, const String& details, bool retainFlag = false) {
    char buf[256];

    snprintf(buf, sizeof(buf),
        "{"
            "\"device\":\"%s\","
            "\"event\":\"%s\","
            "\"fw\":\"%s\","
            "\"time\":\"%s\","
            "\"details\":\"%s\""
        "}",
        THIS_MQTT_CLIENT,
        event,
        FWVersion,                     // <-- NO .c_str()
        bootTimestamp.c_str(),     // <-- use cached boot time
        details.c_str()
    );

    publishMQTT(MQTT_DEBUG_LOG, String(buf), retainFlag);
}

// Used to publish current counts & heartbeat every 30s during quiet periods
void KeepMqttAlive() {

    // ---- Heartbeat (retained, ONLY here) ----
    publishMQTT(MQTT_PUB_FIRMWARE, String(FWVersion), true);

    publishMQTT(
        MQTT_PUB_HEARTBEAT,
        String("{\"boot\":\"") + bootTimestamp +
        "\",\"now\":\"" + getRtcTimestamp() +
        "\",\"exit\":" + totalDailyCars +
        ",\"inpark\":" + inParkCars +
        ",\"rssi\":" + WiFi.RSSI() +
        "}",
        true
    );

    // ---- Temp/RH JSON (retained) ----
    char jsonPayload[100];
    snprintf(jsonPayload, sizeof(jsonPayload),
             "{\"tempF\": %.1f, \"humidity\": %.1f}", tempF, humidity);
    publishMQTT(MQTT_PUB_TEMP, String(jsonPayload), true);

    // ---- Retained core counts ----
    publishMQTT(MQTT_PUB_EXIT_CARS,   String(totalDailyCars), true);
    publishMQTT(MQTT_PUB_INPARK_CARS, String(inParkCars),     true);
    publishMQTT(MQTT_PUB_SHOWTOTAL,   String(totalShowCars),  true);  // if you want it retained too

    // ---- WiFi diagnostics (retained, NEW TREE topics) ----
    publishMQTT(MQTT_PUB_WIFI_RSSI, String(WiFi.RSSI()), true);
    publishMQTT(MQTT_PUB_WIFI_SSID, WiFi.SSID(),        true);
    publishMQTT(MQTT_PUB_WIFI_IP,   WiFi.localIP().toString(), true);

    // DO NOT touch start_MqttMillis here anymore
}

/*** HELLO / STATUS JSON HELPER ****/
// GAL 25-11-26: Standardize HELLO as JSON status/event channel
static inline void publishHello(const char* status, const char* msg, bool retainFlag) {
  char buf[256];

  snprintf(buf, sizeof(buf),
      "{"
        "\"device\":\"%s\","
        "\"status\":\"%s\","
        "\"fw\":\"%s\","
        "\"boot\":\"%s\","
        "\"msg\":\"%s\""
      "}",
      THIS_MQTT_CLIENT,
      status,
      FWVersion,
      bootTimestamp.c_str(),
      msg
  );

  publishMQTT(MQTT_PUB_HELLO, String(buf), retainFlag);
}


//GATE COUNTER Connection to MQTT Server
void MQTTreconnect() {
    static unsigned long lastReconnectAttempt = 0; // Tracks the last reconnect attempt time
    const unsigned long reconnectInterval = 5000; // Time between reconnect attempts (5 seconds)

    // If the client is already connected, do nothing
    if (mqtt_client.connected()) {
        return;
    }

    // Check if enough time has passed since the last attempt
    if (millis() - lastReconnectAttempt > reconnectInterval) {
        lastReconnectAttempt = millis(); // Update the last attempt time
        Serial.println("Attempting MQTT connection...");

        for (int i = 0; i < mqtt_servers_count; i++) {
            // Set the server for the current configuration
            mqtt_client.setServer(mqtt_configs[i].server, mqtt_configs[i].port);
            mqtt_client.setCallback(callback);  // required to receive messages

            // Create a unique client ID
            String clientId = THIS_MQTT_CLIENT;

            // Attempt to connect using the current server's credentials
            Serial.printf("Trying MQTT server: %s:%d\n", mqtt_configs[i].server, mqtt_configs[i].port);
            if (mqtt_client.connect(clientId.c_str(), mqtt_configs[i].username, mqtt_configs[i].password)) {
                Serial.printf("Connected to MQTT server: %s\n", mqtt_configs[i].server);
                
                // Display connection status
                display.setTextSize(1);
                display.setTextColor(WHITE);
                display.setCursor(0,line5);
                display.println("MQTT Connect");
                display.display();
                Serial.println("connected!");
                Serial.println("Waiting for Car");

                // Once connected, publish an announcement (retained)
                publishHello("online", "Gate Counter ONLINE", true);
                publishMQTT(MQTT_PUB_FIRMWARE, FWVersion, true);
                publishMQTT(MQTT_PUB_SEASON_YEAR,   String(determineSeasonYear(rtc.now())), true);
                publishMQTT(MQTT_PUB_SEASON_FOLDER, seasonFolder, true);


                // publish temp/RH JSON on connect (retained)
                char jsonPayload[100];
                snprintf(jsonPayload, sizeof(jsonPayload),
                        "{\"tempF\": %.1f, \"humidity\": %.1f}", tempF, humidity);
                publishMQTT(MQTT_PUB_TEMP, String(jsonPayload), true);

                // retained core counts
                publishMQTT(MQTT_PUB_EXIT_CARS,   String(totalDailyCars), true);
                publishMQTT(MQTT_PUB_INPARK_CARS, String(inParkCars),     true);

                // ===== WiFi telemetry (retained) =====
                publishMQTT(MQTT_PUB_WIFI_SSID, WiFi.SSID(), true);
                publishMQTT(MQTT_PUB_WIFI_RSSI, String(WiFi.RSSI()), true);
                publishMQTT(MQTT_PUB_WIFI_IP,   WiFi.localIP().toString(), true);

                // GAL 25-11-22: retained online snapshot for remote debugging
                publishDebugEvent(
                    "online",
                    "ssid=" + WiFi.SSID() +
                    " rssi=" + String(WiFi.RSSI()) +
                    " ip=" + WiFi.localIP().toString(),
                    true
                );

                // Subscribe to necessary topics (NEW TREE)
                mqtt_client.subscribe(MQTT_SUB_CC_ENTER_TOTAL);
                mqtt_client.subscribe(MQTT_SUB_CC_SHOW_TOTAL);
                mqtt_client.subscribe(MQTT_SUB_GATE_RESET_DAILY);
                mqtt_client.subscribe(MQTT_SUB_GATE_RESET_SHOW);
                mqtt_client.subscribe(MQTT_SUB_GATE_RESET_DOM);
                mqtt_client.subscribe(MQTT_SUB_GATE_RESET_DAYS);
                mqtt_client.subscribe(MQTT_SUB_GATE_TIMEOUT);
                mqtt_client.subscribe(MQTT_SUB_CARMS);
                mqtt_client.subscribe(MQTT_SUB_LOGGING);
                mqtt_client.subscribe(MQTT_SUB_CC_SHOW_START_DATE);
                mqtt_client.subscribe(MQTT_SUB_CC_DAY_OF_MONTH);
                mqtt_client.subscribe(MQTT_SUB_CC_DAYS_RUNNING);


                // Log subscriptions
                Serial.println("Subscribed to MQTT topics.");
                publishMQTT(MQTT_DEBUG_LOG, "MQTT connected and topics subscribed.");

                return; // Exit loop on successful connection
            } else {
                // Log connection failure for the current server
                Serial.printf("Failed to connect to MQTT server: %s (rc=%d)\n", mqtt_configs[i].server, mqtt_client.state());
            }
        }

        // If all servers fail
        Serial.println("All MQTT server attempts failed. Will retry...");
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0, line6);
        display.println("MQTT Error");
        display.display();
    }
}
/***** END MQTT SECTION *****/
// =====================================================
// Compute DaysRunning from showStartDate
// Day 1 = opening night; legacy Xmas Eve gap preserved
// =====================================================
int computeDaysRunningFromStart() {
    if (!showStartDateValid) return daysRunning; // fallback

    DateTime now = rtc.now();
    DateTime start(showStartYear, showStartMonth, showStartDay, 0, 0, 0);

    TimeSpan diff = now - start;
    int computed = diff.days() + 1;   // opening night is Day 1

    // Legacy Christmas Eve gap:
    // if date is 12/24 or later, subtract 1 day
    if (now.month() == 12 && now.day() >= 24) {
        computed -= 1;
    }

    if (computed < 1) computed = 1;
    return computed;
}

void checkWiFiConnection() {

/* non-blocking WiFi and MQTT Connectivity Checks 
    First check if WiFi is connected */
    if (wifiMulti.run() == WL_CONNECTED) {
        /* If MQTT is not connected then Attempt MQTT Connection */
        if (!mqtt_client.connected()) {
            Serial.print("hour = ");
            Serial.println(currentHr12);
            Serial.println("Attempting MQTT Connection");
            MQTTreconnect();
            start_MqttMillis = millis();
        } else {
                //keep MQTT client connected when WiFi is connected
                mqtt_client.loop();
        }
    } else {
        // If WiFi if lost, then attemp non blocking WiFi Connection
        if ((millis() - start_WiFiMillis) > wifi_connectioncheckMillis) {
            setup_wifi();
            start_WiFiMillis = millis();
        }
    }    
}

// =========== GET SAVED SETUP FROM SD CARD ==========
// open DAILYTOT.txt to get initial dailyTotal value
void getDailyTotal() {
    myFile = SD.open(fileName1, FILE_READ);
    if (myFile) {
        while (myFile.available()) {
            totalDailyCars = myFile.parseInt(); // read total
            Serial.print(" Daily cars from file = ");
            Serial.println(totalDailyCars);
        }
        myFile.close();

        // Recalculate in-park value on reboot
        inParkCars = carCounterCars - totalDailyCars;

        // GAL 25-11-24: retained state publishes
        publishMQTT(MQTT_PUB_EXIT_CARS,   String(totalDailyCars), true);
        publishMQTT(MQTT_PUB_INPARK_CARS, String(inParkCars),     true);

    } else {
        Serial.print("SD Card: Cannot open the file: ");
        Serial.println(fileName1);
    }
}


/** Get season total cars since show opened */
void getShowTotal() {
    myFile = SD.open(fileName2, FILE_READ);
    if (myFile) {
        while (myFile.available()) {
            totalShowCars = myFile.parseInt(); // read total
            Serial.print(" Total Show cars from file = ");
            Serial.println(totalShowCars);
        }
        myFile.close();

        // GAL 25-11-24: retained state publish
        publishMQTT(MQTT_PUB_SHOWTOTAL, String(totalShowCars), true);

    } else {
        Serial.print(F("SD Card: Cannot open the file: "));
        Serial.println(fileName2);
    }
}


// get the last calendar day used for reset daily counts
void getDayOfMonth() {
    myFile = SD.open(fileName3, FILE_READ);
    if (myFile) {
        while (myFile.available()) {
            lastDayOfMonth = myFile.parseInt(); // read day Number
            Serial.print(" Calendar Day = ");
            Serial.println(lastDayOfMonth);
        }
        myFile.close();

        // GAL 25-11-24: retained state publish
        publishMQTT(MQTT_PUB_DAYOFMONTH, String(lastDayOfMonth), true);

    } else {
        Serial.print(F("SD Card: Cannot open the file: "));
        Serial.println(fileName3);
    }
}


// Days the show has been running
void getDaysRunning() {
    myFile = SD.open(fileName4, FILE_READ);
    if (myFile) {
        while (myFile.available()) {
            daysRunning = myFile.parseInt(); // read day Number
            Serial.print(" Days Running = ");
            Serial.println(daysRunning);
        }
        myFile.close();

        // GAL 25-11-24: retained state publish
        publishMQTT(MQTT_PUB_DAYSRUNNING, String(daysRunning), true);

    } else {
        Serial.print(F("SD Card: Cannot open the file: "));
        Serial.println(fileName4);
    }
}


/** Get hourly car counts on reboot */
void getHourlyData() {
    DateTime now = rtc.now();
    char dateBuffer[13];
    snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", now.year(), now.month(), now.day());

    // Open the file for reading
    File file = SD.open(fileName5, FILE_READ);
    if (!file) {
        Serial.println("Failed to open GateHourlyData.csv. Resetting hourly data.");
        publishMQTT(MQTT_DEBUG_LOG, "Failed to open GateHourlyData.csv. Resetting hourly data.");
        memset(hourlyCount, 0, sizeof(hourlyCount)); // Reset to zeros
        return;
    }

    bool rowFound = false;

    // Read the file line by line
    while (file.available()) {
        String line = file.readStringUntil('\n');
        if (line.startsWith(dateBuffer)) {
            // Parse the row for the current date
            rowFound = true;

            int parsedValues = sscanf(line.c_str(),
                                      "%*[^,],%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d",
                                      &hourlyCount[0], &hourlyCount[1], &hourlyCount[2], &hourlyCount[3],
                                      &hourlyCount[4], &hourlyCount[5], &hourlyCount[6], &hourlyCount[7],
                                      &hourlyCount[8], &hourlyCount[9], &hourlyCount[10], &hourlyCount[11],
                                      &hourlyCount[12], &hourlyCount[13], &hourlyCount[14], &hourlyCount[15],
                                      &hourlyCount[16], &hourlyCount[17], &hourlyCount[18], &hourlyCount[19],
                                      &hourlyCount[20], &hourlyCount[21], &hourlyCount[22], &hourlyCount[23]);

            if (parsedValues == 24) {
                Serial.println("Successfully loaded hourly data for today.");
                publishMQTT(MQTT_DEBUG_LOG, "Successfully loaded hourly data for today.");

                for (int i = 0; i < 24; i++) {

                    // ---- Serial debug (keep) ----
                    Serial.printf("Hour %02d: %d cars\n", i, hourlyCount[i]);

                    // ---- NEW: publish retained hourly value for dashboards ----
                    char hourlyTopic[120];
                    snprintf(hourlyTopic, sizeof(hourlyTopic),
                            "%s/%02d", MQTT_PUB_CARS_HOURLY, i);
                    publishMQTT(hourlyTopic, String(hourlyCount[i]), true);

                    // ---- OPTIONAL: quiet MQTT debug to avoid 24-line spam ----
                    // Remove or comment out the noisy per-hour debug:
                    //
                    // char debugMsg[50];
                    // snprintf(debugMsg, sizeof(debugMsg), "Hour %02d: %d cars", i, hourlyCount[i]);
                    // publishMQTT(MQTT_DEBUG_LOG, String(debugMsg));
                }
            } else {
                Serial.println("Error parsing today's row. Resetting hourly data.");
                publishMQTT(MQTT_DEBUG_LOG, "Error parsing today's row. Resetting hourly data.");
                memset(hourlyCount, 0, sizeof(hourlyCount)); // Reset to zeros
            }

            break; // Exit loop after processing today's row
        }
    }
    file.close();

    if (!rowFound) {
        Serial.println("No data for today found. Resetting hourly data.");
        publishMQTT(MQTT_DEBUG_LOG, "No data for today found. Resetting hourly data.");
        memset(hourlyCount, 0, sizeof(hourlyCount)); // Reset to zeros
    }
}

/***** UPDATE and SAVE TOTALS TO SD CARD *****/
/** Save the daily Total of cars counted */
void saveDailyTotal() {
    myFile = SD.open(fileName1, FILE_WRITE);
    if (myFile) {
        myFile.print(totalDailyCars);
        myFile.close();
    } else {
        Serial.print(F("SD Card: Cannot open the file: "));
        Serial.println(fileName1);
    }

    // GAL 25-11-24: Retained state publishes
    publishMQTT(MQTT_PUB_EXIT_CARS,   String(totalDailyCars), true);
    publishMQTT(MQTT_PUB_INPARK_CARS, String(inParkCars),     true);
}


/* Save the grand total cars file for season  */
void saveShowTotal() {  
    myFile = SD.open(fileName2, FILE_WRITE);
    if (myFile) {
        myFile.print(totalShowCars);
        myFile.close();
    } else {
        Serial.print(F("SD Card: Cannot open the file: "));
        Serial.println(fileName2);
    }

    // GAL 25-11-24: retained state publish
    publishMQTT(MQTT_PUB_SHOWTOTAL, String(totalShowCars), true);  
}


// Save the calendar day to file
void saveDayOfMonth() {
    myFile = SD.open(fileName3, FILE_WRITE);
    if (myFile) {
        myFile.print(dayOfMonth);
        myFile.close();
    } else {
        Serial.print(F("SD Card: Cannot open the file: "));
        Serial.println(fileName3);
    }

    // GAL 25-11-24: Retained state publish (required)
    publishMQTT(MQTT_PUB_DAYOFMONTH, String(dayOfMonth), true);
}


/** Save number of days the show has been running */
void saveDaysRunning() {
    myFile = SD.open(fileName4, FILE_WRITE);
    if (myFile) {
      myFile.print(daysRunning);
      myFile.close();
    } else {
      Serial.print(F("SD Card: Cannot open the file: "));
      Serial.println(fileName4);
    }

    // retained state publish (NEW tree topic already correct)
    publishMQTT(MQTT_PUB_DAYSRUNNING, String(daysRunning), true);
}


// Save cars counted each hour in the event of a reboot
// Refactored saveHourlyCounts function
void saveHourlyCounts() {
    DateTime now = rtc.now();
    char dateBuffer[13];
    snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", now.year(), now.month(), now.day());

    int currentHour = now.hour(); // Get the current hour (0-23)

    File file = SD.open(fileName5, FILE_READ);
    String updatedContent = "";
    bool rowExists = false;

    if (file) {
        while (file.available()) {
            String line = file.readStringUntil('\n');
            if (line.startsWith(dateBuffer)) {
                rowExists = true;
                updatedContent += dateBuffer; // Start with the date
                int lastCommaIndex = line.indexOf(",") + 1; // Start after the date
 
                // Parse each value in the line
                for (int i = 0; i < 24; i++) {
                    int nextCommaIndex = line.indexOf(",", lastCommaIndex);
                    String currentValue = (nextCommaIndex != -1) 
                                           ? line.substring(lastCommaIndex, nextCommaIndex)
                                           : line.substring(lastCommaIndex);
                    lastCommaIndex = (nextCommaIndex != -1) ? nextCommaIndex + 1 : lastCommaIndex;

                    // Replace value for the current hour
                    updatedContent += (i == currentHour) 
                                      ? "," + String(hourlyCount[i]) 
                                      : "," + currentValue;
                }
                updatedContent += "\n";

            } else {
                updatedContent += line + "\n"; // Preserve other rows
            }
        }
        file.close();
    }

    // If no row exists for today, create a new one
    if (!rowExists) {
        updatedContent += dateBuffer;
        for (int i = 0; i < 24; i++) {
            updatedContent += (i == currentHour) ? "," + String(hourlyCount[i]) : ",0";
        }
        updatedContent += "\n";

        // Publish debug log
        char debugMessage[100];
        snprintf(debugMessage, sizeof(debugMessage), "Hourly data saved for hour %02d.", currentHour);
        publishMQTT(MQTT_DEBUG_LOG, debugMessage);
    }

    // -------------------------------------------------
    // GAL 25-11-24: Publish current hour's data to MQTT
    // NEW tree: msb/traffic/GateCounter/Cars/Hourly/07
    // -------------------------------------------------
    char hourlyTopic[120];
    snprintf(hourlyTopic, sizeof(hourlyTopic),
             "%s/%02d", MQTT_PUB_CARS_HOURLY, currentHour);
    publishMQTT(hourlyTopic, String(hourlyCount[currentHour]), true);

    // Write updated content back to the file
    file = SD.open(fileName5, FILE_WRITE);
    if (file) {
        file.print(updatedContent);
        file.close();
        Serial.printf("Hourly counts for hour %02d saved and published.\n", currentHour);
    } else {
        Serial.println("Failed to open GateHourlyData.csv for writing.");
    }
}


// Save and Publish Show Totals
void saveDailyShowSummary() {
    DateTime now = rtc.now();

    // Define show hours (5 PM to 9 PM)
    //const int showStartHour = 17; // 5 PM
    //const int showEndHour = 20;  // Up to 9 PM

    // Calculate cumulative totals for each key hour
    int cumulative6PM = hourlyCount[17]; // Total at 6 PM
    int cumulative7PM = cumulative6PM + hourlyCount[18]; // Total at 7 PM
    int cumulative8PM = cumulative7PM + hourlyCount[19]; // Total at 8 PM
    int cumulative9PM = cumulative8PM + hourlyCount[20]; // Total at 9 PM

    // Calculate total cars counted before the show starts
    int totalBefore5 = 0;
    for (int i = 0; i < 17; i++) { // Loop from hour 0 to hour 16
        totalBefore5 += hourlyCount[i];
    }

    // Include additional cars detected between 9:00 PM and 9:20 PM
    if (now.hour() * 60 + now.minute() <= showEndMin) {
        cumulative9PM += hourlyCount[21];
    }

    // Calculate the average temperature during show hours (5 PM to 9 PM)
    float showTempSum = 0.0;
    int showTempCount = 0;

    for (int i = 17; i <= 20; i++) { // Loop only between 5 PM and 9 PM
        if (hourlyTemp[i] != 0.0) { // Include valid temperature readings
            showTempSum += hourlyTemp[i];
            showTempCount++;
        }
    }

    float showAverageTemp = (showTempCount > 0) ? (showTempSum / showTempCount) : 0.0;

    // Open file for appending
    File summaryFile = SD.open(fileName7, FILE_APPEND);
    if (!summaryFile) {
        Serial.println("Failed to open daily show summary file.");
        publishMQTT(MQTT_DEBUG_LOG, "Failed to open daily show summary file.");
        return;
    }

    // Format the current date
    char dateBuffer[13];
    snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", now.year(), now.month(), now.day());

    // Append the show summary data
    summaryFile.printf("%s,%d,%d,%d,%d,%d,%d,%d,%.1f\n",
                       dateBuffer,       // Current date
                       daysRunning,      // Total days running
                       totalBefore5,     // Cars counted before 5pm
                       cumulative6PM,    // Cumulative total up to 6 PM
                       cumulative7PM,    // Cumulative total up to 7 PM
                       cumulative8PM,    // Cumulative total up to 8 PM
                       cumulative9PM,    // Cumulative total up to 9 PM, including 9:10 PM cars
                       totalShowCars,    // Total show cars
                       showAverageTemp); // Average temperature during show hours
    summaryFile.close();

    // Publish show summary data to MQTT (retained latest summary)
    publishMQTT(
        MQTT_PUB_SUMMARY,
        String("Date: ") + dateBuffer +
            ", DaysRunning: " + daysRunning +
            ", Before5: " + totalBefore5 +
            ", 6PM: " + cumulative6PM +
            ", 7PM: " + cumulative7PM +
            ", 8PM: " + cumulative8PM +
            ", 9PM: " + cumulative9PM +
            ", ShowTotal: " + totalShowCars +
            ", ShowAvgTemp: " + String(showAverageTemp, 1),
        true
    );
}

void getSavedValuesOnReboot() {
    DateTime now = rtc.now();

    // Read the last recorded day from the SD card
    getDayOfMonth();
    getShowStartDate();

    // Check if the ESP32 is rebooting on a new day
    if (now.day() != lastDayOfMonth) {
        dayOfMonth = now.day();   // Update to the current day
        saveDayOfMonth();         // Save the new day to the SD card

        totalDailyCars = 0;       // Reset daily car count
        saveDailyTotal();         // Persist reset

        // Show total should NOT reset on a new day
        getShowTotal();

        // Recompute daysRunning, except on Christmas Eve (legacy gap rule)
        if (!(now.month() == 12 && now.day() == 24)) {
            daysRunning = computeDaysRunningFromStart();
            saveDaysRunning();

            publishMQTT(
                MQTT_DEBUG_LOG,
                "Rebooted on new day. DaysRunning recomputed from showStartDate."
            );
        } else {
            publishMQTT(
                MQTT_DEBUG_LOG,
                "Rebooted on Christmas Eve. DaysRunning left unchanged (legacy gap)."
            );
        }

        // Log the update
        Serial.println("ESP32 reboot detected on a new day. Counts reset/updated.");
        publishMQTT(MQTT_DEBUG_LOG, "Rebooted, Counts reset/updated for new day.");

    } else {
        // Same day reboot: reload everything
        getDailyTotal();
        getShowTotal();
        getDaysRunning();
        getHourlyData();

        Serial.println("ESP32 reboot detected on the same day. Reloading saved counts.");
        publishMQTT(MQTT_DEBUG_LOG, "Rebooted, Counts reloaded for same day.");
    }

    // -------------------------------------------------
    // GAL 25-11-24: republish retained state on reboot
    // so HA/Grafana never sit stale after ESP reboot.
    // -------------------------------------------------
    inParkCars = carCounterCars - totalDailyCars;

    // Calendar is now sourced from CarCounter → callback mirror
    // publishMQTT(MQTT_PUB_DAYOFMONTH,  String(dayOfMonth),     true);
    // publishMQTT(MQTT_PUB_DAYSRUNNING, String(daysRunning),    true);
    publishMQTT(MQTT_PUB_EXIT_CARS,   String(totalDailyCars), true);
    publishMQTT(MQTT_PUB_SHOWTOTAL,   String(totalShowCars),  true);
    publishMQTT(MQTT_PUB_INPARK_CARS, String(inParkCars),     true);
}




/***** END OF DATA STORAGE & RETRIEVAL OPS *****/
void debugInPark(const char* reason) {
    String msg = String(reason) +
                 " Enter=" + String(carCounterCars) +
                 " Exit="  + String(totalDailyCars) +
                 " InPark=" + String(inParkCars);
    publishMQTT(MQTT_DEBUG_LOG, msg);
}


/*** MQTT CALLBACK TOPICS ****/
static inline bool topicIs(const char* t, const char* target) {
  return strcmp(t, target) == 0;
}



static inline void publishStateExitInPark() {
  publishMQTT(MQTT_PUB_EXIT_CARS,   String(totalDailyCars), true);
  publishMQTT(MQTT_PUB_INPARK_CARS, String(inParkCars),     true);
}

// GAL 25-11-27: Publish timing configuration (for MQTT/HA visibility)
static inline void publishTimingConfig() {
  // These use the same topics you subscribe to:
  //   msb/traffic/GateCounter/Config/gateCounterTimeout
  //   msb/traffic/GateCounter/Config/carDetectMS
  publishMQTT(MQTT_SUB_GATE_TIMEOUT, String(gateCounterTimeout), true);
  publishMQTT(MQTT_SUB_CARMS,        String(carDetectMS),        true);
}

void callback(char* topic, byte* payload, unsigned int length) {

  char message[length + 1];
  strncpy(message, (char*)payload, length);
  message[length] = '\0'; // Safely null-terminate the payload

  // -------------------------------------------------
  // Car Counter → ShowStartDate
  // Topic: msb/traffic/CarCounter/Config/showStartDate
  // Payload: YYYY-MM-DD
  // -------------------------------------------------
  if (topicIs(topic, "msb/traffic/CarCounter/Config/showStartDate")) {
      showStartDate = message;
      showStartDateValid = (showStartDate.length() == 10); // "YYYY-MM-DD"
      Serial.printf("ShowStartDate received: %s\n", showStartDate.c_str());
      return;
  }

  // -------------------------------------------------
  // Car Counter → EnterTotal (new tree)
  // -------------------------------------------------
  if (topicIs(topic, MQTT_SUB_CC_ENTER_TOTAL))  {
    carCounterCars = atoi(message);
    inParkCars = carCounterCars - totalDailyCars; // recalc cars in park

    if (carCounterCars != lastcarCounterCars) {
      publishStateExitInPark();
      debugInPark("EnterUpdate");
      lastcarCounterCars = carCounterCars;
    }
    return;
  }

  // -------------------------------------------------
  // Car Counter → Calendar mirror to GateCounter
  // -------------------------------------------------
  if (topicIs(topic, MQTT_SUB_CC_DAY_OF_MONTH)) {
    dayOfMonth = atoi(message);
    saveDayOfMonth();

    // Re-publish under the GateCounter tree so HA keeps using existing sensors
    publishMQTT(MQTT_PUB_DAYOFMONTH, String(dayOfMonth), true);

    Serial.print(F(" GateCounter DayOfMonth mirrored from CarCounter: "));
    Serial.println(dayOfMonth);
    return;
  }

  if (topicIs(topic, MQTT_SUB_CC_DAYS_RUNNING)) {
    daysRunning = atoi(message);
    saveDaysRunning();

    publishMQTT(MQTT_PUB_DAYSRUNNING, String(daysRunning), true);

    Serial.print(F(" GateCounter DaysRunning mirrored from CarCounter: "));
    Serial.println(daysRunning);
    return;
  }

  // -------------------------------------------------
  // Config resets / setpoints (new tree)
  // -------------------------------------------------
  if (topicIs(topic, MQTT_SUB_GATE_RESET_DAILY)) {
    totalDailyCars = atoi(message);
    saveDailyTotal();
    Serial.println(F(" Gate Counter Updated"));

    publishStateExitInPark();
    publishDebugLog("Daily Total Updated");
    return;
  }

  if (topicIs(topic, MQTT_SUB_GATE_RESET_SHOW)) {
    totalShowCars = atoi(message);
    saveShowTotal();
    Serial.println(F(" Show Counter Updated"));

    publishMQTT(MQTT_PUB_SHOWTOTAL, String(totalShowCars), true);
    publishDebugLog("Show Counter Updated");
    return;
  }

  if (topicIs(topic, MQTT_SUB_GATE_RESET_DOM)) {
    dayOfMonth = atoi(message);
    saveDayOfMonth();
    Serial.println(F(" Calendar Day of Month Updated"));

    publishMQTT(MQTT_PUB_DAYOFMONTH, String(dayOfMonth), true);
    publishDebugLog("Calendar Day Updated");
    return;
  }

  if (topicIs(topic, MQTT_SUB_GATE_RESET_DAYS)) {
    daysRunning = atoi(message);
    saveDaysRunning();
    Serial.println(F(" Days Running Updated"));

    publishMQTT(MQTT_PUB_DAYSRUNNING, String(daysRunning), true);
    publishDebugLog("Days Running Reset");
    return;
  }

  if (topicIs(topic, MQTT_SUB_GATE_TIMEOUT)) {
    gateCounterTimeout = atoi(message);
    Serial.print(F(" Gate Counter Alarm Timer Updated (ms): "));
    Serial.println(gateCounterTimeout);

    // // Echo the new value back out so MQTT has a retained state
    // publishMQTT(MQTT_SUB_GATE_TIMEOUT, String(gateCounterTimeout), true);

    publishDebugLog("Gate Counter Timeout Updated");
    return;
  }

  if (topicIs(topic, MQTT_SUB_CARMS)) {
    carDetectMS = atoi(message);
    Serial.print(F(" Gate Counter carDetectMS Updated (ms): "));
    Serial.println(carDetectMS);

    // DO NOT echo this topic back to MQTT – HA owns this value.
    // publishMQTT(MQTT_SUB_CARMS, String(carDetectMS), true);

    publishDebugLog("Gate Counter carDetectMS Updated");
    return;
  }

  if (topicIs(topic, MQTT_SUB_LOGGING)) {
    if (strcmp(message, "1") == 0) {
      loggingEnabled = true;
      Serial.println("Sensor logging ENABLED.");
      publishDebugLog("Logging Enabled");
    } else if (strcmp(message, "0") == 0) {
      loggingEnabled = false;
      Serial.println("Sensor logging DISABLED.");
      publishDebugLog("Logging Disabled");
    }
    return;
  }
}
/***** END OF CALLBACK TOPICS *****/


/***** IDLE STUFF  *****/
void logSensorStates() {
    if (!loggingEnabled) return; // Do not log if logging is disabled

    static unsigned long lastLogTime = 0; // Prevent excessive logging
    unsigned long currentTime = millis();

    if (currentTime - lastLogTime >= 100) { // Log every 100 ms
        lastLogTime = currentTime;

        // Read sensor states
        beamBState = digitalRead(beamSensorPin); // Beam B: HIGH = clear, LOW = broken
        //beamAState = !digitalRead(magSensorPin); // Active high
        beamAState = digitalRead(magSensorPin);  // Beam A: HIGH = clear, LOW = broken  25-11-20 GAL took off invert     

        // Log to Serial Monitor (or save to SD card)
        Serial.printf("%lu,%d,%d\n", currentTime, beamBState, beamAState);

        // OPTIONAL: Log to CSV file on SD card
        File logFile = SD.open(fileName10, FILE_APPEND);
        if (logFile) {
            logFile.printf("%lu,%d,%d\n", currentTime, beamBState, beamAState);
            logFile.close();
        } else {
            Serial.println("Failed to open sensorLog.csv");
        }
    }
}
// Average Temperature each hour
void averageHourlyTemp() {
    static int lastPublishedHour = -1;     // Tracks the last hour when data was published
    static int tempReadingsCount = 0;      // Number of valid temperature readings
    static float tempReadingsSum = 0.0;    // Sum of valid temperature readings
    
    // Get the current time
    DateTime now = rtc.now();
    int nowHour = now.hour();

    // Check if the hour has changed
    if (nowHour != lastPublishedHour) {
        // Publish the average for the completed hour
        if (tempReadingsCount > 0 && lastPublishedHour >= 0) {
            hourlyTemp[lastPublishedHour] = tempReadingsSum / tempReadingsCount;

            // Publish to MQTT
            char hourlyTopic[120];  // GAL 25-11-24: expanded for new /Env/tempHumidity path
            snprintf(hourlyTopic, sizeof(hourlyTopic),
                     "%s/hourly/%02d", MQTT_PUB_TEMP, lastPublishedHour);

            publishMQTT(hourlyTopic, String(hourlyTemp[lastPublishedHour], 1), true); 
            // retained hourly snapshot

            // Log the published temperature
            Serial.printf("Hour %02d average temperature published: %.1f°F\n",
                          lastPublishedHour, hourlyTemp[lastPublishedHour]);
            //publishDebugLog("Hourly average temperature published: " + String(hourlyTemp[lastPublishedHour], 1));
        }

        // Reset for the new hour
        lastPublishedHour = nowHour;
        tempReadingsSum = 0.0;
        tempReadingsCount = 0;
    }

    // Add the latest temperature reading if valid
    if (tempF != -999) { // Ensure only valid readings are processed
        tempReadingsSum += tempF;
        tempReadingsCount++;
        //publishDebugLog("Temperature added for averaging: " + String(tempF));
    }
}


// Exit Car Counted, increment the counter by 1 and append to the Exitlog.csv log file on the SD card
void countTheCar() {
    DateTime now = rtc.now();

    // Build a fresh timestamp string (do NOT use now.toString(buf2) here)
    char timeBuf[25];
    snprintf(timeBuf, sizeof(timeBuf),
             "%04d-%02d-%02d %02d:%02d:%02d",
             now.year(), now.month(), now.day(),
             now.hour(), now.minute(), now.second());

    // Serial log
    Serial.print(timeBuf);
    Serial.print(", Time to pass = ");
    Serial.println(timeToPassMS);

    totalDailyCars++;

    // Increment hourly car count
    int currentHour = now.hour();
    hourlyCount[currentHour]++;

    saveDailyTotal();   // retain numbers with reboot
    saveHourlyCounts();

    // Publish current hour's data to MQTT (NEW tree)
    // Final form: msb/traffic/GateCounter/Cars/Hourly/07
    char hourlyTopic[120];  // GAL 25-11-24: expanded for new bucketed paths
    snprintf(hourlyTopic, sizeof(hourlyTopic),
             "%s/%02d", MQTT_PUB_CARS_HOURLY, currentHour);

    publishMQTT(hourlyTopic, String(hourlyCount[currentHour]), true);

    // increase Show Count only when show is open
    if (showTime == true) {
        totalShowCars++;
        saveShowTotal();
    }

    inParkCars = carCounterCars - totalDailyCars;

    // open file for writing Car Data
    myFile = SD.open(fileName6, FILE_APPEND);
    if (myFile) {
        // DateTime
        myFile.print(timeBuf);
        myFile.print(", ");

        // TimeToPass_ms (A broken -> B cleared)
        myFile.print(timeToPassMS);
        myFile.print(", ");

        // ExitDailyTotal
        myFile.print(totalDailyCars);
        myFile.print(", ");

        // InParkCars
        myFile.print(inParkCars);
        myFile.print(", ");

        // AB_Follow_ms (A -> B follow time)
        myFile.print(abFollow_ms);
        myFile.print(", ");

        // TimeBetweenCars_ms This car to previous car
        myFile.println(timeBetweenCars_ms);

        myFile.close();

        // ---- Event-driven state publishes (NEW tree) ----
        publishMQTT(MQTT_PUB_TIME, timeBuf);  // use the same timestamp string

        publishMQTT(MQTT_PUB_EXIT_CARS,   String(totalDailyCars), true);
        publishMQTT(MQTT_PUB_INPARK_CARS, String(inParkCars),     true);
        publishMQTT(MQTT_PUB_SHOWTOTAL,   String(totalShowCars),  true);

        publishMQTT(MQTT_PUB_BEAM_B_STATE, String(beamBState), true);
        publishMQTT(MQTT_PUB_BEAM_A_STATE, String(beamAState), true);

        publishMQTT(MQTT_PUB_TTP, String(timeToPassMS), true);

    } else {
        Serial.print(F("SD Card: Cannot open the file: "));
        Serial.println(fileName6);
    }
}




// =========================================================
// DUAL-BEAM CAR DETECTION 25-11-20 GAL
// Beam A  (magSensorPin, 32)  : upstream beam
// Beam B  (beamSensorPin, 33) : downstream beam
// Both are normally HIGH (beam clear) and go LOW when broken.
// =========================================================

//   /* 24/10/14 - Both beams are normally open. Optocoupler reads HIGH when sensors are NOT tripped
//   changed code to read inverse of pin. Changing pinmode from pullup or pulldown made no difference 
//   Continually Read state of sensors. REVISED 12/12/24 with new Through Beam Sensor 
//   When the magSensor drops LOW during the IDLE state and beamSensor is also LOW, reset magSensorWasTriggered
//   to prepare for a new car.*/


/** GateCounter CarCounter-parity state machine (keeps MQTT messages) */
void detectCar() {
    unsigned long currentMillis = millis();

    // Raw reads (25.11.21.1 VERIFIED): raw HIGH = broken, raw LOW = clear
    bool rawA = digitalRead(magSensorPin);    // Beam A
    bool rawB = digitalRead(beamSensorPin);   // Beam B

    // ---- Debounce Beam A ----
    if (rawA != stableRawA) {
        if (currentMillis - lastRawAChangeMs >= debounceDelay) {
            stableRawA = rawA;
            lastRawAChangeMs = currentMillis;
        }
    } else {
        lastRawAChangeMs = currentMillis;
    }

    // ---- Debounce Beam B ----
    if (rawB != stableRawB) {
        if (currentMillis - lastRawBChangeMs >= debounceDelay) {
            stableRawB = rawB;
            lastRawBChangeMs = currentMillis;
        }
    } else {
        lastRawBChangeMs = currentMillis;
    }

    // Map to internal + MQTT semantics (unchanged from 25.11.21.1):
    // 1 = broken, 0 = clear
    beamAState = (stableRawA == HIGH) ? 1 : 0;
    beamBState = (stableRawB == HIGH) ? 1 : 0;

    // Publish stable state changes (same topics as now)
    if (beamBState != lastBeamBState) {
        lastBeamBState = beamBState;
        publishMQTT(MQTT_PUB_BEAM_B_STATE, String(beamBState), true);
    }
    if (beamAState != lastBeamAState) {
        lastBeamAState = beamAState;
        publishMQTT(MQTT_PUB_BEAM_A_STATE, String(beamAState), true);
    }

    bool aBroken = (beamAState == 1);
    bool bBroken = (beamBState == 1);

    switch (gateDetectState) {

        case WAITING_FOR_CAR: {
            unsigned long now = currentMillis;

            // -----------------------------------------
            // Idle Beam Health Check (no car in progress)
            // -----------------------------------------

            // Beam A health
            if (aBroken) {
                if (firstBeamHealth_ms == 0) {
                    firstBeamHealth_ms = now;  // start timing
                } else if ((now - firstBeamHealth_ms) >= gateCounterTimeout && !gateStuckAlarmActive) {
                    publishMQTT(MQTT_COUNTER_LOG, "Beam A stuck HIGH (idle)");
                    publishMQTT(MQTT_PUB_ALARM, "ALARM_GATE_STUCK", true);
                    gateStuckAlarmActive = true;
                }
            } else {
                firstBeamHealth_ms = 0;
            }

            // Beam B health
            if (bBroken) {
                if (secondBeamHealth_ms == 0) {
                    secondBeamHealth_ms = now;  // start timing
                } else if ((now - secondBeamHealth_ms) >= gateCounterTimeout && !gateStuckAlarmActive) {
                    publishMQTT(MQTT_COUNTER_LOG, "Beam B stuck HIGH (idle)");
                    publishMQTT(MQTT_PUB_ALARM, "ALARM_GATE_STUCK", true);
                    gateStuckAlarmActive = true;
                }
            } else {
                secondBeamHealth_ms = 0;
            }

            // Clear stuck alarm when both beams recover in idle
            if (!aBroken && !bBroken && gateStuckAlarmActive) {
                publishMQTT(MQTT_PUB_ALARM, "CLEAR", true);
                gateStuckAlarmActive = false;
            }

            // -----------------------------------------
            // Normal entry into car detection
            // -----------------------------------------
            // Start on Beam A broken while B is still clear
            if (aBroken && !bBroken) {
                beamATripTime_ms = currentMillis;
                gateDetectState  = BEAM_A_HIGH;

                // NEW: reset one-shot timeout log at event start (per-event determinism)
                abTimeoutLogged = false;

                publishMQTT(MQTT_COUNTER_LOG, "Beam A broken (event start).");
            }

            break;
        }

        case BEAM_A_HIGH:
            // Beam B follows → validate minimum activation
            if (bBroken) {

                // A→B FOLLOW TIMING (correct location)
                abFollow_ms = currentMillis - beamATripTime_ms;
                publishMQTT(MQTT_PUB_BEAM_AB_MS, String(abFollow_ms));

                // Validate minimum activation
                unsigned long timeBeamsHigh = currentMillis - beamATripTime_ms;

                if (timeBeamsHigh >= minActivationDuration) {
                    bothBeamsBroken_ms = currentMillis;
                    carPresentFlag = true;
                    gateDetectState = BOTH_BEAMS_HIGH;

                    // Leaving BEAM_A_HIGH via success path → reset log flag
                    abTimeoutLogged = false;

                    publishMQTT(MQTT_COUNTER_LOG, "State changed: Both beams High.");
                }
            }
            // If A clears before B breaks → reset
            else if (!aBroken) {
                gateDetectState = WAITING_FOR_CAR;

                // Leaving BEAM_A_HIGH via reset path → reset log flag
                abTimeoutLogged = false;

                publishMQTT(MQTT_COUNTER_LOG, "Beam A cleared before Beam B. Reset.");
            }

            // If B never follows within maxABFollow_ms → DIAGNOSTIC ONLY (do NOT reset)
            if (!bBroken && (currentMillis - beamATripTime_ms > maxABFollow_ms)) {
                if (!abTimeoutLogged) {
                    publishMQTT(
                        MQTT_COUNTER_LOG,
                        "Beam A->B follow exceeded maxABFollow_ms (NO RESET). Continuing to wait for Beam B."
                    );
                    abTimeoutLogged = true;
                }
            }

            break;

        case BOTH_BEAMS_HIGH:
            // Stuck-vehicle alarm
            if ((currentMillis - beamATripTime_ms) >= gateCounterTimeout) {
                if (!gateStuckAlarmActive) {
                    publishMQTT(MQTT_PUB_ALARM, "ALARM_GATE_STUCK", true);
                    publishMQTT(MQTT_COUNTER_LOG, "Sensor blocked", false);
                    gateStuckAlarmActive = true;
                }
            }

            // When Beam B clears, validate duration and move to count
            if (!bBroken && carPresentFlag) {
                unsigned long brokenDuration = currentMillis - bothBeamsBroken_ms;

                publishMQTT(
                    MQTT_COUNTER_LOG,
                    "Beam B clear. Broken duration: " + String(brokenDuration) + " ms"
                );
                publishMQTT(MQTT_PUB_BEAM_B_BROKEN_MS, String(brokenDuration));

                if (brokenDuration >= carDetectMS) {
                    gateDetectState = CAR_DETECTED;
                    publishMQTT(MQTT_COUNTER_LOG, "Changed state to Car Detected", false);
                } else {
                    carPresentFlag = false;
                    gateDetectState = WAITING_FOR_CAR;
                    publishMQTT(MQTT_COUNTER_LOG, "No car detected (duration too short).");
                }

                // Clear stuck alarm when event ends
                if (gateStuckAlarmActive) {
                    publishMQTT(MQTT_PUB_ALARM, "CLEAR", true);
                    gateStuckAlarmActive = false;
                }
            }
            break;

        case CAR_DETECTED:
            if (carPresentFlag) {
                // TTP from Beam A broken to Beam B clear
                timeToPassMS = currentMillis - beamATripTime_ms;
                publishMQTT(MQTT_PUB_TTP, String(timeToPassMS));
                publishMQTT(MQTT_COUNTER_LOG, "Car confirmed and counted!");

                countTheCar();

                // Between cars
                if (lastCarDetected_ms > 0) {
                    timeBetweenCars_ms = currentMillis - lastCarDetected_ms;
                    publishMQTT(MQTT_PUB_BETWEENCARS_MS, String(timeBetweenCars_ms), true);
                }
                lastCarDetected_ms = currentMillis;

                carPresentFlag = false;
                gateDetectState = WAITING_FOR_CAR;
                publishMQTT(MQTT_COUNTER_LOG, "System ready for next car.");
            }
            break;
    }
}
// END GATE CAR DETECTION




// CHECK AND CREATE FILES on SD Card and WRITE HEADERS if Needed
void checkAndCreateFile(const String &fileName, const String &header = "") {
    if (!SD.exists(fileName)) {
        Serial.printf("%s not found. Creating...\n", fileName.c_str());
        if (fileName.endsWith("/")) { // Create directory if it ends with '/'
            if (!SD.mkdir(fileName)) {
                Serial.printf("Failed to create directory %s\n", fileName.c_str());
            } else {
                Serial.printf("Directory %s created successfully\n", fileName.c_str());
            }
        } else { // Create file if not a directory
            File file = SD.open(fileName, FILE_WRITE);
            if (!file) {
                Serial.printf("Failed to create file %s\n", fileName.c_str());
            } else {
                if (!header.isEmpty()) {
                    file.println(header);
                }
                file.close();
                Serial.printf("File %s created successfully\n", fileName.c_str());
            }
        }
    }
}

void createAndInitializeHourlyFile(const String &fileName) {
    if (!SD.exists(fileName)) {
        Serial.printf("%s not found. Creating...\n", fileName.c_str());
        File file = SD.open(fileName, FILE_WRITE);
        if (!file) {
            Serial.printf("Failed to create file %s\n", fileName.c_str());
            return;
        }

        // Write the header
        file.println("Date,Hr 00,Hr 01,Hr 02,Hr 03,Hr 04,Hr 05,Hr 06,Hr 07,Hr 08,Hr 09,Hr 10,Hr 11,Hr 12,Hr 13,Hr 14,Hr 15,Hr 16,Hr 17,Hr 18,Hr 19,Hr 20,Hr 21,Hr 22,Hr 23");

        // Add an initial row for the current day
        DateTime now = rtc.now();
        char dateBuffer[13];
        snprintf(dateBuffer, sizeof(dateBuffer), "%04d-%02d-%02d", now.year(), now.month(), now.day());
        file.print(dateBuffer); // Write the current date

        // Initialize 24 hourly counts to 0
        for (int i = 0; i < 24; i++) {
            file.print(",0");
        }
        file.println(); // Move to the next line

        file.close();
        Serial.println("Hourly file created and initialized for the current day.");
    } else {
        Serial.printf("File %s already exists. No initialization performed.\n", fileName.c_str());
    }
}

/** Initilaize microSD card GAL 25-11-18*/ 
void initSDCard() {
  if(!SD.begin(PIN_SPI_CS)) {
    Serial.println("Card Mount Failed");
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, line1);
    display.println("Check SD Card");
    display.setCursor(0, line2);
    display.println("Logging disabled");
    display.display();
    // Removed while(1); — continue running without SD card
    return;
  }
  uint8_t cardType = SD.cardType();

  if(cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if(cardType == CARD_MMC){
    Serial.println("MMC");
  } else if(cardType == CARD_SD){
    Serial.println("SDSC");
  } else if(cardType == CARD_SDHC){
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);

  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  Serial.println(F("SD CARD INITIALIZED."));
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,line1);
  display.printf("SD Card Size: %lluMB\n", cardSize);
  display.display();
}

// Gate Counter Temperature and Humidity Reading (mirrors Car Counter behavior)
void readTempandRH() {
    // --- Non-blocking warmup and timing state (function-local statics) ---
    static bool dhtWarmupDone = false;
    static unsigned long dhtWarmupStart = 0;

    static unsigned long lastDHTReadMillis  = 0;    // Last time temperature was read
    static unsigned long lastDHTPrintMillis = 0;    // Last time temperature was printed

    const unsigned long DHT_WARMUP_MS      = 10000;   // 10 seconds warmup
    const unsigned long dhtReadInterval    = 60000;   // 60 seconds interval for reading temp
    const unsigned long dhtPrintInterval   = 600000;  // 10 minutes interval for printing temp

    static bool tempOutOfRangeReported = false;

    unsigned long currentMillis = millis();

    // --- Warmup gate: don't touch tempF/humidity until sensor has stabilized ---
    if (!dhtWarmupDone) {
        if (dhtWarmupStart == 0) {
            dhtWarmupStart = currentMillis;  // start warmup on first call
        }
        if (currentMillis - dhtWarmupStart < DHT_WARMUP_MS) {
            return;  // too early, skip DHT entirely
        }
        dhtWarmupDone = true;  // from now on, reads are allowed
    }

    // --- Interval gate: only read every dhtReadInterval ms ---
    if (currentMillis - lastDHTReadMillis < dhtReadInterval) {
        return;
    }
    lastDHTReadMillis = currentMillis;

    // Read temperature and humidity
    humidity = dht.readHumidity();
    tempF   = dht.readTemperature(true); // Read Fahrenheit directly

    // Check if the readings are valid
    if (isnan(tempF) || isnan(humidity)) {
        Serial.println("Failed to read from DHT sensor!");
        publishDebugLog("DHT sensor reading failed.");
        tempF    = -999;  // sentinel for failure
        humidity = -999;
        return; // Exit function if the readings are invalid
    }

    // Check for temperature out of range
    if (tempF < -40 || tempF > 120) {
        if (!tempOutOfRangeReported) {
            Serial.println("Temperature out of range!");
            publishDebugLog("DHT temperature out of range: " + String(tempF));
            tempOutOfRangeReported = true; // prevent duplicate reporting
        }
        tempF = -999; // sentinel for out-of-range condition
    } else {
        // Reset the flag if temperature is back in range
        if (tempOutOfRangeReported) {
            Serial.println("Temperature back in range.");
            tempOutOfRangeReported = false;
        }

        // Build JSON for temp/RH (actual MQTT publish happens elsewhere)
        char jsonPayload[100];
        snprintf(jsonPayload, sizeof(jsonPayload),
                 "{\"tempF\": %.1f, \"humidity\": %.1f}", tempF, humidity);
        // Do NOT publish here – Gate now matches Car: temp/RH publish happens
        // in the periodic MQTT/keepalive logic, not on every sensor read.

        // Forward valid readings to the hourly average system
        averageHourlyTemp(); // Ensure the reading is processed for summaries
    }

    // --- 10-minute printout for serial diagnostics ---
    if (currentMillis - lastDHTPrintMillis >= dhtPrintInterval) {
        lastDHTPrintMillis = currentMillis;

        if (tempF != -999 && humidity != -999) {
            Serial.printf("Temperature: %.1f °F, Humidity: %.1f %%\n", tempF, humidity);
        } else {
            Serial.println("Temperature/Humidity data invalid. Check sensor.");
        }
    }
}




/** Resets the hourly count array at midnight */
void resetHourlyCounts() {
    for (int i = 0; i < 24; i++) {
        hourlyCount[i] = 0; // Reset the hourly counts
    }
    // Log the reset
    Serial.println("Hourly counts have been reset.");
    publishMQTT(MQTT_DEBUG_LOG, "Hourly counts reset for the new day.");
}

/* Resets counts at Start of Show and Midnight */
void timeTriggeredEvents() {
    DateTime now = rtc.now();

    // Reset hourly counts at midnight
    if (now.hour() == 23 && now.minute() == 59 && !flagMidnightReset) { 
        resetHourlyCounts();  // Reset array for collecting hourly car counts      
        totalDailyCars = 0;   // Reset total daily cars to 0 at midnight 
        saveDailyTotal();        publishMQTT(MQTT_DEBUG_LOG, "Total cars reset at Midnight");        flagMidnightReset = true;
    }
    
    // Increment days running only if not Christmas Eve
    if (now.day() != lastDayOfMonth) {
        if (!(now.month() == 12 && now.day() == 24) && !flagDaysRunningReset) {
            daysRunning = computeDaysRunningFromStart();
            saveDaysRunning();
            publishMQTT(MQTT_DEBUG_LOG, "Days running: " + String(daysRunning));
        }
        dayOfMonth = now.day(); // Update day of month
        saveDayOfMonth(); // Save new day of month
        getDayOfMonth(); // Get Day of month
        publishMQTT(MQTT_DEBUG_LOG, "Day of month: " + String(dayOfMonth));
        flagDaysRunningReset = true;
    }

    // Reset total daily cars for show to 0 at 5:10 PM
    if (now.hour() == 17 && now.minute() == 8 && !flagDailyShowStartReset) {
        totalDailyCars = 0;
        saveDailyTotal();
        publishMQTT(MQTT_DEBUG_LOG, "Total Exit Cars Reset at 5:08 PM");
        flagDailyShowStartReset = true;
    }

    // Save daily summary at 9:20 PM
    if (now.hour() == 21 && now.minute() == 20 && !flagDailyShowSummarySaved) {
        saveDailyShowSummary();
        publishMQTT(MQTT_DEBUG_LOG, "Daily Show Summary Saved");
        flagDailyShowSummarySaved = true;
    }

    // 5 minute Event Timer
    if (now.minute() %5 == 0  && !flagHourlyReset) {
        //saveHourlyCounts(); // Saves hourly counts
        flagHourlyReset = true;
        // Add debug message with counts
        char debugMessage[100]; // Increase size if necessary
        int currentHour = now.hour(); // Get the current hour
        snprintf(debugMessage, sizeof(debugMessage), "Hourly data saved for hour %02d. Current count: %d.", 
                currentHour, hourlyCount[currentHour]);
        // Publish the debug message
        publishMQTT(MQTT_DEBUG_LOG, debugMessage);
        Serial.println(debugMessage); // Print to serial for debugging
    }

    // reset 5 Min Event Timer
    if (now.minute() %5 != 0) {
        flagHourlyReset = false;
    }


    // Reset flags for the next day at 12:01:01 AM
    if (now.hour() == 0 && now.minute() == 1 && now.second() == 1 && !resetFlagsOnce) { 
        flagDaysRunningReset = false;
        flagMidnightReset = false;
        flagDailyShowStartReset = false;
        flagDailySummarySaved = false;
        flagDailyShowSummarySaved = false;
        flagHourlyReset = false;
        publishMQTT(MQTT_DEBUG_LOG, "Run once flags reset for new day");
        resetFlagsOnce = true; // Prevent further execution within the same day
    }

    // Reset the `resetFlagsOnce` at 12:02:00 AM to allow it to run the next day
    if (now.hour() == 0 && now.minute() == 2 && now.second() == 0) {
        resetFlagsOnce = false; // Allow reset logic to run again the next day
    }
}


// Update OLED Display while running
void updateDisplay() {
    DateTime now = rtc.now();
    //float tempF = ((rtc.getTemperature() * 9 / 5) + 32); // Get temperature in Fahrenheit
    int currentHr24 = now.hour();
    int currentHr12 = currentHr24 > 12 ? currentHr24 - 12 : (currentHr24 == 0 ? 12 : currentHr24);
    const char* ampm = currentHr24 < 12 ? "AM" : "PM";

    // Clear display and set formatting
    display.clearDisplay();
    display.setTextColor(WHITE);
    //display.setFont(&FreeSans12pt7b);

    // Line 1: Date and Day of the Week
    display.setTextSize(1);
    display.setCursor(0, line1);
    display.printf("%s %s %02d, %04d", days[now.dayOfTheWeek()], months[now.month() - 1], now.day(), now.year());

    // Line 2: Time and Temperature
    display.setCursor(0, line2);
    display.printf("%02d:%02d:%02d %s  %d F", currentHr12, now.minute(), now.second(), ampm, (int)tempF);

    // Line 3: Days Running and Show Total
    display.setCursor(0, line3);
    display.printf("Day %d  Total: %d", daysRunning, totalShowCars);

    // Line 4: Total Daily Cars
    display.setTextSize(1);
    display.setCursor(0, line4);
    display.printf("Exiting: %d", totalDailyCars);

    //Line 5: In Park Cars
    display.setTextSize(1);
    display.setCursor(0, line5);
    display.printf("InPark: %d", carCounterCars - totalDailyCars);

    // Write to the display
    display.display();
}

/******  BEGIN SETUP ******/
void setup() {
    Serial.begin(115200);
    ElegantOTA.setAutoReboot(true);
    //ElegantOTA.setFilesystemMode(true);
    Serial.println("Starting Gate Counter...");

  //Initialize Display
    display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, line1);
    display.println("Initializing...");
    display.display();

// Set the ESP32 hostname
    if (WiFi.setHostname(THIS_MQTT_CLIENT)) {
        Serial.printf("Hostname set to: %s\n", THIS_MQTT_CLIENT);
    } else {
        Serial.println("Failed to set hostname!");
    }

    // Scan WiFi networks
    WiFi.mode(WIFI_STA);
    int n = WiFi.scanNetworks();
    Serial.println("WiFi scan completed.");
    if (n == 0) {
        Serial.println("No networks found.");
    } else {
        Serial.printf("%d networks found:\n", n);
        for (int i = 0; i < n; ++i) {
            Serial.printf("%d: %s (%d dBm) %s\n",
                          i + 1,
                          WiFi.SSID(i).c_str(),
                          WiFi.RSSI(i),
                          WiFi.encryptionType(i) == WIFI_AUTH_OPEN ? "OPEN" : "SECURE");
        }
    }

    // Add multiple WiFi APs
    wifiMulti.addAP(secret_ssid_AP_1, secret_pass_AP_1);
    wifiMulti.addAP(secret_ssid_AP_2, secret_pass_AP_2);
    wifiMulti.addAP(secret_ssid_AP_3, secret_pass_AP_3);
    wifiMulti.addAP(secret_ssid_AP_4, secret_pass_AP_4);
    wifiMulti.addAP(secret_ssid_AP_5, secret_pass_AP_5);
    setup_wifi();  

    //If RTC not present, stop and check battery
    if (! rtc.begin()) {
        rtcReady = false;
        Serial.println("Could not find RTC! Check circuit.");
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,line1);
        display.println("Clock DEAD");
        display.display();
        publishMQTT(MQTT_DEBUG_LOG, "EXIT RTC BEGIN FAILED", true);
        } else {
        rtcReady = true;   // GAL 25-11-22
    }

    // Get NTP time from Time Server 
    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
    SetLocalTime();
    DateTime now = rtc.now();

    // Save boot timestamp
    char buf[32];
    snprintf(buf, sizeof(buf), "%04d-%02d-%02d %02d:%02d:%02d",
            now.year(), now.month(), now.day(),
            now.hour(), now.minute(), now.second());

    // If RTC failed earlier, keep placeholder instead of junk time
    if (rtcReady) {
        bootTimestamp = String(buf);
    } else {
        bootTimestamp = "rtc-not-ready";
    }
  
    // MQTT Reconnection with login credentials
    MQTTreconnect(); // Ensure MQTT is connected

    // After MQTT connect on Gate Counter: ensure alarm state is CLEAR and retained
    publishMQTT(MQTT_PUB_ALARM, "CLEAR", true);
    gateStuckAlarmActive = false;

    // (Optional / existing) publish initial beam states as retained
    publishMQTT(MQTT_PUB_BEAM_A_STATE, String(beamAState), true);
    publishMQTT(MQTT_PUB_BEAM_B_STATE, String(beamBState), true);

    // Both beams via optocouplers: HIGH = clear, LOW = broken 25-11-20 GAL
    pinMode(magSensorPin, INPUT);      // Beam A
    pinMode(beamSensorPin, INPUT);     // Beam B

    delay(5); // tiny settle time

    // Read raw beam levels
    int rawA = digitalRead(magSensorPin);
    int rawB = digitalRead(beamSensorPin);

    // Map to logical: 1=blocked, 0=clear (2025 normally HIGH beams)
    beamAState  = (rawA == HIGH) ? 1 : 0;
    beamBState = (rawB == HIGH) ? 1 : 0;

    // Force MQTT to correct states on every reboot (RETAINED!)
    publishMQTT(MQTT_PUB_BEAM_B_STATE, String(beamBState), true);
    publishMQTT(MQTT_PUB_BEAM_A_STATE, String(beamAState), true);
    
    // GAL 25-11-27: Make sure default timing config is visible on MQTT
    publishTimingConfig();

    // Sync last-states so loop doesn't immediately republish
    lastBeamAState  = beamAState;
    lastBeamBState = beamBState;

    // Initialize DHT sensor
    dht.begin();
    Serial.println("DHT22 sensor initialized.");
    
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(0, line3);
    display.println(" Starting");
    display.println("GateCounter");

    Serial.println  ("Initializing Gate Counter");
    Serial.print("Temperature: ");
    Serial.print(tempF);
    Serial.println(" F");
    display.display();
    delay(500); // display Startup

    //Initialize SD Card
    SD.begin(PIN_SPI_CS);
    initSDCard();  // Initialize SD card and ready for Read/Write

    initSeasonalPaths();   // sets up annual data file structure
        publishDebugEvent(
        "season_files",
        "ExitTotal=" + fileName1 +
        " Hourly=" + fileName5 +
        " GateLog=" + fileName6,
        true
    );




    // Check and create Required Data files
    checkAndCreateFile(fileName1);
    checkAndCreateFile(fileName2);
    checkAndCreateFile(fileName3);
    checkAndCreateFile(fileName4);
    //checkAndCreateFile(fileName5, "Date,Hour-17,Hour-18,Hour-19,Hour-20,Hour-21,Total,Temp");
    checkAndCreateFile(fileName6, "DateTime, TimeToPass_ms, ExitDailyTotal, InParkCars, AB_Follow_ms, timeBetweenCars_ms");
    checkAndCreateFile(fileName7, "Date,DaysRunning,Before5,6PM,7PM,8PM,9PM,ShowTotal,DailyAvgTemp");
    checkAndCreateFile(fileName8);
    checkAndCreateFile(fileName9);
    checkAndCreateFile(fileName10, "ms,Beam,mag");

    // Required Hourly Data Files
    createAndInitializeHourlyFile(fileName5);

    // Initialize Server
    setupServer();

    //on reboot, get totals saved on SD Card
    getSavedValuesOnReboot();  // Update/reset counts based on reboot day

    // Setup MDNS
    if (!MDNS.begin(THIS_MQTT_CLIENT)) {
      Serial.println("Error starting mDNS");
      return;
    }
    delay(1000);
    start_MqttMillis = millis();
} /***** END SETUP ******/

void loop() {  
    DateTime now = rtc.now();

    currentTimeMinute = now.hour()*60 + now.minute(); // convert time to minutes since midnight

    showTime = (currentTimeMinute >= showStartMin && currentTimeMinute <= showEndMin); // show is running and save counts

    readTempandRH();          // Get Temperature and Humidity

    ElegantOTA.loop();        // Keep OTA Updates Alive

    updateDisplay();          // Update the display

    checkWiFiConnection();    // Check and maintain WiFi connection

    timeTriggeredEvents();    // Various functions/saves/resets based on time of day

    detectCar();              // Detect cars

    // Keep MQTT client alive and publish heartbeat on its own clock
    if (millis() - lastKeepAliveMillis >= (unsigned long)mqttKeepAlive * 1000UL) {
        KeepMqttAlive();
        lastKeepAliveMillis = millis();
    }
} 
/***** Repeat Loop *****/
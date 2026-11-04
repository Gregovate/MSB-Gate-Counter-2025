# Preserved firmware source history

Verbatim introductory comments from the pre-extraction main.cpp. Current interfaces live under include/traffic.

```text
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
```

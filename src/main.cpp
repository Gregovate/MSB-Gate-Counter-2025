/*
 * MSB Gate Counter — RELEASE CONTROL
 *
 * Before compiling a changed firmware:
 *   1. Add every change below, newest entry first.
 *   2. Manually update FWVersion immediately below this comment.
 * This is the only version value: MQTT, ElegantOTA and diagnostics use it.
 * Older original history is preserved in docs/Firmware_History.md.
 *
 * CHANGELOG — newest first
 * 26.11.25.4 — 2026-11-25
 *   - Explain passage-based hourly peaks and distinguish production session and SD errors.
 * 26.11.25.3 — 2026-11-25
 *   - Match report controls to the Car red and Gate blue file manager colors.
 * 26.11.25.2 — 2026-11-25
 *   - Serve legacy nightly reports in current column order before the next nightly write.
 * 26.11.25.1 — 2026-11-25
 *   - Release the installed 25.12.28.0-modular-test.25 code under the local release date.
 * 25.12.28.0-modular-test.25 — 2026-11-25
 *   - Group nightly hours through After9PM, preserve legacy reports and MQTT order.
 * 25.12.28.0-modular-test.24 — 2026-11-24
 *   - Archive the test season on the accepted date change, qualify passages by HA window,
 *     report dated occupancy and carry forward HA weather-closure day adjustments.
 * 25.12.28.0-modular-test.23 — 2026-11-24
 *   - Normalize String/char-array season paths before shared status formatting.
 *   - Remove unused Arduino-DS3231 dependency; retain RTClib RTC driver.
 *   - Expand Gate date buffers to eliminate truncation warnings.
 * 25.12.28.0-modular-test.22 — 2026-11-24
 *   - Retain completed hourly temperatures and replay them on MQTT reconnect.
 *   - Date each hourly slot; preserve the last 24 completed hours across midnight.
 * 25.12.28.0-modular-test.21 — 2026-11-24
 *   - Preserve Car outdoor temperature sample averages across reboots in NVS.
 *   - Keep saved report temperatures when no new valid average is available.
 *   - Label Gate enclosure temperature as unofficial backup; keep common modules.
 * 25.12.28.0-modular-test.20 — 2026-11-24
 *   - Restore shared daily report views, date selection and copy/print for email.
 *   - Replace deprecated response API; guard post-backup closed-snapshot deletion.
 * 25.12.28.0-modular-test.19 — 2026-11-23
 *   - Serve shared file manager from firmware even when SD is offline.
 *   - Open active season by default; expose read-only journal/capture status.
 *   - Show archive folders and separate routine downloads from maintenance.
 * 25.12.28.0-modular-test.18 — 2026-11-23
 *   - Standardize primary environment topics to Env/temp and hourly/HH.
 *   - Mirror legacy Gate temperature topics for existing HA subscriptions.
 * 25.12.28.0-modular-test.17 — 2026-11-23
 *   - Share identical DHT, environment averaging and show-summary modules.
 *   - Unify health schema, unavailable samples and reconnect/periodic publication.
 * 25.12.28.0-modular-test.16 — 2026-11-23
 *   - Decode bounded DHT22 two's-complement negative-temperature variants.
 *   - Preserve raw diagnostics, standard encoding, counting and journal paths.
 * 25.12.28.0-modular-test.15 — 2026-11-23
 *   - Commit vehicle/reset/correction snapshots to OTA-preserving flash journal.
 *   - Restore without SD; archive immutable timing records with verified bank reuse.
 *   - Capture beam edges in IRAM/DRAM during flash writes; replay debounce by time.
 *   - Report flash, SD and capture-overflow faults on MQTT and OLED.
 * 25.12.28.0-modular-test.14 — 2026-11-23
 *   - Retry startup count restoration after a missing SD becomes available.
 *   - Preserve corrupt checkpoints and live totals; show persistent SD faults.
 * 25.12.28.0-modular-test.13 — 2026-11-22
 *   - Preserve occupied/long passages; publish counts before SD verification.
 *   - Poll beams before and between foreground maintenance operations.
 *   - Correct Gate spacing log and both-clear rearming; preserve checkpoints.
 * 25.12.28.0-modular-test.12 — 2026-11-10
 *   - Invert existing magneticRawState: LOW input reports 1 (triggered).
 *   - Preserve raw diagnostic traces and beam behavior; no new sensor.
 * 25.12.28.0-modular-test.11 — 2026-11-09
 *   - Name GPIO32/33 as Beam A/B; add GPIO25 magnetic diagnostics only.
 *   - Trace raw levels and beam confirmations to MQTT/current-year SD with loss metrics.
 * 25.12.28.0-modular-test.10 — 2026-11-05
 *   - Coordinate persisted HA schedules/tuning with both counter releases.
 *   - Separate official show buckets from uninterrupted all-day traffic.
 *   - Add conditional extension reporting and verified idempotent summaries.
 *   - Include optional private AP 6 without committing secrets.h.
 *   - Remove WiFi boot wait; preserve .5 counts with versioned checkpoints.
 *   - Use Central daylight-saving rules and current-state MQTT reconnects.
 * 25.12.28.0-modular-test.8 — 2026-11-05
 *   - Share complete hourly JSON and canonical per-hour topic reporting.
 *   - Correct Gate double-slash hourly topics; preserve case-sensitive names.
 *   - Service count retries/date rollover in every main loop, not just events.
 *   - Document the HA naming contract and remaining configuration dependencies.
 * 25.12.28.0-modular-test.7 — 2026-11-05
 *   - Stop publishing firmware timing defaults onto HA command topics.
 *   - Share State-only reporting; acknowledge applied settings and repeat them
 *     on heartbeat without queueing disconnected configuration snapshots.
 *   - Document disabled Car beam command and remaining configuration gaps.
 * 25.12.28.0-modular-test.6 — 2026-11-05
 *   - Repeat the latest hourly-export status on heartbeat in both counters.
 *   - Report skipped/failed exports explicitly; do not replay stale count queues.
 *   - Test late status delivery, retained failures and successful recovery.
 * 25.12.28.0-modular-test.5 — 2026-11-05
 *   - Accept a verified zero legacy daily total with older valid hourly rows.
 *   - Preserve historical rows and reject inconsistent nonzero daily totals.
 *   - Add the exact field migration case and post-migration reboot regression.
 * 25.12.28.0-modular-test.4 — 2026-11-04
 *   - Save each vehicle in a shared verified, dated two-copy SD checkpoint.
 *   - Restore counts before reporting; preserve files when recovery is uncertain.
 *   - Retry failed checkpoints independently of MQTT and report persistence faults.
 *   - Use full-date rollover and persist show-reset markers across reboot.
 *   - Verify hourly CSV replacements and retain the previous complete file.
 * 25.12.28.0-modular-test.3 — 2026-11-04
 *   - Share persistent calendar-year selection; never reopen a prior year for writes.
 *   - Protect historical SD folders from logging and web mutations.
 *   - Derive Black Friday fallback annually; persist valid current-year HA dates.
 *   - Report accepted date separately; reject stale dates and compute pre-show day 0.
 *   - Rotate paths while running; retry network clock sync without rolling year back.
 *   - Preserve GPIOs, detector timing and manual release control.
 * 25.12.28.0-modular-test.2 — 2026-11-04
 *   - Restore the editable firmware identifier and active changelog to main.cpp.
 *   - Share the identifier through FirmwareVersion.h; no duplicate version value.
 *   - Allow deliberate version edits in preservation checks; keep logic checks.
 *   - Make version-helper text access explicitly UTF-8.
 * 25.12.28.0-modular-test.1 — 2026-11-04
 *   - Extract MQTT, network, web/OTA, temperature, clock, SD, display and detector
 *     services; retain existing counting behavior, pins and topic names.
 *   - Preserve original source history and exclude all licensed OTA source.
 *   - Add preservation/shared-file checks and Windows UTF-8/CRLF fixes.
 *   - Record local builds with licensed ElegantOTAPro 3.1.4; no device upload.
 */
#include "traffic/NightlyCsv.h"
#include "traffic/FirmwareVersion.h"
const char FWVersion[] = "26.11.25.4";

#include "traffic/Firmware.h"
#include "traffic/MagneticDiagnostics.h"

void setup() {
    Serial.begin(115200);
    pinMode(beamAPin, INPUT_PULLDOWN);pinMode(beamBPin, INPUT_PULLDOWN);
    initializeBeamCapture(beamAPin,beamBPin);
    initializeTrafficJournal();
    initializeProductionSession();
    ElegantOTA.setAutoReboot(true);
    ElegantOTA.setFilesystemMode(false); // Internal data contains the traffic journal; firmware OTA remains enabled.
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

    // Connect asynchronously; RTC, SD and detection must start without WiFi.
    WiFi.mode(WIFI_STA);

    // Add multiple WiFi APs
    configureWifiAccessPoints();
    configureAdditionalWifiAccessPoints();
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
    configTzTime("CST6CDT,M3.2.0,M11.1.0", ntpServer);
    initializeRecordingCalendar();
    SetLocalTime();
    updateRecordingCalendar();
    serviceCountState();
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
    // Connect after SD restoration so boot defaults are never advertised as totals.

    // After MQTT connect on Gate Counter: ensure alarm state is CLEAR and retained
    publishMQTT(MQTT_PUB_ALARM, "CLEAR", true);
    gateStuckAlarmActive = false;

    // (Optional / existing) publish initial beam states as retained
    publishMQTT(MQTT_PUB_BEAM_A_STATE, String(beamAState), true);
    publishMQTT(MQTT_PUB_BEAM_B_STATE, String(beamBState), true);

    // Existing verified detector polarity: raw HIGH = broken, LOW = clear.
    pinMode(beamAPin, INPUT);      // Beam A
    pinMode(beamBPin, INPUT);     // Beam B

    initializeMagneticDiagnostics();

    delay(5); // tiny settle time

    // Read raw beam levels
    int rawA = digitalRead(beamAPin);
    int rawB = digitalRead(beamBPin);

    // Map to logical: 1=blocked, 0=clear.
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
    checkAndCreateFile(fileName7, NightlyCsv::cumulative);
    checkAndCreateFile(fileName8);
    checkAndCreateFile(fileName9);
    checkAndCreateFile(fileName10, "ms,beamB_raw,beamA_raw");

    // Required Hourly Data Files
    createAndInitializeHourlyFile(fileName5);

    // Initialize Server
    setupServer();

    //on reboot, get totals saved on SD Card
    initializeOperatingSettings();
    getSavedValuesOnReboot();  // Update/reset counts based on reboot day
    daysRunning = recordingDaysRunning();
    MQTTreconnect();
    publishRecordingCalendar();

    // Setup MDNS
    if (!MDNS.begin(THIS_MQTT_CLIENT)) {
      Serial.println("Error starting mDNS");
      return;
    }
    delay(1000);
    start_MqttMillis = millis();
}

void loop() {
    detectCar(); // Beam sampling precedes all foreground maintenance.
    updateRecordingCalendar();
    // Retry durable saves and roll full dates even when no vehicle arrives.
    serviceCountState();
    detectCar();
    static uint32_t lastSdHealth=0;
    if(millis()-lastSdHealth>=60000UL){lastSdHealth=millis();checkSdHealth();detectCar();}
    DateTime now = rtc.now();

    currentTimeMinute = now.hour()*60 + now.minute(); // convert time to minutes since midnight

    showTime = officialShowNow();

    readTempandRH(); // Get Temperature and Humidity
    detectCar();

    ElegantOTA.loop();        // Keep OTA Updates Alive

    updateDisplay();
    detectCar();

    checkWiFiConnection();
    detectCar();

    timeTriggeredEvents();
    detectCar();

    detectCar();              // Detect cars
    serviceMagneticDiagnostics(); // Bounded diagnostic export after counting

    // Keep MQTT client alive and publish heartbeat on its own clock
    if (millis() - lastKeepAliveMillis >= (unsigned long)mqttKeepAlive * 1000UL) {
        KeepMqttAlive();
        lastKeepAliveMillis = millis();
    }
    serviceProductionSession();
    serviceTrafficJournal();
    detectCar();
}

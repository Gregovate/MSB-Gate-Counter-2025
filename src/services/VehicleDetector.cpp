#include "traffic/Firmware.h"

// Extracted from the production baseline; behavior fixes are separate changes.
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
        File logFile = recordingOpen(fileName10, FILE_APPEND);
        if (logFile) {
            logFile.printf("%lu,%d,%d\n", currentTime, beamBState, beamAState);
            logFile.close();
        } else {
            Serial.println("Failed to open sensorLog.csv");
        }
    }
}

void countTheCar() {
    serviceCountState(); // Network work may have crossed midnight since loop start.
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


    // Publish current hour's data to MQTT (NEW tree)
    // Final form: msb/traffic/GateCounter/Cars/hour/07
    char hourlyTopic[120];  // GAL 25-11-24: expanded for new bucketed paths
    snprintf(hourlyTopic, sizeof(hourlyTopic),
             "%s%02d", MQTT_PUB_CARS_HOURLY, currentHour);


    // increase Show Count only when show is open
    if (showTime == true) {
        totalShowCars++;
    }

    // Commit this vehicle as one dated daily/show/hourly checkpoint before
    // MQTT or the web-facing files can delay persistence.
    persistCountState();
    // Reporting continues if SD fails; countPersistence identifies durability.
    publishMQTT(MQTT_PUB_EXIT_CARS, String(totalDailyCars), true);
    publishMQTT(MQTT_PUB_SHOWTOTAL, String(totalShowCars), true);
    publishMQTT(hourlyTopic, String(hourlyCount[currentHour]), true);
    saveDailyTotal();
    saveShowTotal();
    saveHourlyCounts();

    inParkCars = carCounterCars - totalDailyCars;

    // open file for writing Car Data
    myFile = recordingOpen(fileName6, FILE_APPEND);
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


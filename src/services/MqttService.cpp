#include "traffic/Firmware.h"

// Extracted from the production baseline; behavior fixes are separate changes.


void publishMQTT(const char *topic, const String &message, bool retainFlag) {
    if (!countTopicPublishable(topic)) return;
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

void publishMQTT(const char *topic, const String &message) {
    publishMQTT(topic, message, false);
}







void KeepMqttAlive() {
    publishTimingConfig();
    publishHourlySnapshot();
    publishCountHealth();
    publishRecordingCalendar();

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

void publishHello(const char* status, const char* msg, bool retainFlag) {
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

void debugInPark(const char* reason) {
    String msg = String(reason) +
                 " Enter=" + String(carCounterCars) +
                 " Exit="  + String(totalDailyCars) +
                 " InPark=" + String(inParkCars);
    publishMQTT(MQTT_DEBUG_LOG, msg);
}

bool topicIs(const char* t, const char* target) {
  return strcmp(t, target) == 0;
}

void publishStateExitInPark() {
  publishMQTT(MQTT_PUB_EXIT_CARS,   String(totalDailyCars), true);
  publishMQTT(MQTT_PUB_INPARK_CARS, String(inParkCars),     true);
}

void publishTimingConfig() {
    // Commands remain HA-owned; expose the actual applied values separately.
    publishConfigState(MQTT_SUB_GATE_TIMEOUT, String(gateCounterTimeout));
    publishConfigState(MQTT_SUB_CARMS, String(carDetectMS));
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
      acceptShowStartDate(String(message));
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
      daysRunning = recordingDaysRunning();
      publishMQTT(MQTT_PUB_DAYSRUNNING, String(daysRunning), true);
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
    publishTimingConfig();
    return;
  }

  if (topicIs(topic, MQTT_SUB_CARMS)) {
    carDetectMS = atoi(message);
    Serial.print(F(" Gate Counter carDetectMS Updated (ms): "));
    Serial.println(carDetectMS);

    // DO NOT echo this topic back to MQTT – HA owns this value.
    // publishMQTT(MQTT_SUB_CARMS, String(carDetectMS), true);

    publishDebugLog("Gate Counter carDetectMS Updated");
    publishTimingConfig();
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


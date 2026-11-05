#include "traffic/Firmware.h"

// Extracted from the production baseline; behavior fixes are separate changes.


void publishMQTT(const char *topic, const String &message, bool retainFlag) {
    if (!countTopicPublishable(topic) || !mqttOnline()) return;
    mqtt_client.publish(topic,message.c_str(),retainFlag);
    start_MqttMillis=millis();
}

void publishMQTT(const char *topic, const String &message) {
    publishMQTT(topic, message, false);
}







void KeepMqttAlive() {
    inParkCars=carCounterCars-totalDailyCars;
    publishTimingConfig();
    publishOperatingSettings();
    publishMQTT(MQTT_PUB_ALARM, gateStuckAlarmActive ? "ALARM_GATE_STUCK" : "CLEAR", true);
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

bool parseCountCorrection(const char* p,uint32_t& value) {
    if (!*p) return false;
    uint32_t n=0;
    while (*p) {
        if (*p<'0' || *p>'9' || n>(2147483647UL-(*p-'0'))/10) return false;
        n=n*10+(*p++-'0');
    }
    value=n;return true;
}

void callback(char* topic, byte* payload, unsigned int length) {
    // Bound stack use and reject embedded NULs/truncated command payloads.
    if (length>180) return;
    char message[181];memcpy(message,payload,length);message[length]='\0';
    if (strlen(message)!=length) return;
    if (handleOperatingCommand(topic,message)) return;
    String command(topic);
    if (command==String(topic_base_path)+"/Config/showStartDate") {
        acceptShowStartDate(String(message));return;
    }
    if (topicIs(topic,MQTT_SUB_CC_ENTER_TOTAL)) {
        uint32_t count=0;if (!parseCountCorrection(message,count)) return;
        carCounterCars=count;inParkCars=carCounterCars-totalDailyCars;
        publishStateExitInPark();return;
    }
    if (topicIs(topic,MQTT_SUB_LOGGING)) {
        if (strcmp(message,"1")==0) loggingEnabled=true;
        else if (strcmp(message,"0")==0) loggingEnabled=false;
        return;
    }
    // Old retained resets, calendar mirrors and individual timing commands
    // are deliberately inert. RTC owns the date; HA owns the full settings transaction.
}

void applyOperatingDetector(int alarmMs,int detectMs) {
    gateCounterTimeout=alarmMs;carDetectMS=detectMs;
}

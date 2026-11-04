#include "traffic/Firmware.h"
#include "secrets.h"

// Extracted from the production baseline; behavior fixes are separate changes.
// Register the same five configured APs at the original setup position.
void configureWifiAccessPoints() {
    wifiMulti.addAP(secret_ssid_AP_1, secret_pass_AP_1);
    wifiMulti.addAP(secret_ssid_AP_2, secret_pass_AP_2);
    wifiMulti.addAP(secret_ssid_AP_3, secret_pass_AP_3);
    wifiMulti.addAP(secret_ssid_AP_4, secret_pass_AP_4);
    wifiMulti.addAP(secret_ssid_AP_5, secret_pass_AP_5);
}

void setup_wifi() {
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
}

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

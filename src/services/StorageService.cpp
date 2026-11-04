#include "traffic/Firmware.h"

// Extracted from the production baseline; behavior fixes are separate changes.
String sPath(const char *fname) {
    return seasonFolder + "/" + String(fname);
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

void checkAndCreateFile(const String &fileName, const String &header) {
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

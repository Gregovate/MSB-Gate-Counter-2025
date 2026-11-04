#include "traffic/Firmware.h"

// Extracted from the production baseline; behavior fixes are separate changes.




int determineSeasonYear(const DateTime &now) {
    int y = now.year();
    int m = now.month();
    if (m >= 11) return y;     // Nov/Dec = current year season
    return y - 1;             // Jan–Oct belongs to previous season
}

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

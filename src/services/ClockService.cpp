#include "traffic/Firmware.h"

// Extracted from the production baseline; behavior fixes are separate changes.




int determineSeasonYear(const DateTime &now) {
    (void)now;
    return recordingYear();
}

int computeDaysRunningFromStart() {
    return recordingDaysRunning();
}



void timeTriggeredEvents() {
    // One shared HA-configured scheduler, independent of MQTT availability.
    serviceShowSchedule();
}


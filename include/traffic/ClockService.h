#pragma once
#include "RuntimeState.h"

// Service interface; definitions are compiled once in src/services.
String getRtcTimestamp();
void SetLocalTime();
int determineSeasonYear(const DateTime &now);
int computeDaysRunningFromStart();
void resetHourlyCounts();
void timeTriggeredEvents();

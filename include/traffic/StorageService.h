#pragma once
#include "RuntimeState.h"

// Service interface; definitions are compiled once in src/services.
String sPath(const char *fname);
void ensureSeasonFolderExists(int seasonYear);
void initSeasonalPaths();
void saveShowStartDate();
void getShowStartDate();
void getDailyTotal();
void getShowTotal();
void getDayOfMonth();
void getDaysRunning();
void getHourlyData();
void saveDailyTotal();
void saveShowTotal();
void saveDayOfMonth();
void saveDaysRunning();
void saveHourlyCounts();
void saveDailyShowSummary();
void getSavedValuesOnReboot();
void checkAndCreateFile(const String &fileName, const String &header = "");
void createAndInitializeHourlyFile(const String &fileName);
void initSDCard();

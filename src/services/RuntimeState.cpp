#include "traffic/RuntimeState.h"

// Single instances retain the original initialization values and ordering.
String showStartDate = ""; // "YYYY-MM-DD"
bool showStartDateValid = false; // true once parsed
int showStartYear = 0;
int showStartMonth = 0;
int showStartDay = 0;
const int showStartMin = 17 * 60 + 10; // 5:10 PM in minutes
const int showEndMin = 21 * 60 + 20; // 9:20 PM in minutes (including additional checks till 9:20 PM)
bool rtcReady = false; // GAL 25-11-22: RTC boot-safety guard
String bootTimestamp = ""; // GAL 25-11-22: Store boot timestamp for logging
int mqttKeepAlive = 30; // publish select values every x seconds to keep MQTT client connected
unsigned long lastKeepAliveMillis = 0;
char topic[60];
char topicBase[60];
const unsigned long debounceDelay = 50; // ms; adjust 30–80 if needed
bool stableRawA = HIGH; // stable debounced raw for Beam A
bool stableRawB = HIGH; // stable debounced raw for Beam B
unsigned long lastRawAChangeMs = 0;
unsigned long lastRawBChangeMs = 0;
GateDetectState gateDetectState = WAITING_FOR_CAR;
unsigned long beamATripTime_ms = 0; // when Beam A first broke
unsigned long bothBeamsBroken_ms = 0; // when both beams confirmed broken
unsigned long lastCarDetected_ms = 0; // when last car was detected
unsigned long firstBeamHealth_ms = 0; // for Beam A
unsigned long secondBeamHealth_ms = 0; // for Beam B
unsigned long timeBetweenCars_ms = 0; // time between cars for ExitLog.csv
bool carPresentFlag = false;
bool gateStuckAlarmActive = false;
unsigned long abFollow_ms = 0; // A to B follow time
unsigned long timeToPassMS = 0; // Time from start of detection to confirmation
bool abTimeoutLogged = false;
const unsigned long minActivationDuration = 150; // ignore tiny blips
const unsigned long maxABFollow_ms = 900;
unsigned long carDetectMS = 1000; // Minimum wait duration for a vehicle to be confirmed
unsigned long gateCounterTimeout = 60000; // default time for car counter alarm in millis
AsyncWebServer server(80); // Define Webserver
String currentDirectory = "/"; // Current working directory
unsigned long ota_progress_millis = 0;
RTC_DS3231 rtc;
const char* ampm ="AM";
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -21600;
const int daylightOffset_sec = 3600;
DHT dht(DHTPIN, DHTTYPE);
float tempF = -998.0; // Temperature
float humidity = -998.0; // Humidity
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
const int line1 =0;
const int line2 =9;
const int line3 = 19;
const int line4 = 30;
const int line5 = 42;
const int line6 = 50;
const int line7 = 53;
WiFiMulti wifiMulti;
WiFiClient espGateCounter;
uint16_t connectTimeOutPerAP=5000;
PubSubClient mqtt_client(espGateCounter);
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
bool loggingEnabled = false; // Default: Logging is OFF
bool mqtt_connected = false;
bool wifi_connected = false;
int wifi_connect_attempts = 5;
bool hasRun = false;
unsigned int dayOfMonth; // Current Calendar day
unsigned int lastDayOfMonth; // Last calendar day used to reset days running
unsigned int currentHr12; // Current Hour 12 Hour format
unsigned int currentHr24; // Current Hour 24 Hour Format
unsigned int currentMin; // Current Minute
unsigned int currentSec; // Current Second
unsigned int daysRunning; // Number of days the show is running.
unsigned int currentTimeMinute; // for converting clock time hh:mm to clock time min since midnight
int totalDailyCars; // total cars counted per day 24/7 Needed for debugging
int totalShowCars; // total cars counted for durning show hours open (5:00 pm to 9:10 pm)
int inParkCars; // cars in park Enter Cars - Exit Cars
int carCounterCars; // Counts from Car Counter
int lastcarCounterCars; // Used to publish in park cars when car counter increases
int beamAState = 0;
int lastBeamAState = -1;
int beamBState = 0;
int lastBeamBState = -1;
unsigned long triggerTime; // Stores the time when sensor 1 is triggered
unsigned long beamSensorAlarm; // Monitor time Beam Sensor is blocked
const unsigned long wifi_connectioncheckMillis = 5000; // check for connection every 5 sec
const unsigned long mqtt_connectionCheckMillis = 30000; // check for connection
unsigned long start_MqttMillis; // for Keep Alive Timer
unsigned long start_WiFiMillis; // for keep Alive Timer
char buf2[25] = "YYYY-MM-DD hh:mm:ss"; // time car detected
bool flagDaysRunningReset = false;
bool flagMidnightReset = false;
bool flagDailyShowStartReset = false;
bool flagDailySummarySaved = false;
bool flagDailyShowSummarySaved = false;
bool flagHourlyReset = false;
bool showTime = false;
bool resetFlagsOnce = false;
File myFile; // global handle for SD file ops
String seasonFolder = ""; // Example: "/GC/2025"
String fileName1; // ExitTotal.txt       (daily total)
String fileName2; // ShowTotal.txt       (season total)
String fileName3; // DayOfMonth.txt
String fileName4; // RunDays.txt
String fileName5; // GateHourlyData.csv
String fileName6; // GateLog.csv
String fileName7; // GateDailySummary.csv
String fileName8; // data/index.html     (OTA / UI)
String fileName9; // data/style.css
String fileName10; // SensorLog.csv
String fileNameShowStart; // ShowStart.txt   (opening date)
unsigned int hourlyCount[24] = {0}; // Array for Daily total cars per hour
float hourlyTemp[24] = {0.0}; // Array to store average temperatures for 24 hours
char days[7][4] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
char months[12][4] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
std::queue<String> publishQueue;
const size_t MAX_QUEUE = 3000;
const size_t MAX_FLUSH_PER_CALL = 150; // prevents WDT resets

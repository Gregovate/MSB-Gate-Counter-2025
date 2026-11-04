# MQTT and Home Assistant — existing contract and gaps

Status: source-derived review draft. Preserve existing topics until HA mappings and effective broker state are reconciled. This document is not permission to send commands.

## Integration chain

CarCounter produces entry totals; GateCounter consumes EnterTotal and derives occupancy as entries minus exits. Park HA consumes the raw counts and derives rates, occupancy bands, travel estimates and operational advisories. The exact deployed broker address/credentials and HA base MQTT mappings were not read. Do not infer those from a topic or entity name.

Car uses a single mqtt_Server configuration (C:1650). Gate iterates mqtt_configs with potentially multiple brokers (G:1382). Switching brokers does not replicate retained state between them. Record which park broker is authoritative before standardizing reconnection behavior. Both use client IDs tied to espCarCounter/espGateCounter; do not run a test instance using a production client identity.

## Payloads and ownership

| Category | Existing wire behavior | Compatibility requirement |
|---|---|---|
| EnterTotal / ExitTotal | Plain decimal integer string; mutable by reset/assignment commands | Not JSON, not an event stream, not monotonic lifetime count; includes programmed resets |
| ShowTotal | Plain integer cumulative season count incremented within each MCU's own show window | Do not present as tonight's count; entry and exit windows differ |
| InParkCars | Plain signed integer derived on Gate from received EnterTotal minus local daily exits | Negative values are possible; staleness and unmatched reset epochs must be visible rather than silently clamped in firmware |
| Hour buckets | Integer strings. Car uses Cars/hour/HH; actual Gate dynamic construction uses Cars/hour//HH | Exact slash and case matter. Do not silently normalize during extraction |
| Car hour/json | Retained 24-element JSON array in heartbeat | Retain array order00–23. Gate declares equivalent macro but no active publish site was found |
| System/heartbeat | Retained JSON: boot, now, rssi; Car enter/show, Gate exit/inpark | Retained does not mean currently online; preserve keys and add validity/freshness only through reviewed compatibility plan |
| Env | JSON keys tempF and humidity. Car Env/temp; Gate Env/tempHumidity | Fahrenheit, sentinel failures -999 and startup sentinels exist. Do not treat them as valid temperatures |
| Beam states | Retained decimal0/1; active detectors treat1 as broken | Preserve mapping; verify electrical polarity on bench because comments conflict |
| Timings | Decimal milliseconds; AB, B-broken, TTP, between cars | Measurement points differ between algorithms; do not merge field meaning merely because names match |
| Alarm | Retained text CLEAR, ALARM_ENTER_STUCK or ALARM_GATE_STUCK | Existing boot CLEAR is not evidence that physical inputs have been checked |
| HELLO/debug | HELLO online JSON; debug topic mixes text and JSON, some calls retained | No last-will argument in inspected connect calls. Establish offline/expiry policy in HA rather than trusting retained online alone |
| Summary | Car Calendar/showSummary is non-retained human-readable text in fixed160-byte buffer; Gate Calendar/Summary retained human-readable text | Not a durable nightly ledger or guaranteed complete JSON. SD write failure can suppress summary publishing |

Retain flags are call-site dependent, not a property of a macro: Car event and many save/get publications are non-retained, heartbeat makes core totals retained every30 seconds when totalsInitialized. Gate save/get/heartbeat totals are retained. Do not globally change wrapper default retention; debug/events and state need separate policy.

## Commands actually handled

All topics below begin `msb/traffic/`; names and case are literal. Reset commands assign the supplied value; they are not limited to zero. Existing callbacks mostly use atoi, so malformed input may become0. Existing documentation's `{"payload":0}` examples describe an envelope, not the literal payload the firmware parses.

| Device | Topic suffix | Current handling |
|---|---|---|
| Car | CarCounter/Config/resetDailyCount | Assign daily entries and save |
| Car | CarCounter/Config/resetShowCount | Assign season total and save |
| Car | CarCounter/Config/resetDayOfMonth | Assign day marker and save |
| Car | CarCounter/Config/resetDaysRunning | Assign running-day counter and save |
| Car | CarCounter/Config/carCounterTimeout | Parse unsigned ms, clamp5000–600000 |
| Car | CarCounter/Config/beamWaitDuration | Subscribed and publishes default, but callback branch is commented out; cannot tune detector through this command in assessed release |
| Car | CarCounter/Config/showStartDate | Parse year/month/day with loose ranges; update folder and running days; idempotency guard commented out; not a full calendar-date validator |
| Car | CarCounter/Config/showStartTime | Parse HH:MM, no hour/minute range checks; RAM setting |
| Car | CarCounter/Config/showEndTime | Same; RAM setting |
| Gate | GateCounter/Config/resetDailyCount | Assign exits and save/publish, but no occupancy recalculation in handler |
| Gate | GateCounter/Config/resetShowCount | Assign season exits and save/publish |
| Gate | GateCounter/Config/resetDayOfMonth | Assign/save/publish day marker |
| Gate | GateCounter/Config/resetDaysRunning | Assign/save/publish running days |
| Gate | GateCounter/Config/gateCounterTimeout | atoi ms; no bounds |
| Gate | GateCounter/Config/carDetectMS | atoi ms; no bounds; comments explicitly state HA owns value |
| Gate | GateCounter/Config/loggingEnabled | Exact "1"/"0" toggles flag; sensor logging function has no caller |

Gate also subscribes to Car EnterTotal, ShowTotal, showStartDate, dayOfMonth and daysRunning. EnterTotal updates occupancy; calendar callbacks save and republish under Gate names; showStartDate only stores string and checks length; ShowTotal has no handler. Car subscribes to own HELLO but has no matching handler.

Neither current detector exposes a Gate seasonal mode command. Do not invent one in HA without the corresponding approved firmware design.

At boot both publish timing defaults retained to subscribed config topics. Car heartbeat publishes showStartDate to its own subscribed topic; callback recomputes/saves running days, including for unchanged dates. Gate default publication can compete with HA-owned tuning. Proposed contract: HA desired configuration and device effective configuration must be distinguishable; application validates and acknowledges changes. Exact migration topic names remain a design decision after inspecting base HA mappings. Reset actions should not be replayed as retained boot commands.

## HA evidence reviewed

Source: [traffic_analytics.yaml](https://chatgpt.com/library?file_id=libfile_92c60620bc588191b0d1be028ebdab32&account_id=personal), version1,615 lines. Do not include HA credentials in this documentation.

| Dependency | Verified file behavior |
|---|---|
| Raw entries/exits | Derivative sensors source sensor.msb_car_counter_enter_cars and sensor.msb_gate_counter_exit_cars; five-minute time window (lines19–35) |
| Occupancy | Canonical template reads sensor.msb_gate_counter_in_park_cars; this file does not recompute from entries/exits (around337–347) |
| Public opening | msb_show_open checks external msb_show_season plus start/end input_datetime; guest hours distinct from analytics windows |
| Analytics windows | Entry starts at show opening; exit starts after gate cushion default8min; both end after car cushion default15min. These two templates do not themselves test season_on |
| Firmware divergence | Car defaults17:00–21:10 with HA time commands; Gate hard-codes17:10–21:20, reset17:08. HA windows must not be assumed identical to either |
| Derived status | Entry/exit cars/min, load band, travel-time estimate, traffic pressure, flow status, lane2 and merger advisories |
| Export | Traffic status snapshot and JSON payload for website, including now()-based last_updated |

Integration findings requiring review, not configuration changes in this work:

1. The Time Between Cars (ms) - Valid template reads `sensor.msb_car_counter_time_between_cars_ms_valid` within its own state expression. This is a self-reference if that is the actual registered entity; verify registry identity before correcting to raw source.
2. `sensor.msb_flow_smoothed` is consumed but not defined in this package. It may be defined elsewhere; do not mark it missing from the whole installation.
3. Many templates convert unavailable raw values to0. A stale retained count can appear plausible; last_updated=now() timestamps template rendering, not counter freshness. Snapshot validity should derive from device heartbeats and source ages.
4. Derivatives of resettable daily totals need explicit reset/bad-data semantics. Clipping negative rates to0 does not by itself establish correct flow after resets or reconnects.
5. Public show-open has a season guard; analytics windows do not. Preserve observed behavior until all owning templates/automations are read and desired off-season behavior is agreed.

## Evidence still required for complete deployed contract

- HA base MQTT definitions mapping each state/command topic to entity IDs, availability, expire_after and value templates.
- Automations that set dates/times/tuning, reset counts, capture nightly results and distribute reports; helper definitions for season/cushions.
- Effective retained messages on the authoritative park broker and actual HA entity registry IDs.
- Historical-table dashboard mechanism: REST scrape, iframe, file-backed report, recorder or another path. Operator sees the same results in HA, but this analytics package alone does not establish durable historical storage.
- Broker persistence/retention and HA recorder retention, backup/restore boundaries, clock timezone settings.

These are completion gates for integration implementation, not blockers to source assessment. No public ESP32 access, broker access or new HA service was enabled.

## Literal source topic inventory

The following generated tables enumerate macro definitions, not guaranteed active publishes. The callback table above identifies handled commands. Duplicate PUB/SUB macros for the same topic are intentional evidence of existing overlap.

### C literal topic macros

| Macro | Literal topic | Source line |
|---|---|---|
| `MQTT_PUB_HELLO` | `msb/traffic/CarCounter/System/hello` | 385 |
| `MQTT_PUB_FW_VERSION` | `msb/traffic/CarCounter/System/firmware` | 386 |
| `MQTT_PUB_TIME` | `msb/traffic/CarCounter/System/time` | 387 |
| `MQTT_DEBUG_LOG` | `msb/traffic/CarCounter/System/debug` | 388 |
| `MQTT_PUB_HEARTBEAT` | `msb/traffic/CarCounter/System/heartbeat` | 389 |
| `MQTT_PUB_SEASON_FOLDER` | `msb/traffic/CarCounter/System/seasonFolder` | 392 |
| `MQTT_PUB_SEASON_YEAR` | `msb/traffic/CarCounter/System/seasonYear` | 393 |
| `MQTT_PUB_TEMP` | `msb/traffic/CarCounter/Env/temp` | 396 |
| `MQTT_PUB_ENTER_CARS` | `msb/traffic/CarCounter/Cars/EnterTotal` | 399 |
| `MQTT_PUB_SHOWTOTAL` | `msb/traffic/CarCounter/Cars/ShowTotal` | 400 |
| `MQTT_PUB_CARS_HOURLY` | `msb/traffic/CarCounter/Cars/hour/` | 401 |
| `MQTT_PUB_HOURLY_JSON` | `msb/traffic/CarCounter/Cars/hour/json` | 402 |
| `MQTT_PUB_BETWEENCARS_MS` | `msb/traffic/CarCounter/Cars/timeBetweenCars` | 403 |
| `MQTT_PUB_DAYOFMONTH` | `msb/traffic/CarCounter/Calendar/dayOfMonth` | 406 |
| `MQTT_PUB_DAYSRUNNING` | `msb/traffic/CarCounter/Calendar/daysRunning` | 407 |
| `MQTT_PUB_SUMMARY` | `msb/traffic/CarCounter/Calendar/showSummary` | 408 |
| `MQTT_PUB_BEAM_A_STATE` | `msb/traffic/CarCounter/Sensors/beam1State` | 411 |
| `MQTT_PUB_BEAM_B_STATE` | `msb/traffic/CarCounter/Sensors/beam2State` | 412 |
| `MQTT_PUB_BEAM_AB_MS` | `msb/traffic/CarCounter/Sensors/beamAB_ms` | 413 |
| `MQTT_PUB_BEAM_B_BROKEN_MS` | `msb/traffic/CarCounter/Sensors/beamB_broken_ms` | 414 |
| `MQTT_PUB_TTP` | `msb/traffic/CarCounter/Sensors/TTP` | 415 |
| `MQTT_COUNTER_LOG` | `msb/traffic/CarCounter/Sensors/CounterLog` | 416 |
| `MQTT_PUB_ALARM` | `msb/traffic/CarCounter/Sensors/Alarm` | 417 |
| `MQTT_PUB_RSSI` | `msb/traffic/CarCounter/System/wifi/rssi` | 420 |
| `MQTT_PUB_SSID` | `msb/traffic/CarCounter/System/wifi/ssid` | 421 |
| `MQTT_PUB_IP` | `msb/traffic/CarCounter/System/wifi/ip` | 422 |
| `MQTT_PUB_SD_STATUS` | `msb/traffic/CarCounter/System/sd/status` | 427 |
| `MQTT_PUB_SD_DIAG` | `msb/traffic/CarCounter/System/sd/diag` | 428 |
| `MQTT_PUB_SD_HEALTH` | `msb/traffic/CarCounter/System/sd/health` | 429 |
| `MQTT_PUB_SHOW_START_DATE` | `msb/traffic/CarCounter/Config/showStartDate` | 432 |
| `MQTT_PUB_SHOW_START_MIN` | `msb/traffic/CarCounter/Config/showStartMin` | 433 |
| `MQTT_PUB_SHOW_END_MIN` | `msb/traffic/CarCounter/Config/showEndMin` | 434 |
| `MQTT_SUB_SHOWSTART` | `msb/traffic/CarCounter/Config/showStartDate` | 440 |
| `MQTT_SUB_SHOWSTARTTIME` | `msb/traffic/CarCounter/Config/showStartTime` | 441 |
| `MQTT_SUB_SHOWENDTIME` | `msb/traffic/CarCounter/Config/showEndTime` | 442 |
| `MQTT_SUB_TOPIC1` | `msb/traffic/CarCounter/Config/resetDailyCount` | 445 |
| `MQTT_SUB_TOPIC2` | `msb/traffic/CarCounter/Config/resetShowCount` | 446 |
| `MQTT_SUB_TOPIC3` | `msb/traffic/CarCounter/Config/resetDayOfMonth` | 447 |
| `MQTT_SUB_TOPIC4` | `msb/traffic/CarCounter/Config/resetDaysRunning` | 448 |
| `MQTT_SUB_TOPIC5` | `msb/traffic/CarCounter/Config/carCounterTimeout` | 451 |
| `MQTT_SUB_TOPIC6` | `msb/traffic/CarCounter/Config/beamWaitDuration` | 452 |

### G literal topic macros

| Macro | Literal topic | Source line |
|---|---|---|
| `MQTT_PUB_HELLO` | `msb/traffic/GateCounter/System/hello` | 362 |
| `MQTT_PUB_FIRMWARE` | `msb/traffic/GateCounter/System/firmware` | 363 |
| `MQTT_PUB_TIME` | `msb/traffic/GateCounter/System/time` | 364 |
| `MQTT_DEBUG_LOG` | `msb/traffic/GateCounter/System/debug` | 365 |
| `MQTT_PUB_HEARTBEAT` | `msb/traffic/GateCounter/System/heartbeat` | 366 |
| `MQTT_PUB_SEASON_FOLDER` | `msb/traffic/GateCounter/System/seasonFolder` | 369 |
| `MQTT_PUB_SEASON_YEAR` | `msb/traffic/GateCounter/System/seasonYear` | 370 |
| `MQTT_PUB_WIFI_SSID` | `msb/traffic/GateCounter/System/wifi/ssid` | 373 |
| `MQTT_PUB_WIFI_RSSI` | `msb/traffic/GateCounter/System/wifi/rssi` | 374 |
| `MQTT_PUB_WIFI_IP` | `msb/traffic/GateCounter/System/wifi/ip` | 375 |
| `MQTT_PUB_TEMP` | `msb/traffic/GateCounter/Env/tempHumidity` | 379 |
| `MQTT_PUB_EXIT_CARS` | `msb/traffic/GateCounter/Cars/ExitTotal` | 382 |
| `MQTT_PUB_INPARK_CARS` | `msb/traffic/GateCounter/Cars/InParkCars` | 383 |
| `MQTT_PUB_SHOWTOTAL` | `msb/traffic/GateCounter/Cars/ShowTotal` | 384 |
| `MQTT_PUB_CARS_HOURLY` | `msb/traffic/GateCounter/Cars/hour/` | 385 |
| `MQTT_PUB_HOURLY_JSON` | `msb/traffic/GateCounter/Cars/hour/json` | 386 |
| `MQTT_PUB_BETWEENCARS_MS` | `msb/traffic/GateCounter/Cars/timeBetweenCars` | 387 |
| `MQTT_PUB_DAYOFMONTH` | `msb/traffic/GateCounter/Calendar/DayOfMonth` | 390 |
| `MQTT_PUB_DAYSRUNNING` | `msb/traffic/GateCounter/Calendar/DaysRunning` | 391 |
| `MQTT_PUB_SUMMARY` | `msb/traffic/GateCounter/Calendar/Summary` | 392 |
| `MQTT_PUB_BEAM_A_STATE` | `msb/traffic/GateCounter/Sensors/beamAState` | 395 |
| `MQTT_PUB_BEAM_B_STATE` | `msb/traffic/GateCounter/Sensors/beamBState` | 396 |
| `MQTT_PUB_BEAM_AB_MS` | `msb/traffic/GateCounter/Sensors/beamAB_ms` | 397 |
| `MQTT_PUB_BEAM_B_BROKEN_MS` | `msb/traffic/GateCounter/Sensors/beamB_broken_ms` | 398 |
| `MQTT_PUB_TTP` | `msb/traffic/GateCounter/Sensors/TTP` | 399 |
| `MQTT_COUNTER_LOG` | `msb/traffic/GateCounter/Sensors/CounterLog` | 400 |
| `MQTT_PUB_ALARM` | `msb/traffic/GateCounter/Sensors/Alarm` | 401 |
| `MQTT_SUB_GATE_RESET_DAILY` | `msb/traffic/GateCounter/Config/resetDailyCount` | 404 |
| `MQTT_SUB_GATE_RESET_SHOW` | `msb/traffic/GateCounter/Config/resetShowCount` | 405 |
| `MQTT_SUB_GATE_RESET_DOM` | `msb/traffic/GateCounter/Config/resetDayOfMonth` | 406 |
| `MQTT_SUB_GATE_RESET_DAYS` | `msb/traffic/GateCounter/Config/resetDaysRunning` | 407 |
| `MQTT_SUB_GATE_TIMEOUT` | `msb/traffic/GateCounter/Config/gateCounterTimeout` | 408 |
| `MQTT_SUB_CARMS` | `msb/traffic/GateCounter/Config/carDetectMS` | 409 |
| `MQTT_SUB_LOGGING` | `msb/traffic/GateCounter/Config/loggingEnabled` | 410 |
| `MQTT_SUB_CC_DAY_OF_MONTH` | `msb/traffic/CarCounter/Calendar/dayOfMonth` | 411 |
| `MQTT_SUB_CC_DAYS_RUNNING` | `msb/traffic/CarCounter/Calendar/daysRunning` | 412 |
| `MQTT_SUB_CC_ENTER_TOTAL` | `msb/traffic/CarCounter/Cars/EnterTotal` | 416 |
| `MQTT_SUB_CC_SHOW_TOTAL` | `msb/traffic/CarCounter/Cars/ShowTotal` | 417 |
| `MQTT_SUB_CC_SHOW_START_DATE` | `msb/traffic/CarCounter/Config/showStartDate` | 419 |

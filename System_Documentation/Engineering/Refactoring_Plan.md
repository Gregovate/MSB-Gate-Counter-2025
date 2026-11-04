# Refactoring design and verification plan

Status: PROPOSED, awaiting design review. This plan coordinates both devices and both historical Gate sensing implementations. No firmware implementation is included in the documentation PR.

## Responsibilities

| Module | Owns | Must not do |
|---|---|---|
| DeviceProfile | Identity, role, pins, electrical polarity, arch capabilities, supported detector policies | Change global pins through scattered conditionals |
| SensorSampler | Raw inputs, normalization, debounce, monotonic observation timestamps | SD, MQTT, Wi-Fi, display or wall-clock work |
| VehicleDetector | Pure per-device algorithm state, transition/rejection events, completed passage | Mutate totals or depend on transport |
| CountManager | Apply completed event once, daily/hourly/season totals, manual assignments and derived occupancy | Publish partially updated state |
| ClockService | Wall-clock validity, RTC/NTP/timezone, full date/season identity | Control elapsed detection timing |
| ScheduleService | Show windows, rollover and summary action keys | Rely on hitting one exact second |
| StorageService | One owner for SD, checkpoint+journal/export results and recovery validity | Read/rewrite full season table in sensor path |
| MqttService | Existing topic adapters, validation, state coalescing, bounded event queue, reconnect progression | Write SD or execute reset side effects directly inside callback |
| DiagnosticService | Consistent event format, local trace, counters, health and timing budgets | Make sensing depend on broker availability |
| Web/OtaService | Per-device private file/UI operations and ElegantOTA lifecycle | Let global browsing directory decide another request's path |
| Display/ArchService | Scheduled OLED and single-owner arch output policy | Compete with detection through independent writes to the same outputs |

Use `.h` for declarations and types, `.cpp` for implementations. Keep main.cpp as composition/setup and short scheduler calls. Keep existing `/data` HTML/CSS assets as explicit release artifacts. Do not activate the example headers in include/ merely to claim modularity; sdfilesystem.h contains its own setup/loop and is not a production service.

Retain separately named CarDualBeam, GateDualBeam and HistoricalGateMagBeam policies initially. A future Gate mode selector must use validated hardware profile/wiring evidence. Shared sampling, typed events and diagnostics can be identical even while decision rules differ. Small predictable modules are preferred to a plugin framework or a large inheritance hierarchy.

## State and storage contract to settle

Each snapshot needs device identity, boot/session ID, full date, season, active mode, sequence number, count values and validity. Track current in-memory and persisted sequence numbers separately. A successful local event application is not equivalent to SD persistence, successful client publish or HA receipt.

Define recovery precedence before coding: validated local checkpoint/journal for device-owned counts; approved reconciliation if local recovery is unavailable; retained MQTT is not accepted blindly. Avoid both zero publication before restore and choosing max(local,retained), which would defeat intentional resets. Handle manual correction through an explicit record with date/epoch and reason.

Use a compact current-day checkpoint and append-only events/exports rather than rebuilding season CSV for every checkpoint. Power-failure recovery must be validated on actual SD/FAT behavior; a rename alone is not proof of power-loss atomicity. Define checkpoint interval and maximum acceptable count loss explicitly; the Car currently defers totals up to its nominal30-second heartbeat while logging events per car. Do not improve latency by silently expanding this loss window.

Persist daily action identity and completion result; summary output should be idempotent by device/season/date/report type and retry after failure without duplicating rows. Preserve existing CSV exports via adapters until approved migration. Only acknowledged persistence can clear dirty state.

## Stages and review gates

| Stage | Deliverable | Acceptance |
|---|---|---|
| 0 — baseline and build | Source/binary/UI/dependency manifest; verified reproducible dev build; actual Gate wiring and HA contract evidence | Baseline version+commit distinct; no credentials in artifacts; local ElegantOTAPro dependency located/licensed; existing binaries retained |
| 1 — characterization | Pure test harness around existing decision logic; synthetic and recorded traces; timing/ownership expectations documented | Same trace yields same count decisions/state progression as captured baseline, including known odd behavior. Characterization is not proof that behavior is correct |
| 2 — structural extraction | One firmware first (recommend Gate show branch for clearer code), declaration/implementation modules; matching Car modules next | No topic, retention, threshold, CSV, schedule or detector-policy change hidden in moves; private web and ElegantOTA exercised |
| 3 — diagnostics and responsiveness | Input-gap/service metrics, scheduled OLED, bounded network retry and acquisition isolation as needed | Measured budgets under AP loss, broker loss, SD delay and file browsing; no missed expected vehicle events |
| 4 — data correctness | Separately reviewed fixes for startup/rollover, checkpoint outcomes, event timing, queue draining and idempotent summaries | Reboot/fault/correction tests pass; HA comparison uses current mappings; no unreviewed format/topic migration |
| 5 — common library | Extract actually identical services, version and pin dependencies in both applications | Both application builds/tests run against same library revision; no floating branch dependency; no manual copy-back workflow |
| 6 — Gate mode unification | Explicit selector, wiring profile, effective-mode diagnostics and persistence | Both physical sensing arrangements characterized; safe transition with no in-flight count/reset; reboot/off-season works without Car present |
| 7 — controlled pilot | Bench then limited park validation, recoverable OTA release and evidence | Operator runbook, backup/restore and exact firmware/UI manifest reviewed; operator approves production deployment |

Stages3/4 may be divided into smaller defect PRs by consequence. They are not authorization to deploy. Do not merge the dual-beam feature branch into main merely to add documentation; choose its baseline deliberately when implementation begins.

## Required verification scenarios

| Group | Scenarios and expected evidence |
|---|---|
| Normal/aborted | A then B passage; A clears early; B-first; simultaneous start; short blips; A clear while B high; one event causes one count |
| Heavy traffic | B follows after900ms, several seconds and longer; stopped vehicle; closely spaced cars with short clear gaps; no hidden timeout reset; baseline discrepancy recorded |
| Timing | Threshold-minus/at/plus boundary; debounce behavior; millis wrap; delayed polling; first car has undefined interval, not fabricated vehicle interval |
| Sensor health | A stuck idle, B stuck idle, A-only stuck, both stuck; actual recovery; alarm latching and boot seeded from hardware |
| Networking | No AP at boot; loss during passage; primary AP reappears; broker socket stalls; repeated reconnect; backlog over150 and over3000; publish failure; retained freshness |
| Storage | No card, slow card, open failure, short write, removal, power cut during checkpoint; future restore complete/current/degraded explicitly known |
| Dates/config | Same/new full date reboot, same day number different month, hour-boundary checkpoint,23:59/00:00, missed flag second, reboot during summary minute, Christmas Eve, season boundary, timezone transition, invalid date/time/settings |
| HA commands | Assigned count recalculates occupancy; invalid payload rejected; retained defaults do not overwrite desired tuning; command ordering and effective values visible |
| Web/OTA | Two simultaneous file sessions, large download/upload with sensing metrics; proper error results; matched firmware/UI assets; OTA successful and failed path; post-reboot count recovery |
| Off-season | Actual magnetic arrangement, no CarCounter online, stale retained Car values, appropriate availability of occupancy and season analytics |

Do not perform production fault injection or reset commands to gather assessment evidence. Run simulations/bench tests first. Use isolated test topics and client IDs; hardware tests need a bench environment and approved operating procedure.

## Future consumers

Nightly publishing should consume a dated completed report or a validated HA snapshot after the nightly contract is established. Keep stored history and source freshness distinct. Do not expose ESP32 administration or introduce a second mail sender without identifying the owning email service.

A roadside live matrix should consume read-only count state, with explicit meaning (tonight's entries, occupancy or season total) and stale-data behavior. Controller/network details and coexistence with LOR are unresolved; there is no verified dynamic LOR text binding in this assessment. Display output belongs outside firmware counting logic.

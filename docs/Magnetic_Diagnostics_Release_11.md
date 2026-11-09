# Gate magnetic diagnostics — release .11

Date: 2026-11-09. Firmware: `25.12.28.0-modular-test.11`.
Gate-only change; Car remains on `.10`. No shared implementation files changed.

## Wiring and naming

| Function | Firmware name | GPIO |
|---|---|---|
| Existing Beam A | `beamAPin` | 32 |
| Existing Beam B | `beamBPin` | 33 |
| Added magnetic EL817 output | `magneticSensorPin` | 25 |

The old `magSensorPin` meant Beam A, not the new magnetic sensor. It and
`beamSensorPin` have been removed from active code. Original baseline manifests
retain historical names as evidence; the verifier explicitly maps the reviewed
rename and asserts the added GPIO25 assignment. Existing beam GPIOs, polarity,
debounce, qualification, alarms, count recovery and HA commands are preserved.

User confirms all MCU-side interfaces are at 3.3 V. Connect the new module's
VCC to 3.3 V, GND to MCU GND, OUT to P25. GPIO25 uses INPUT_PULLUP. Do not apply
sensor-side 12 V to the MCU-side pins. The magnetic active polarity has not yet
been measured; telemetry is explicitly raw electrical 0/1, not occupancy.

## Diagnostic behavior

The magnetic input cannot increment exit/show/hourly counts, affect occupancy,
trigger beam alarms, or change reset times. The existing beams remain the only
vehicle-counting authority. This release does not implement off-season magnetic
counting or magnetic tuning commands.

At each detector poll, sample all three raw levels on the same MCU uptime clock.
Record initial state, any observed raw transition, and each beam-confirmed vehicle.
The optional legacy beam log no longer writes its raw reads into detector state;
its new-file header explicitly names Beam B and Beam A. Existing CSVs are not
rewritten, including older files with the legacy `ms,Beam,mag` header.

| MQTT topic (prefix `msb/traffic/GateCounter/`) | Payload | Retained |
|---|---|---|
| `Sensors/magneticRawState` | 0 or 1; refreshed during event export and heartbeat | Yes |
| `Diagnostics/magnetic/event` | JSON event below | No |
| `Diagnostics/magnetic/status` | mode, GPIO, session ID and loss/health metrics | Yes |

Events: `bootId` (random 32-bit session label), `seq` (monotonic within boot),
`ms` (32-bit uptime milliseconds), `kind` (0 initial, 1 raw change, 2 beam vehicle),
`beamARaw`, `beamBRaw`, `magRaw`, `magLevelMs`.
On a magnetic transition `magLevelMs` is the elapsed time in the previous magnetic
level. On a beam-only transition/confirmation it is time since the last magnetic
transition. The first level begins at initialization and can be a clipped pulse.
Compare consecutive records to identify which inputs changed. Simultaneous reads
do not establish physical ordering below the sampling interval. A kind-2 record
marks the detector decision before count persistence; it is not a durable count receipt.

Status is republished on heartbeat (nominally every 30 seconds):
- `mode=DIAGNOSTIC_ONLY`, `gpio=25`, `bootId`.
- `transitions`: magnetic raw changes since boot, NOT cars.
- `queueDropped`: observations discarded because the 64-event queue filled.
- `sdLost`: events not written (clock/recovery unavailable, year transition,
  file unavailable/full or short write).
- `mqttLost`: events not sent while offline or rejected by the MQTT client.
- `maxSampleGapMs`: largest interval between detector samples since boot.
- `sd`: NOT_WRITTEN, OK, CLOCK_OR_RECOVERY_NOT_READY, OPEN_FAILED,
  FILE_LIMIT_REACHED, or WRITE_FAILED. OK reports a successful append, not
  read-back verification or guaranteed power-loss durability.

The trace is polling-based, not an interrupt/oscilloscope capture. Pulses entirely
between samples can be missed. The shared loop still performs display, SD, network
and sensor work. Diagnostic writes also consume loop time. Evaluate maxSampleGapMs
against measured pulse lengths; absence of an event is not proof of no pulse.
All metrics reset at boot. Random boot IDs distinguish sessions probabilistically;
retain the heartbeat boot timestamp alongside them for analysis.

## SD fallback

Append to `/GC/<validated-current-year>/MagneticDiagnostics.csv`, with columns
`bootId,sequence,uptimeMs,kind,beamARaw,beamBRaw,magRaw,magLevelMs`.
No invented wall-clock timestamp is assigned. Correlate uptime with the heartbeat
boot timestamp when valid. No prior-year writes. Events captured while count
recovery is unavailable are exported to MQTT if connected but not written as dated
SD records. Queued events from a previous year are discarded from SD explicitly.

At most one event drains per 100 ms after production detection/persistence. The
64-event RAM buffer is bounded; overload drops diagnostics visibly. No MQTT event
replay after reconnect. SD writes continue when MQTT is offline. File size is capped
at 128 MiB (last row may cross that threshold); once full, SD export stops with
FILE_LIMIT_REACHED while MQTT and beam counting continue. Download/archive the
file before removing it deliberately through the existing file manager. RAM events
can be lost on power loss; this is a diagnostic trace, not the count checkpoint.

## Build, installation and field acceptance

Owner: MSB-Gate-Counter-2025, branch `refactor/shared-firmware`.
Use the established pull/build/OTA workflow with the user's private secrets and
licensed ElegantOTAPro library. No Car update or existing HA YAML changes required.

1. `git pull --ff-only` from the Gate project after publication.
2. `python tools/verify_modular.py --peer "../MSB-Car-Counter-2024"`.
3. `python -m unittest discover -s tests -p "test_*.py"` where native g++ is available.
4. Build the normal PlatformIO environment with the licensed OTA library.
5. Upload Gate `firmware.bin` via its existing firmware OTA page; confirm `.11`.
6. In MQTT Explorer expand `GateCounter/Diagnostics/magnetic` and
   `GateCounter/Sensors/magneticRawState`. Save the current counts/status first.
7. Activate only the magnetic sensor: raw events should change, while exit/show/
   hourly counts and beam alarm remain unchanged. Confirm actual polarity.
8. Pass one vehicle through the beams and magnetic sensing area. Confirm one
   normal exit count, a kind-2 trace marker, and magnetic changes on the same boot
   uptime timeline. Inspect the CSV and all loss metrics.
9. Retain samples for isolated cars, slow/stopped cars, and closely spaced vehicles.
   Tune future magnetic-only counting from this evidence, not from assumed pulses.

No physical deployment or acceptance was performed by the development environment.
The existing cold-start/display fault is separate and is not fixed in this release.

## Development validation

23 host tests passed, including compilation/execution of the real diagnostic
exporter against controlled I/O stubs, raw timing/overflow/wrap tests, and
normalized .10 beam-detector/count-body hash comparison. The modular verifier
passed 133 function bodies with 114 explicit revisions and 34 unchanged shared
files. The original pin-baseline manifest was preserved.

An ESP32/PlatformIO production build with the licensed OTA dependency was not
performed here. Greg's normal local build and physical sensor tests remain
required; native tests are not a hardware or timing acceptance result.

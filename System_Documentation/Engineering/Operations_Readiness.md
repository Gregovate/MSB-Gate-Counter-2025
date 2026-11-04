# Operations readiness and runbook gaps

Status: ASSESSMENT ONLY — this is not an executable deployment runbook.

Repository authority: `System_Documentation/Project_Rules/README.md` and `Runbook_First_Production_Rule.md` were read in both application repositories. They require the responsible repository's governing runbook before any production mutation.

**RUNBOOK GAP FOUND:** the reviewed repositories do not contain a complete, device-specific, recoverable release/rollback procedure for these installed firmware versions and separately stored UI files. No production commands were issued. These gaps must be resolved before a future pilot/deployment.

## Known operating facts

- Car installed version shown by operator:25.12.03.2; Gate:25.12.28.0. Matching version source/binary candidates are in Source_Index.md, but exact live image hashes are unverified.
- Each ESP32 hosts a private HTTP management server on port80 and ElegantOTAPro on the same server. Operator uses ElegantOTA for updates.
- Car screenshot identifies192.168.3.31. Gate address, network access boundaries and actual licensed OTA version were not independently verified.
- Web UI HTML/CSS reside on SD under/data and are uploaded separately. Firmware-only update does not establish that UI/CSV viewers match.
- File manager includes download/upload/delete/rename/directory selection and HTTP reboot. It is administration, not a public report site. No public exposure or new ZeroTier grants are authorized by this assessment.
- Car's onOTAStart has the hourly-save call commented out. Existing callbacks report progress/result, not a demonstrated flush/rollback guarantee. Do not assume reboot is lossless.

## Required release evidence before production

1. Identify each target by role, network address, client ID and current version/source variant. Record actual input wiring and sensor polarity; do not infer from legacy magSensorPin comments.
2. Establish reproducible build prerequisites: exact PlatformIO platform/framework, board/partition settings, compiler and library versions including AsyncTCP, ESPAsyncWebServer and local ElegantOTAPro. Current ini files are insufficient to reproduce the prior builds independently.
3. Create a manifest associating unique firmware version, source commit, binary SHA256, build settings, UI-file hashes and data schema. Retain the previous known-good binary and UI bundle. Do not reuse a version for source changes.
4. Document validated backup and restore of seasonal totals, hourly records, event logs, summaries, show-date configuration and UI assets. Restoring firmware alone does not restore counts or reverse data migrations.
5. Define how counting is paused/observed during OTA, how pending writes are completed, and the maximum accepted count-loss window. Arrange updates outside active traffic until a tested alternative exists.
6. Write exact operator steps for ElegantOTA selection/progress, successful reboot proof, failed upload/reboot handling and recovery fallback. Verify whether prior image rollback is manual and what physical access is required; do not assume automatic rollback support.
7. Specify post-update proof: identity/version, valid clock and season, restored totals, effective tuning/mode, MQTT input/output, private web summary/file access, and a controlled sensor passage on bench before field operation.
8. Identify the approving operator, rollback triggers, evidence/report location and documentation closeout requirements. Retain a source-and-UI-consistent recovery set.

## Diagnostic checklist for the future runbook

The first diagnostic view should distinguish sensor state, detector state, count validity, SD persistence, MQTT connectivity and HA freshness. Record last successful action and error code for each. A device that is reachable but not sampling inputs must not report itself healthy solely because its web page or heartbeat responds.

No card recovery: current Car can continue in RAM but has an incomplete protection against publishing unrestored counts. Current Gate continues through SD failure without an equivalent availability model. The runbook must not recommend reboots as a generic recovery action while count state is unverified.

HA/counter disagreement: preserve evidence of both values and their dates/epochs before issuing assignments. Avoid max(value) reconciliation and automatic repeated resets. Off-season Gate occupancy must not treat a stale retained seasonal Car count as fresh.

Reporting access: external readers should receive a saved report or isolated read-only report page. Keep file manager, reboot, upload/delete and ElegantOTA private. This is a user requirement, not a claim that ZeroTier itself is unsafe.

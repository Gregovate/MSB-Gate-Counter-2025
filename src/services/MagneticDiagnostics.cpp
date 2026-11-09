#include "traffic/Firmware.h"
#include "traffic/MagneticDiagnostics.h"
#include "traffic/MagneticTrace.h"
#include <esp_system.h>

namespace {
traffic::MagneticTrace magneticTrace;
uint32_t magneticBootId=0, magneticLastDrain=0;
uint32_t magneticSdLost=0, magneticMqttLost=0;
const char* magneticSdHealth="NOT_WRITTEN";
constexpr size_t magneticFileLimit=128UL*1024UL*1024UL;
constexpr char magneticEventTopic[]="msb/traffic/GateCounter/Diagnostics/magnetic/event";
constexpr char magneticStatusTopic[]="msb/traffic/GateCounter/Diagnostics/magnetic/status";
}

void initializeMagneticDiagnostics() {
    // EL817 MCU side is supplied at 3.3 V. Pull-up gives an unconnected input
    // a defined level; raw HIGH is NOT interpreted as a detected vehicle.
    pinMode(magneticSensorPin, INPUT_PULLUP);
    magneticBootId=esp_random();
    // First sample is taken in detectCar(), after normal boot recovery.
}

void sampleMagneticDiagnostics(bool beamARaw, bool beamBRaw) {
    // Poll beside the detector's raw beam reads, before any MQTT/SD work.
    // A zero year means recovery was not ready: no dated SD write is allowed.
    magneticTrace.sample(millis(),countsReady()?recordingYear():0,
                         beamARaw,beamBRaw,digitalRead(magneticSensorPin));
}

void markMagneticBeamVehicle() {
    // This is an observation of an existing beam decision, NOT another count.
    magneticTrace.push(millis(),countsReady()?recordingYear():0,2);
}

void serviceMagneticDiagnostics() {
    // Drain at most one event per 100 ms, after production detection/persistence.
    // Overload drops diagnostic events visibly instead of growing a heap queue.
    const uint32_t now=millis();
    if (uint32_t(now-magneticLastDrain)<100) return;
    magneticLastDrain=now;
    traffic::MagneticEvent e;
    if (!magneticTrace.pop(e)) return;
    char payload[256];
    snprintf(payload,sizeof(payload),
        "{\"bootId\":%lu,\"seq\":%lu,\"ms\":%lu,\"kind\":%u,\"beamARaw\":%u,\"beamBRaw\":%u,\"magRaw\":%u,\"magLevelMs\":%lu}",
        (unsigned long)magneticBootId,(unsigned long)e.sequence,(unsigned long)e.ms,
        unsigned(e.kind),unsigned(e.a),unsigned(e.b),unsigned(e.mag),(unsigned long)e.magLevelMs);
    if (mqttOnline())
        publishMQTT("msb/traffic/GateCounter/Sensors/magneticRawState",String(int(magneticTrace.mag)),true);
    // Never enqueue old diagnostic traffic for reconnect replay.
    if (!mqttOnline() || !mqtt_client.publish(magneticEventTopic,payload,false)) ++magneticMqttLost;

    if (!e.year || e.year!=uint32_t(recordingYear()) || !countsReady()) {
        magneticSdHealth="CLOCK_OR_RECOVERY_NOT_READY"; ++magneticSdLost; return;
    }
    const String path=String("/GC/")+String(e.year)+"/MagneticDiagnostics.csv";
    File file=recordingOpen(path,FILE_APPEND);
    if (!file) { magneticSdHealth="OPEN_FAILED"; ++magneticSdLost; return; }
    if (file.size()>=magneticFileLimit) {
        file.close(); magneticSdHealth="FILE_LIMIT_REACHED"; ++magneticSdLost; return;
    }
    constexpr char header[]="bootId,sequence,uptimeMs,kind,beamARaw,beamBRaw,magRaw,magLevelMs\n";
    bool ok=true;
    if (!file.size()) ok=file.print(header)==sizeof(header)-1;
    char row[160];
    const int n=snprintf(row,sizeof(row),"%lu,%lu,%lu,%u,%u,%u,%u,%lu\n",
        (unsigned long)magneticBootId,(unsigned long)e.sequence,(unsigned long)e.ms,
        unsigned(e.kind),unsigned(e.a),unsigned(e.b),unsigned(e.mag),(unsigned long)e.magLevelMs);
    if (ok) ok=file.print(row)==size_t(n);
    file.close();
    magneticSdHealth=ok?"OK":"WRITE_FAILED";
    if (!ok) ++magneticSdLost;
}

void publishMagneticDiagnostics() {
    if (!magneticTrace.initialized || !mqttOnline()) return;
    // Retained snapshot is current state; event stream above is non-retained.
    publishMQTT("msb/traffic/GateCounter/Sensors/magneticRawState",String(int(magneticTrace.mag)),true);
    char payload[384];
    snprintf(payload,sizeof(payload),
        "{\"mode\":\"DIAGNOSTIC_ONLY\",\"gpio\":25,\"bootId\":%lu,\"transitions\":%lu,\"queueDropped\":%lu,\"sdLost\":%lu,\"mqttLost\":%lu,\"maxSampleGapMs\":%lu,\"sd\":\"%s\"}",
        (unsigned long)magneticBootId,(unsigned long)magneticTrace.transitions,
        (unsigned long)magneticTrace.dropped,(unsigned long)magneticSdLost,
        (unsigned long)magneticMqttLost,(unsigned long)magneticTrace.maxGapMs,magneticSdHealth);
    publishMQTT(magneticStatusTopic,String(payload),true);
}

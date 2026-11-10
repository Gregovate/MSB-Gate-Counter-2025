#pragma once
#include <stdint.h>

namespace traffic {
// Fixed-size, allocation-free trace queue. Raw levels deliberately have no
// assumed magnetic polarity or debounce; calibration requires seeing chatter.
struct MagneticEvent {
    uint32_t sequence, ms, magLevelMs, year;
    uint8_t a, b, mag, kind; // kind: 0 initial, 1 transition, 2 beam vehicle
};
struct MagneticTrace {
    MagneticEvent queue[64]{};
    uint32_t sequence=0, dropped=0, transitions=0, maxGapMs=0;
    uint32_t lastSample=0, magSince=0;
    uint8_t head=0, size=0;
    bool initialized=false, a=false, b=false, mag=false;

    void push(uint32_t ms, uint32_t year, uint8_t kind) {
        MagneticEvent e{++sequence, ms, uint32_t(ms-magSince), year,
                        uint8_t(a),uint8_t(b),uint8_t(mag),kind};
        if (size==64) { ++dropped; return; }
        queue[(head+size)%64]=e;
        ++size;
    }
    void sample(uint32_t ms, uint32_t year, bool ar, bool br, bool mr) {
        if (!initialized) {
            initialized=true; a=ar; b=br; mag=mr;
            lastSample=magSince=ms;
            push(ms,year,0); return;
        }
        const uint32_t gap=ms-lastSample;
        if (gap>maxGapMs) maxGapMs=gap;
        lastSample=ms;
        if (a==ar && b==br && mag==mr) return;
        const bool magChanged=mag!=mr;
        a=ar; b=br; mag=mr;
        // On magnetic transition magLevelMs describes the PREVIOUS level.
        push(ms,year,1);
        if (magChanged) { ++transitions; magSince=ms; }
    }
    bool pop(MagneticEvent &e) {
        if (!size) return false;
        e=queue[head]; head=(head+1)%64; --size; return true;
    }
};
}

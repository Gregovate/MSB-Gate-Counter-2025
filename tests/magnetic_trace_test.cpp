#include "traffic/MagneticTrace.h"
#include <cassert>
#include <limits>
int main() {
    traffic::MagneticTrace t;
    traffic::MagneticEvent e;
    t.sample(10,2026,0,0,1);
    assert(t.pop(e) && e.kind==0 && e.mag==1 && e.sequence==1);
    t.sample(20,2026,0,0,1); assert(!t.pop(e));
    t.sample(30,2026,1,0,1);
    assert(t.pop(e) && e.a==1 && e.magLevelMs==20);
    t.sample(40,2026,1,1,0);
    assert(t.pop(e) && e.b==1 && e.mag==0 && e.magLevelMs==30);
    t.sample(41,2026,1,1,1);
    assert(t.pop(e) && e.mag==1 && e.magLevelMs==1 && t.transitions==2);
    t.push(42,2026,2); assert(t.pop(e) && e.kind==2 && e.ms==42);
    // Bounded overflow preserves sequence gaps for comparison and keeps
    // tracking current levels even if the diagnostic exporter is overloaded.
    for (unsigned i=0;i<70;++i) t.sample(100+i,2026,0,0,i%2);
    assert(t.size==64 && t.dropped==6);
    for(unsigned i=0;i<64;++i) assert(t.pop(e));
    assert(!t.pop(e));
    t.sample(1000,0,0,0,0); assert(t.pop(e) && e.year==0);
    assert(t.maxGapMs==831);
    traffic::MagneticTrace wrap;
    wrap.sample(UINT32_MAX-4,2026,0,0,0); wrap.pop(e);
    wrap.sample(5,2026,0,0,1);
    assert(wrap.pop(e) && e.magLevelMs==10 && wrap.maxGapMs==10);
}

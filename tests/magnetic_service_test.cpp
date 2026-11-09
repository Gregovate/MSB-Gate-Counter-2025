#include "../src/services/MagneticDiagnostics.cpp"
#include <cassert>
int main(){
    pins[25]=1;initializeMagneticDiagnostics();
    sampleMagneticDiagnostics(0,0);
    assert(configuredPin==25 && configuredMode==INPUT_PULLUP);
    clockMs=100;serviceMagneticDiagnostics();
    assert(lastPath=="/GC/2026/MagneticDiagnostics.csv");
    assert(logBytes.find("123456,1,0,0,0,0,1,0")!=std::string::npos);
    online=false;pins[25]=0;clockMs=110;sampleMagneticDiagnostics(1,1);
    clockMs=200;serviceMagneticDiagnostics();
    assert(magneticMqttLost==1 && magneticSdLost==0);
    ready=false;clockMs=210;sampleMagneticDiagnostics(0,0);
    auto before=logBytes;clockMs=300;serviceMagneticDiagnostics();
    assert(logBytes==before && magneticSdLost==1);
    ready=true;online=true;clockMs=310;sampleMagneticDiagnostics(1,0);
    openOk=false;clockMs=400;serviceMagneticDiagnostics();
    assert(magneticSdLost==2 && std::string(magneticSdHealth)=="OPEN_FAILED");
    openOk=true;clockMs=410;sampleMagneticDiagnostics(0,0);
    writeOk=false;clockMs=500;serviceMagneticDiagnostics();
    assert(magneticSdLost==3 && std::string(magneticSdHealth)=="WRITE_FAILED");
    writeOk=true;clockMs=510;markMagneticBeamVehicle();
    fakeSize=128UL*1024UL*1024UL;clockMs=600;serviceMagneticDiagnostics();
    assert(magneticSdLost==4 && std::string(magneticSdHealth)=="FILE_LIMIT_REACHED");
    fakeSize=0;clockMs=610;markMagneticBeamVehicle();year=2027;
    clockMs=700;serviceMagneticDiagnostics();assert(magneticSdLost==5);
    publishMagneticDiagnostics();
    assert(published.back().find("DIAGNOSTIC_ONLY")!=std::string::npos);
}

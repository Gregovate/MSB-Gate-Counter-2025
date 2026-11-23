"""Execute the real Gate detector with long stops and close arrivals."""
from pathlib import Path
import re,subprocess,tempfile,unittest
ROOT=Path(__file__).resolve().parents[1]
class GatePassages(unittest.TestCase):
 def test_real_gate_detector(self):
  source=(ROOT/'src/services/VehicleDetector.cpp').read_text()
  source=re.sub(r'void countTheCar\(\) \{.*?^\}', 'void countTheCar(){++totalDailyCars;loggedBetween=timeBetweenCars_ms;}',source,count=1,flags=re.S|re.M)
  header=r'''#pragma once
#include <string>
#include <cassert>
#include <cstdint>
#include <iostream>
#define HIGH 1
#define LOW 0
#define FILE_APPEND "a"
struct String:std::string {using std::string::string;String(const std::string&s):std::string(s){}String(unsigned long n):std::string(std::to_string(n)){}String(int n):std::string(std::to_string(n)){}};
unsigned long ticks=0;unsigned long millis(){return ticks;}
int pins[40]{};int digitalRead(int p){return pins[p];}
bool loggingEnabled=false,stableRawA=false,stableRawB=false,gateStuckAlarmActive=false,abTimeoutLogged=false,carPresentFlag=false;
int beamAPin=32,beamBPin=33,beamAState=0,beamBState=0,lastBeamAState=0,lastBeamBState=0,totalDailyCars=0;
unsigned long lastRawAChangeMs=0,lastRawBChangeMs=0,debounceDelay=50,firstBeamHealth_ms=0,secondBeamHealth_ms=0,gateCounterTimeout=60000,beamATripTime_ms=0,minActivationDuration=100,bothBeamsBroken_ms=0,maxABFollow_ms=5000,carDetectMS=500,lastCarDetected_ms=0,timeToPassMS=0,abFollow_ms=0,timeBetweenCars_ms=0,loggedBetween=0;
enum State{WAITING_FOR_CAR,BEAM_A_HIGH,BOTH_BEAMS_HIGH,CAR_DETECTED};State gateDetectState=WAITING_FOR_CAR;
struct SerialMock{template<class...T>void printf(const char*,T...){}template<class T>void println(T){}} Serial;
struct File{operator bool(){return false;}template<class...T>void printf(const char*,T...){}void close(){}};
String fileName10="trace";File recordingOpen(const String&,const char*){return {};}
void publishMQTT(const char*,const String&,bool=false){}
void sampleMagneticDiagnostics(bool,bool){}void markMagneticBeamVehicle(){}
#define MQTT_PUB_BEAM_A_STATE "a"
#define MQTT_PUB_BEAM_B_STATE "b"
#define MQTT_PUB_BEAM_AB_MS "ab"
#define MQTT_PUB_BEAM_B_BROKEN_MS "bms"
#define MQTT_PUB_BETWEENCARS_MS "gap"
#define MQTT_PUB_ALARM "alarm"
#define MQTT_PUB_TTP "ttp"
#define MQTT_COUNTER_LOG "log"
'''
  harness=r'''
void sample(int a,int b,unsigned long t){pins[beamAPin]=a;pins[beamBPin]=b;ticks=t;detectCar();ticks=t+60;detectCar();}
void car(unsigned long t){sample(1,0,t);sample(1,1,t+200);sample(0,1,t+900);sample(0,0,t+1100);}
int main(){
 sample(0,0,100);sample(1,0,1000);sample(1,1,1300);
 sample(1,1,61000);assert(gateStuckAlarmActive && totalDailyCars==0);
 sample(0,1,62000);sample(1,1,120000); // axle/trailer rebreak is still same car
 sample(1,1,583000);assert(totalDailyCars==0 && gateStuckAlarmActive);
 sample(1,0,583500);assert(totalDailyCars==1 && gateDetectState==CAR_DETECTED);
 sample(1,0,584000);assert(totalDailyCars==1);sample(0,0,584500);
 for(unsigned i=0;i<100;++i)car(585000+i*1300);
 assert(totalDailyCars==101 && loggedBetween==1300 && !gateStuckAlarmActive);
 std::cout<<"PASS: Gate 582-second hold, residual A, one count, 100 closely spaced cars and current-event spacing\n";
}
'''
  with tempfile.TemporaryDirectory() as tmp:
   p=Path(tmp);(p/'traffic').mkdir();(p/'traffic/Firmware.h').write_text(header);(p/'traffic/MagneticDiagnostics.h').write_text('')
   (p/'test.cpp').write_text(source+harness)
   subprocess.run(['g++','-std=c++17','-Wall','-Wextra','-Werror','-I'+str(p),str(p/'test.cpp'),'-o',str(p/'test')],check=True)
   subprocess.run([str(p/'test')],check=True)

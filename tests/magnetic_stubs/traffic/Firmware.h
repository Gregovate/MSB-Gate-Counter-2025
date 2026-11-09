#pragma once
#include <string>
#include <map>
#include <vector>
#include <stdint.h>
#include <cstdio>
#include <cstring>
#include <cstddef>
#define INPUT_PULLUP 2
#define beamAPin 32
#define beamBPin 33
#define magneticSensorPin 25
#define FILE_APPEND "a"
struct String:std::string {
    using std::string::string;
    String(std::string s):std::string(s){}
    String(int v):std::string(std::to_string(v)){}
    String(uint32_t v):std::string(std::to_string(v)){}
};
inline uint32_t clockMs=0;
inline bool ready=true, online=true, openOk=true, writeOk=true;
inline int year=2026, pins[40]{}, configuredPin=-1, configuredMode=-1;
inline size_t fakeSize=0;
inline std::string logBytes, lastPath;
inline std::vector<std::string> published;
inline uint32_t millis(){return clockMs;}
inline bool countsReady(){return ready;}
inline int recordingYear(){return year;}
inline bool mqttOnline(){return online;}
inline void pinMode(int p,int m){configuredPin=p;configuredMode=m;}
inline int digitalRead(int p){return pins[p];}
struct File {
    explicit operator bool()const{return openOk;}
    size_t size(){return fakeSize+logBytes.size();}
    size_t print(const char* s){if(!writeOk)return 0;logBytes+=s;return strlen(s);}
    void close(){}
};
inline File recordingOpen(const String &p,const char*){lastPath=p;return File{};}
struct Mqtt { bool publish(const char*,const char* p,bool){published.emplace_back(p);return true;} };
inline Mqtt mqtt_client;
inline void publishMQTT(const char*,const String& s,bool){published.push_back(s);}

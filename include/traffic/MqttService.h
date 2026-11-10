#pragma once
#include "RuntimeState.h"

// Service interface; definitions are compiled once in src/services.
void KeepMqttAlive();
void publishHello(const char* status, const char* msg, bool retainFlag);
void debugInPark(const char* reason);
bool topicIs(const char* t, const char* target);
void publishStateExitInPark();
void publishTimingConfig();
void callback(char* topic, byte* payload, unsigned int length);

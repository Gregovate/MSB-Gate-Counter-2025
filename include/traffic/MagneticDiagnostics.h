#pragma once
#include <stdint.h>

// Gate-only observation: never feeds the production vehicle state machine.
void initializeMagneticDiagnostics();
void sampleMagneticDiagnostics(bool beamARaw, bool beamBRaw);
void markMagneticBeamVehicle();
void serviceMagneticDiagnostics();
void publishMagneticDiagnostics();

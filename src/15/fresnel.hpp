#pragma once

#include "math.hpp"

// Exact, unpolarized Fresnel equation
inline float fresnelUnpolarized(
        float cosI, // cosIncident
        float cosT, // cosTransmitted
        float n1,   // refractiveIndexIncident,
        float n2    // refractiveIndexTransmitted
        )
{
    float Fs = (n1 * cosI - n2 * cosT) / (n1 * cosI + n2 * cosT);
    Fs *= Fs;
    float Fp = (n1 * cosT - n2 * cosI) / (n1 * cosT + n2 * cosI);
    Fp *= Fp;
    return 0.5f * (Fs + Fp);
}

#pragma once

#include <random>

#include "math.hpp"

class Prng
{
public:
    std::mt19937_64 generator;
    std::uniform_real_distribution<float> distribution;

    Prng(unsigned long seed) : generator(seed), distribution(0.0f, 1.0f)
    {
    }

    // return random number uniformly distributed in [0,1)
    float in01()
    {
        return distribution(generator);
    }

    // return random point uniformly distributed in the unit sphere
    vec3 inUnitSphere()
    {
        vec3 p;
        do {
            p = vec3(2.0f * in01() - 1.0f,
                     2.0f * in01() - 1.0f,
                     2.0f * in01() - 1.0f);
        } while (dot(p, p) >= 1.0f);
        return p;
    }

    // return random point uniformly distributed on the unit sphere
    vec3 onUnitSphere()
    {
        return normalize(inUnitSphere());
    }
};

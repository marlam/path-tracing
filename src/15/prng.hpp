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
};

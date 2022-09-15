#pragma once

#include "math.hpp"
#include "surface.hpp"
#include "ray.hpp"

typedef enum {
    ScatterNone,      // no scattering, the path stops here
    ScatterExplicit,  // explicit scatter direction
    ScatterRandom,    // random scatter direction
} ScatterType;

class ScatterRecord
{
public:
    ScatterType type;     // see above
    vec3 direction;       // the scatter direction
    float p;              // the probability of choosing the direction
    vec3 attenuation;     // the attenuation for the direction: brdf * cosTheta

    // Constructor for type ScatterNone
    ScatterRecord() :
        type(ScatterNone), direction(0.0f), p(0.0f), attenuation(0.0f)
    {
    }

    // Constructor for type ScatterExplicit
    ScatterRecord(const vec3& dir, const vec3& att) :
        type(ScatterExplicit), direction(dir), p(1.0f), attenuation(att)
    {
    }

    // Constructor for type ScatterRandom
    ScatterRecord(const vec3& dir, float p, const vec3& att) :
        type(ScatterRandom), direction(dir), p(p), attenuation(att)
    {
    }
};

class Material
{
public:
    Material()
    {
    }

    virtual vec3 Le(const HitRecord& /* hr */, const vec3& /* out */) const
    {
        return vec3(0.0f);
    }

    virtual ScatterRecord scatter(const Ray& /* ray */, const HitRecord& /* hr */, Prng& /* prng */) const
    {
        return ScatterRecord();
    }
};

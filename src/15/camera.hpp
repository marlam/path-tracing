#pragma once

#include <cmath>

#include "math.hpp"
#include "animation.hpp"
#include "sampler.hpp"
#include "prng.hpp"

// Pinhole camera class
class Camera
{
public:
    const float t, b, r, l;    // Frustum corners at distance=1: top, bottom, right, left
    const float focusDistance;
    const float apertureRadius;
    const Animation* animation;

    Camera(float vfov,   // vertical field of view = vertical opening angle, in radians
            float aspectRatio,
            float focusDistance,
            float apertureDiameter,
            const Animation* anim = nullptr) :
        t(std::tan(vfov * 0.5f)),
        b(-t),
        r(t * aspectRatio),
        l(-r),
        focusDistance(focusDistance),
        apertureRadius(0.5f * apertureDiameter),
        animation(anim)
    {
    }

    Ray getRay(float p, float q, float t0, float t1, Prng& prng)
    {
        // get origin O and point P on image plane
        vec3 P = vec3(mix(l, r, p), mix(b, t, q), -1.0f);
        vec3 O = vec3(0.0f);
        // apply depth of field effect
        P *= focusDistance;
        vec2 pointOnLens = apertureRadius * Sampler::uniformInDisk(prng.in01(), prng.in01());
        O = vec3(pointOnLens.x(), pointOnLens.y(), 0.0f);
        // compute direction D
        vec3 D = P - O;
        // assign time
        float t = mix(t0, t1, prng.in01());
        // transform
        if (animation) {
            Transformation T = animation->at(t);
            O = T * O;
            D = T.rotation * D;
        }
        // create ray
        return Ray(O, normalize(D), t);
    }
};

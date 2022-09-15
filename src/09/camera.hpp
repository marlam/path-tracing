#pragma once

#include <cmath>

#include "math.hpp"
#include "animation.hpp"
#include "prng.hpp"

// Pinhole camera class
class Camera
{
public:
    const float t, b, r, l;    // Frustum corners at distance=1: top, bottom, right, left
    const Animation* animation;

    Camera(float vfov,   // vertical field of view = vertical opening angle, in radians
            float aspectRatio,
            const Animation* anim = nullptr) :
        t(std::tan(vfov * 0.5f)),
        b(-t),
        r(t * aspectRatio),
        l(-r),
        animation(anim)
    {
    }

    Ray getRay(float p, float q, float t0, float t1, Prng& prng)
    {
        // get origin O and direction D in camera space
        vec3 P = vec3(mix(l, r, p), mix(b, t, q), -1.0f);
        vec3 O = vec3(0.0f);
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

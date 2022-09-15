#pragma once

#include "math.hpp"

class Sampler
{
public:
    Sampler()
    {
    }

    static vec3 uniformOnHemisphere(float u0, float u1)
    {
        float z = u0;
        float r = std::sqrt(std::max(0.0f, 1.0f - z * z));
        float phi = 2.0f * pi * u1;
        return vec3(r * std::cos(phi), r * std::sin(phi), z);
    }

    static vec2 uniformInDisk(float u0, float u1)
    {
        vec2 u = vec2(u0, u1);
        vec2 uOffset = 2.0f * u - vec2(1.0f, 1.0f);
        float theta, r;
        if (std::abs(uOffset.x()) > std::abs(uOffset.y())) {
            r = uOffset.x();
            theta = pi / 4.0f * (uOffset.y() / uOffset.x());
        } else {
            r = uOffset.y();
            theta = 0.5f * pi - pi / 4.0f * (uOffset.x() / uOffset.y());
        }
        return r * vec2(std::cos(theta), std::sin(theta));
    }

    static vec3 cosineWeightedOnHemisphere(float u0, float u1)
    {
        vec2 d = uniformInDisk(u0, u1);
        float discriminant = 1.0f - dot(d, d);
        float z = std::sqrt(std::max(0.0f, discriminant));
        return vec3(d.x(), d.y(), z);
    }

    static vec3 phongWeightedOnHemisphere(float shininess, float u0, float u1)
    {
        float cosThetaSpec = std::pow(1.0f - u0, 1.0f / (1.0f + shininess));
        float discriminant = 1.0f - cosThetaSpec * cosThetaSpec;
        float sinThetaSpec = std::sqrt(std::max(0.0f, discriminant));
        float phi = 2.0f * pi * u1;
        return vec3(std::cos(phi) * sinThetaSpec, std::sin(phi) * sinThetaSpec, cosThetaSpec);
    }
};

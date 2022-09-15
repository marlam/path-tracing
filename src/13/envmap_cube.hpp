#pragma once

#include "envmap.hpp"
#include "texture.hpp"

class EnvMapCube : public EnvMap
{
public:
    const Texture* cubesides[6];

    EnvMapCube(
            const Texture* posx, const Texture* negx,
            const Texture* posy, const Texture* negy,
            const Texture* posz, const Texture* negz) :
        cubesides { posx, negx, posy, negy, posz, negz }
    {
    }

    virtual vec3 value(const vec3& direction, float t) const override
    {
        float ax = std::abs(direction.x());
        float ay = std::abs(direction.y());
        float az = std::abs(direction.z());
        int cubeside;
        float u, v;
        if (ax > ay && ax > az) {
            u = 0.5f * (direction.z() / -direction.x() + 1.0f);
            v = 0.5f * (direction.y() / ax + 1.0f);
            cubeside = 0 + std::signbit(direction.x());
        } else if (ay > az) {
            u = 0.5f * (direction.x() / ay + 1.0f);
            v = 0.5f * (direction.z() / -direction.y() + 1.0f);
            cubeside = 2 + std::signbit(direction.y());
        } else {
            u = 0.5f * (direction.x() / direction.z() + 1.0f);
            v = 0.5f * (direction.y() / az + 1.0f);
            cubeside = 4 + std::signbit(direction.z());
        }
        return cubesides[cubeside]->value(vec2(u, v), t);
    }
};

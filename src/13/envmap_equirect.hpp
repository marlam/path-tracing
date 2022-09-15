#pragma once

#include "envmap.hpp"
#include "texture.hpp"

class EnvMapEquiRect : public EnvMap
{
public:
    const Texture* map;

    EnvMapEquiRect(const Texture* m) : map(m)
    {
    }

    virtual vec3 value(const vec3& direction, float t) const override
    {
        float theta = std::asin(std::min(1.0f, std::max(-1.0f, direction.y())));
        float phi = std::atan2(-direction.x(), direction.z());
        float u = phi / (2.0f * pi);
        float v = theta / pi + 0.5f;
        return map->value(vec2(u, v), t);
    }
};

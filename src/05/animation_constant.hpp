#pragma once

#include "animation.hpp"

class AnimationConstant : public Animation
{
public:
    const Transformation T;

    AnimationConstant(const Transformation& transf) : T(transf)
    {
    }

    virtual Transformation at(float /* t */) const
    {
        return T;
    }
};

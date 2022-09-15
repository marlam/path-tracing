#pragma once

#include "transformation.hpp"

class Animation
{
public:
    virtual Transformation at(float /* t */) const
    {
        return Transformation();
    }
};

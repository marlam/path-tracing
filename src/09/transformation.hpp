#pragma once

#include "math.hpp"

// A transformation defines a pose, consisting of translation, rotation, and scaling.
class Transformation
{
public:
    // Member variables
    vec3 translation;
    quat rotation;
    vec3 scaling;

    // Constructor
    explicit Transformation(const vec3& t = vec3(0.0f), const quat& r = quat::null(), const vec3& s = vec3(1.0f)) :
        translation(t), rotation(r), scaling(s)
    {
    }

    // Constructor for a common way of defining view matrices (like gluLookAt)
    explicit Transformation(const vec3& eye, const vec3& center, const vec3& up = vec3(0.0f, 1.0f, 0.0f))
    {
        vec3 f = normalize(center - eye);
        vec3 s = normalize(cross(f, up));
        vec3 u = cross(s, f);
        quat rot0 = quat(vec3(0.0f, 0.0f, -1.0f), f);
        quat rot1 = quat(rot0 * vec3(0.0f, 1.0f, 0.0f), u);
        translation = eye;
        rotation = rot1 * rot0;
        scaling = vec3(1.0f);
    }

    // Apply this transformation to a vector
    vec3 operator*(const vec3& v) const
    {
        return translation + (rotation * (v * scaling));
    }

    // Apply a translation to this transformation
    void translate(const vec3& v)
    {
        translation += rotation * (v * scaling);
    }

    // Apply a rotation to this transformation
    void rotate(const quat& q)
    {
        rotation *= q;
    }

    // Apply scaling to this transformation
    void scale(const vec3& s)
    {
        scaling *= s;
    }

    // Combine two transformations
    inline Transformation operator*(const Transformation& T)
    {
        Transformation R = *this;
        R.translate(T.translation);
        R.rotate(T.rotation);
        R.scale(T.scaling);
        return R;
    }

    // Combine and assign transformations
    inline Transformation& operator*=(const Transformation& T)
    {
        this->translate(T.translation);
        this->rotate(T.rotation);
        this->scale(T.scaling);
        return *this;
    }
};

// Interpolate two transformations. The value of alpha must be in [0,1],
// where 0 results in T0, and 1 results in T1. Positions are interpolated
// inearly and rotations are interpolated via spherical linear interpolation (slerp)
inline Transformation mix(const Transformation& T0, const Transformation& T1, float alpha)
{
    return Transformation(
            mix(T0.translation, T1.translation, alpha),
            slerp(T0.rotation, T1.rotation, alpha),
            mix(T0.scaling, T1.scaling, alpha));
}

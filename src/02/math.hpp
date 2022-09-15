#pragma once

#include <cmath>
#include <limits>
#include <numbers>

// Constants

const float pi = std::numbers::pi_v<float>;

// Useful functions on float

inline float radians(float v) { return v * pi / 180.0f; }
inline float degrees(float v) { return v * 180.0f / pi; }

inline float mix(float x, float y, float alpha) { return x + alpha * (y - x); }

// Vectors

template<int N>
class vec
{
public:
    float values[N];

    // Constructors

    vec() {}
    explicit vec(float v) { for (int i = 0; i < N; i++) values[i] = v;  }
    explicit vec(float x, float y)          requires (N == 2) : values { x, y }    {}
    explicit vec(float x, float y, float z) requires (N == 3) : values { x, y, z } {}

    // Data access

    const float& operator[](size_t i) const { return values[i]; }
          float& operator[](size_t i)       { return values[i]; }

    const float& x() const requires (N >= 1) { return values[0]; }
          float& x()       requires (N >= 1) { return values[0]; }
    const float& y() const requires (N >= 2) { return values[1]; }
          float& y()       requires (N >= 2) { return values[1]; }
    const float& z() const requires (N >= 3) { return values[2]; }
          float& z()       requires (N >= 3) { return values[2]; }

    // Operators

    vec operator+() const { return *this; }
    vec operator-() const { vec r; for (int i = 0; i < N; i++) r[i] = -(*this)[i]; return r; }

    vec operator*(float s) const { vec r; for (int i = 0; i < N; i++) r[i] = (*this)[i] * s; return r; }
    vec operator/(float s) const { vec r; for (int i = 0; i < N; i++) r[i] = (*this)[i] / s; return r; }
    vec operator+(const vec& v) const { vec r; for (int i = 0; i < N; i++) r[i] = (*this)[i] + v[i]; return r; }
    vec operator-(const vec& v) const { vec r; for (int i = 0; i < N; i++) r[i] = (*this)[i] - v[i]; return r; }
    vec operator*(const vec& v) const { vec r; for (int i = 0; i < N; i++) r[i] = (*this)[i] * v[i]; return r; }
    vec operator/(const vec& v) const { vec r; for (int i = 0; i < N; i++) r[i] = (*this)[i] / v[i]; return r; }

    vec& operator*=(float s) { for (int i = 0; i < N; i++) values[i] *= s; return *this; }
    vec& operator/=(float s) { for (int i = 0; i < N; i++) values[i] /= s; return *this; }
    vec& operator+=(const vec& v) { for (int i = 0; i < N; i++) values[i] += v[i]; return *this; }
    vec& operator-=(const vec& v) { for (int i = 0; i < N; i++) values[i] -= v[i]; return *this; }
    vec& operator*=(const vec& v) { for (int i = 0; i < N; i++) values[i] *= v[i]; return *this; }
    vec& operator/=(const vec& v) { for (int i = 0; i < N; i++) values[i] /= v[i]; return *this; }

};

template<int N> vec<N> operator*(float s, const vec<N>& v) { return v * s; }

using vec2 = vec<2>;
using vec3 = vec<3>;

// Useful functions on vectors

template<int N>
vec<N> mix(const vec<N>& v0, const vec<N>& v1, float alpha)
{
    vec<N> r;
    for (int i = 0; i < N; i++)
        r[i] = mix(v0[i], v1[i], alpha);
    return r;
}

template<int N>
float dot(const vec<N>& v0, const vec<N>& v1)
{
    float r = 0.0f;
    for (int i = 0; i < N; i++)
        r += v0[i] * v1[i];
    return r;
}

template<int N>
float length(const vec<N>& v)
{
    return std::sqrt(dot(v, v));
}

template<int N>
vec<N> normalize(const vec<N>& v)
{
    return v / length(v);
}

vec3 cross(const vec3& v, const vec3& w)
{
    return vec3(
            v.y() * w.z() - v.z() * w.y(),
            v.z() * w.x() - v.x() * w.z(),
            v.x() * w.y() - v.y() * w.x());
}

// Quaternions

class quat
{
public:
    float x, y, z, w;

    // Basic Constructors

    quat() {}
    quat(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    static quat null() { return quat(0.0f, 0.0f, 0.0f, 1.0f); }

    // Constructor for angle/axis representation.
    quat(float angle, const vec3& axis)
    {
        vec3 a = normalize(axis);
        float sin_angle_2 = std::sin(0.5f * angle);
        float cos_angle_2 = std::cos(0.5f * angle);
        x = a.x() * sin_angle_2;
        y = a.y() * sin_angle_2;
        z = a.z() * sin_angle_2;
        w = cos_angle_2;
    }

    // Constructor for a rotation that rotates dir1 into dir2.
    quat(const vec3& dir1, const vec3& dir2)
    {
        vec3 d1 = normalize(dir1);
        vec3 d2 = normalize(dir2);
        float cosAngle = dot(d1, d2);
        if (cosAngle >= 1.0f - std::numeric_limits<float>::epsilon()) {
            // angle is close to zero
            *this = null();
        } else if (cosAngle <= -1.0f + std::numeric_limits<float>::epsilon()) {
            // angle is close to 180°
            vec3 t = vec3(0.0f, 1.0f, 0.0f);
            if (dot(d1, t) >= 1.0f - std::numeric_limits<float>::epsilon())
                t = vec3(1.0f, 0.0f, 0.0f);
            *this = quat(pi, cross(t, d1));
        } else {
            *this = quat(std::acos(cosAngle), cross(d1, d2));
        }
    }

    // Functions

    friend quat conjugate(const quat& q) { return quat(-q.x, -q.y, -q.z, q.w); }
    friend quat inverse(const quat& q) { return conjugate(q); }

    friend quat slerp(const quat& q, const quat& r, float alpha)
    {
        quat w = r;
        float cos_half_angle = q.x * r.x + q.y * r.y + q.z * r.z + q.w * r.w;
        if (cos_half_angle < 0.0f) {
            // quat(x, y, z, w) and quat(-x, -y, -z, -w) represent the same rotation
            w.x = -w.x; w.y = -w.y; w.z = -w.z; w.w = -w.w;
            cos_half_angle = -cos_half_angle;
        }
        float tmp_q, tmp_w;
        if (cos_half_angle >= 1.0f) {
            // angle is zero => rotations are identical
            tmp_q = 1.0f;
            tmp_w = 0.0f;
        } else {
            float half_angle = std::acos(cos_half_angle);
            float sin_half_angle = std::sqrt(1.0f - cos_half_angle * cos_half_angle);
            if (std::abs(sin_half_angle) < std::numeric_limits<float>::epsilon()){
                // angle is 180 degrees => result is not clear
                tmp_q = 0.5f;
                tmp_w = 0.5f;
            } else {
                tmp_q = std::sin((1.0f - alpha) * half_angle) / sin_half_angle;
                tmp_w = std::sin(alpha * half_angle) / sin_half_angle;
            }
        }
        return quat(
                q.x * tmp_q + w.x * tmp_w,
                q.y * tmp_q + w.y * tmp_w,
                q.z * tmp_q + w.z * tmp_w,
                q.w * tmp_q + w.w * tmp_w);
    }

    // Operators

    quat operator+() const { return *this; }
    quat operator-() const { return inverse(*this); }

    quat operator*(const quat& q) const
    {
        quat p;
        p.x = w * q.x + x * q.w + y * q.z - z * q.y;
        p.y = w * q.y + y * q.w + z * q.x - x * q.z;
        p.z = w * q.z + z * q.w + x * q.y - y * q.x;
        p.w = w * q.w - x * q.x - y * q.y - z * q.z;
        return p;
    }

    const quat& operator*=(const quat& q)
    {
        *this = *this * q;
        return *this;
    }

    // Applying rotations to vectors

    friend vec3 operator*(const quat& q, const vec3& v)
    {
        quat t = q * quat(v.x(), v.y(), v.z(), 0.0f) * conjugate(q);
        return vec3(t.x, t.y, t.z);
    }

    friend vec3 operator*(const vec3& v, const quat& q)
    {
        quat t = conjugate(q) * quat(v.x(), v.y(), v.z(), 0.0f) * q;
        return vec3(t.x, t.y, t.z);
    }
};

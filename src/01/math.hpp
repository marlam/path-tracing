#pragma once

#include <cmath>
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

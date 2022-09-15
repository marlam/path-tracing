#pragma once

#include "math.hpp"

inline vec3 rgb_to_xyz(const vec3& rgb)
{
    return 100.0f * vec3(
            (0.412453f * rgb.x() + 0.357580f * rgb.y() + 0.180423f * rgb.z()),
            (0.212671f * rgb.x() + 0.715160f * rgb.y() + 0.072169f * rgb.z()),
            (0.019334f * rgb.x() + 0.119193f * rgb.y() + 0.950227f * rgb.z()));
}

inline vec3 xyz_to_rgb(const vec3& xyz)
{
    return 0.01f * vec3(
            (+3.240479f * xyz.x() - 1.537150f * xyz.y() - 0.498535f * xyz.z()),
            (-0.969256f * xyz.x() + 1.875991f * xyz.y() + 0.041556f * xyz.z()),
            (+0.055648f * xyz.x() - 0.204023f * xyz.y() + 1.057311f * xyz.z()));
}

inline vec3 adjust_y(const vec3& xyz, float new_y)
{
    if (xyz.y() <= 0.0f)
        return vec3(0.0f);
    float sum = xyz.x() + xyz.y() + xyz.z();
    if (sum <= 0.0f)
        return vec3(0.0f);
    // keep old chromaticity in terms of x, y
    float x = xyz.x() / sum;
    float y = xyz.y() / sum;
    // apply new luminance
    float r = new_y / y;
    return vec3(r * x, new_y, r * (1.0f - x - y));
}

inline float linear_to_nonlinear(float x)
{
    return (x <= 0.0031308f ? (x * 12.92f) : (1.055f * std::pow(x, 1.0f / 2.4f) - 0.055f));
}

inline vec3 linear_to_nonlinear(const vec3& rgb)
{
    return vec3(linear_to_nonlinear(rgb.x()),
                linear_to_nonlinear(rgb.y()),
                linear_to_nonlinear(rgb.z()));
}

inline float nonlinear_to_linear(float x)
{
    return (x <= 0.04045f ? (x * (1.0f / 12.92f)) : std::pow((x + 0.055f) * (1.0f / 1.055f), 2.4f));
}

inline vec3 nonlinear_to_linear(const vec3& rgb)
{
    return vec3(nonlinear_to_linear(rgb.x()),
                nonlinear_to_linear(rgb.y()),
                nonlinear_to_linear(rgb.z()));
}

inline float byte_to_float(unsigned char x)
{
    return x / 255.0f;
}

inline unsigned char float_to_byte(float x)
{
    x = (x < 0.0f ? 0.0f : x > 1.0f ? 1.0f : x);
    return std::round(x * 255.0f);
}

inline void uniformRationalQuantization(std::vector<vec3>& img, float maxVal, float brightness /* in [1,inft] */)
{
    for (size_t i = 0; i < img.size(); i++) {
        vec3 rgb = img[i];
        vec3 xyz = rgb_to_xyz(rgb);
        float old_y = xyz.y();
        float new_y = brightness * old_y / ((brightness - 1.0f) * old_y + maxVal);
        xyz = adjust_y(xyz, 100.0f * new_y);
        rgb = xyz_to_rgb(xyz);
        img[i] = rgb;
    }
}

inline std::vector<unsigned char> to8Bit(const std::vector<vec3>& img)
{
    std::vector<unsigned char> img8bit(img.size() * 3);
    for (size_t i = 0; i < img.size(); i++) {
        vec3 rgb = img[i];
        rgb = linear_to_nonlinear(rgb);
        unsigned char r = float_to_byte(rgb.x());
        unsigned char g = float_to_byte(rgb.y());
        unsigned char b = float_to_byte(rgb.z());
        img8bit[3 * i + 0] = r;
        img8bit[3 * i + 1] = g;
        img8bit[3 * i + 2] = b;
    }
    return img8bit;
}

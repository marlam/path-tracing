#pragma once

#include <vector>
#include <string>

#include "texture.hpp"
#include "math.hpp"

#include "stb_image.h"

class TextureImage : public Texture
{
public:
    int width, height;
    std::vector<vec3> img;

    TextureImage(const std::string& fileName, bool linearizeLDR = true)
    {
        stbi_ldr_to_hdr_gamma(linearizeLDR ? 2.2f : 1.0f);
        stbi_set_flip_vertically_on_load(1);
        int w, h, n;
        float* data = stbi_loadf(fileName.c_str(), &w, &h, &n, 3);
        if (!data) {
            fprintf(stderr, "cannot load texture %s\n", fileName.c_str());
            abort();
        }
        width = w;
        height = h;
        img.resize(width * height);
        for (int i = 0; i < width * height; i++)
            img[i] = vec3(data[3*i+0], data[3*i+1], data[3*i+2]);
        stbi_image_free(data);
    }

    TextureImage(const std::vector<vec3>& img, int w, int h)
        : width(w), height(h), img(img)
    {
    }

    vec3 value(int x, int y) const
    {
        return img[y * width + x];
    }

    virtual vec3 value(const vec2& texcoord, float /* time */) const override
    {
        // use only the fractional part of the texture coordinate so that
        // it is in [0,1) and no over- or underflow can occur
        float u = fract(texcoord.x());
        float v = fract(texcoord.y());
        // bilinear interpolation
        float s = u * width - 0.5f;
        if (s < 0.0f)
            s = 0.0f;
        float t = v * height - 0.5f;
        if (t < 0.0f)
            t = 0.0f;
        int x0 = s;
        int y0 = t;
        int x1 = x0 + 1;
        if (x1 >= width)
            x1 = width - 1;
        int y1 = y0 + 1;
        if (y1 >= height)
            y1 = height - 1;
        float alpha = s - x0;
        float beta = t - y0;
        vec3 a = mix(value(x0, y0), value(x1, y0), alpha);
        vec3 b = mix(value(x0, y1), value(x1, y1), alpha);
        return mix(a, b, beta);
    }
};

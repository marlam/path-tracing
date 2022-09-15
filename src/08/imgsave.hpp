#pragma once

#include <vector>
#include <iostream>
#include <fstream>

#include "math.hpp"

// Save a floating point image in PFM format
bool saveImageAsPfm(const std::string& fileName, const std::vector<vec3>& img, int width, int height)
{
    const float scaleFactor = 1.0f;
    std::ofstream ofs(fileName, std::ofstream::binary);
    std::string pfmHeader = std::string("PF\n") + std::to_string(width) + ' ' + std::to_string(height) + "\n-" + std::to_string(scaleFactor) + '\n';
    ofs << pfmHeader;
    ofs.write(reinterpret_cast<const char*>(img.data()), img.size() * sizeof(vec3));
    ofs.flush();
    bool ok = ofs.good();
    ofs.close();
    return ok;
}

// Save an 8 bit image in PPM format
bool saveImageAsPPM(const std::string& fileName, const std::vector<unsigned char>& img, int width, int height)
{
    std::ofstream ofs(fileName, std::ofstream::binary);
    std::string ppmHeader = std::string("P6\n") + std::to_string(width) + ' ' + std::to_string(height) + "\n" + "255\n";
    ofs << ppmHeader;
    for (int y = 0; y < height; y++) {
        int line = height - 1 - y;
        int offset = line * width * 3;
        ofs.write(reinterpret_cast<const char*>(img.data() + offset), width * 3 * sizeof(unsigned char));
    }
    ofs.flush();
    bool ok = ofs.good();
    ofs.close();
    return ok;
}

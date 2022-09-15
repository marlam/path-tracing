#include <vector>

#include "color.hpp"
#include "math.hpp"
#include "imgsave.hpp"

int main(void)
{
    // The image: RGB values per pixel, floating point
    int width = 1024;
    int height = 1024;
    std::vector<vec3> img(width * height);

    // Tile configuration
    int tileSize = 64;
    int tilesX = width / tileSize;
    int tilesY = height / tileSize;

    // Gather tiles into image
    for (int t = 0; t < tilesX * tilesY; t++) {
        std::ifstream ifs("tile-" + std::to_string(t) + ".raw", std::ofstream::binary);
        int myTileY = t / tilesX;
        int myTileX = t % tilesX;
        for (int ty = 0; ty < tileSize; ty++) {
            int y = myTileY * tileSize + ty;
            int x = myTileX * tileSize;
            ifs.read(reinterpret_cast<char*>(img.data())
                    + (y * width + x) * sizeof(vec3),
                    tileSize * sizeof(vec3));
        }
        ifs.close();
    }

    // Postprocess and save image
    saveImageAsPfm("image.pfm", img, width, height);
    uniformRationalQuantization(img, 250.0f, 32.0f);
    saveImageAsPPM("image.ppm", to8Bit(img), width, height);

    return 0;
}

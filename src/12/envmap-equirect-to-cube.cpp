#include "envmap_equirect.hpp"
#include "texture_image.hpp"
#include "color.hpp"
#include "imgsave.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(void)
{
    int w = 1024;
    int h = 1024;

    TextureImage img("kloofendal_48d_partly_cloudy_4k.hdr");
    EnvMapEquiRect equirect(&img);

    const char* sideName[6] = {
        "posx", "negx", "posy", "negy", "posz", "negz"
    };

    std::vector<vec3> sideImg(w * h);
    for (int side = 0; side < 6; side++) {
        for (int y = 0; y < h; y++) {
            float yf = 2.0f * (y + 0.5f) / h - 1.0f;
            for (int x = 0; x < w; x++) {
                float xf = 2.0f * (x + 0.5f) / w - 1.0f;
                vec3 d;
                if (side == 0) {
                    d = vec3(+1.0f, yf, -xf);
                } else if (side == 1) {
                    d = vec3(-1.0f, yf, xf);
                } else if (side == 2) {
                    d = vec3(xf, +1.0f, -yf);
                } else if (side == 3) {
                    d = vec3(xf, -1.0f, yf);
                } else if (side == 4) {
                    d = vec3(xf, yf, +1.0f);
                } else {
                    d = vec3(-xf, yf, -1.0f);
                }
                d = normalize(d);
                vec3 color = equirect.value(d, 0.0f);
                sideImg[y * w + x] = color;
            }
        }
        saveImageAsPfm(std::string(sideName[side]) + ".pfm", sideImg, w, h);
        saveImageAsPPM(std::string(sideName[side]) + ".ppm", to8Bit(sideImg), w, h);
    }
    return 0;
}

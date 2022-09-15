#include <vector>
#include <limits>

#include "math.hpp"
#include "ray.hpp"
#include "animation.hpp"
#include "animation_constant.hpp"
#include "camera.hpp"
#include "prng.hpp"
#include "sampler.hpp"
#include "surface_sphere.hpp"
#include "surface_triangle.hpp"
#include "material_twosided.hpp"
#include "material_lambertian.hpp"
#include "material_light.hpp"
#include "material_mirror.hpp"
#include "material_glass.hpp"
#include "material_phong.hpp"
#include "texture_constant.hpp"
#include "texture_image.hpp"
#include "scene.hpp"
#include "mesh.hpp"
#include "import.hpp"
#include "color.hpp"
#include "imgsave.hpp"
#include "bvh.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// Compute the radiance for one path sample
vec3 pathSample(const Scene& scene, const Ray& startRay, Prng& prng)
{
    const float MinHitDistance = 0.0001f;
    const float MaxHitDistance = std::numeric_limits<float>::max();
    const int MaxPathSegments = 128;

    vec3 radiance(0.0f);
    vec3 throughput(1.0f);
    Ray ray = startRay;
    for (int segment = 0; segment < MaxPathSegments; segment++) {
        HitRecord hr = scene.bvh.hit(ray, MinHitDistance, MaxHitDistance);
        if (!hr.haveHit)
            break;
        // scatter the ray at the hit point
        ScatterRecord sr = hr.material->scatter(ray, hr, prng);
        // add radiance emitted at this intersection
        radiance += throughput * hr.material->Le(hr, -ray.direction);
        if (sr.type == ScatterNone)
            break;
        // update throughput
        throughput *= sr.attenuation;
        throughput /= sr.p;
        // update the ray for the next path segment
        ray = Ray(hr.position, sr.direction, ray.time);
    }

    return radiance;
}

// Path Tracing main loops
int main(void)
{
    // The image: RGB values per pixel, floating point
    int width = 1024;
    int height = 1024;
    std::vector<vec3> img(width * height);
    int spp = 1024;

    // Camera and scene
    Scene scene;
    if (!importIntoScene(scene, "CornellBox-Original.obj"))
        return 1;
    if (true) {
        Animation* teapotTransformation =
            scene.take(new AnimationConstant(Transformation(
                            vec3(0.3f, 0.75f, 0.35f), quat::null(), vec3(0.3f))));
        if (!importIntoScene(scene, "teapot.obj", teapotTransformation))
            return 1;
    }
    scene.buildBVH(0.0f, 0.0f);
    AnimationConstant camAnim(Transformation(vec3(0.0f, 1.0f, 3.2f), vec3(0.0f, 1.0f, -1.0f)));
    Camera camera(radians(50.0f), float(width) / height, &camAnim);

    // Loop over pixels in the image
    #pragma omp parallel for
    for (int pixel = 0; pixel < width * height; pixel++) {
        // Radom number generator per pixel (so that it works with parallel threads)
        Prng prng(pixel + 42);
        // Get pixel x, y from linear index
        int y = pixel / width;
        int x = pixel % width;
        // Initialize pixel
        img[y * width + x] = vec3(0.0f);
        // Add samples
        for (int i = 0; i < spp; i++) {
            float p = (x + prng.in01()) / width;
            float q = (y + prng.in01()) / height;
            Ray ray = camera.getRay(p, q, 0.0f, 0.0f, prng);
            img[y * width + x] += pathSample(scene, ray, prng);
        }
        // Normalize
        img[y * width + x] /= spp;
    }

    // Save the HDR image
    saveImageAsPfm("image.pfm", img, width, height);

    // Apply dynamic range reduction
    uniformRationalQuantization(img, 250.0f, 32.0f);

    // Save the SDR image
    saveImageAsPPM("image.ppm", to8Bit(img), width, height);

    return 0;
}

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
#include "bvh.hpp"
#include "imgsave.hpp"

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

        // Russian Roulette
        float maxThroughput = std::max(throughput.x(), std::max(throughput.y(), throughput.z()));
        if (maxThroughput < 1.0f && segment >= 5) {
            float q = 1.0f - maxThroughput; // probability to cancel the path
            if (q > 0.95f)
                q = 0.95f;
            if (prng.in01() < q)
                break; // cancel
            float rrWeight = 1.0f / (1.0f - q);
            throughput *= rrWeight;
        }
    }

    return radiance;
}

// Path Tracing main loops
int main(void)
{
    // The image: RGB values per pixel, floating point
    int width = 800;
    int height = 600;
    std::vector<vec3> img(width * height);
    int sqrtSpp = 8;

    // Camera and scene
#if 0
    Scene scene;
    Texture* leftTextureKd = scene.take(new TextureConstant(vec3(0.5f, 0.1f, 0.1f)));
    Texture* leftTextureKs = scene.take(new TextureConstant(vec3(0.5f, 0.5f, 0.5f)));
    Texture* leftTextureS = scene.take(new TextureConstant(vec3(30.0f)));
    Material* leftMaterial = scene.take(new MaterialPhong(leftTextureKd, leftTextureKs, leftTextureS));
    scene.take(new SurfaceSphere(vec3(-1.025f, 0.55f, -5.0f), 1.0f, leftMaterial));
    Texture* rightTexture = scene.take(new TextureConstant(vec3(1.0f)));
    Material* rightMaterial = scene.take(new MaterialMirror(rightTexture));
    scene.take(new SurfaceSphere(vec3(+1.025f, 0.55f, -5.0f), 1.0f, rightMaterial));
    Material* centerMaterial = scene.take(new MaterialGlass(vec3(0.4f, 0.0f, 0.4f), 1.5f));
    scene.take(new SurfaceSphere(vec3(0.0f, 0.3f, -4.0f), 0.5f, centerMaterial));
    Texture* groundTexture = scene.take(new TextureConstant(vec3(0.5f, 0.5f, 0.5f)));
    Material* groundMaterial = scene.take(new MaterialLambertian(groundTexture));
    scene.take(new SurfaceSphere(vec3(0.0f, -100.0f, -5.0f), 99.5f, groundMaterial));
    Material* lightMaterial = scene.take(new MaterialLight(vec3(1.0f, 1.0f, 1.0f)));
    scene.take(new SurfaceSphere(vec3(0.0f, 15.0f, -3.0f), 10.0f, lightMaterial));
    Camera camera(radians(60.0f), float(width) / height);
#else
    Scene scene;
    if (!importIntoScene(scene, "CornellBox-Original.obj"))
        return 1;
    AnimationConstant camAnim(Transformation(vec3(0.0f, 1.0f, 3.2f), vec3(0.0f, 1.0f, -1.0f)));
    Camera camera(radians(50.0f), float(width) / height, &camAnim);
#endif
    scene.buildBVH(0.0f, 0.0f);

    // Loop over pixels in the image
    #pragma omp parallel for schedule(dynamic)
    for (int pixel = 0; pixel < width * height; pixel++) {
        // Radom number generator per pixel (so that it works with parallel threads)
        Prng prng(pixel + 42);
        // Get pixel x, y from linear index
        int y = pixel / width;
        int x = pixel % width;
        // Initialize pixel
        img[y * width + x] = vec3(0.0f);
        // Add stratified samples
        for (int i = 0; i < sqrtSpp; i++) {
            for (int j = 0; j < sqrtSpp; j++) {
                float sp = (i + prng.in01()) / sqrtSpp;
                float sq = (j + prng.in01()) / sqrtSpp;
                float p = (x + sp) / width;
                float q = (y + sq) / height;
                Ray ray = camera.getRay(p, q, 0.0f, 0.0f, prng);
                img[y * width + x] += pathSample(scene, ray, prng);
            }
        }
        // Normalize
        img[y * width + x] /= sqrtSpp * sqrtSpp;
    }

    // Save the image
    saveImageAsPfm("image.pfm", img, width, height);
    uniformRationalQuantization(img, 250.0f, 32.0f);
    saveImageAsPPM("image.ppm", to8Bit(img), width, height);

    return 0;
}

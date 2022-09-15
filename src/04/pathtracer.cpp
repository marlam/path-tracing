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
#include "material_twosided.hpp"
#include "material_lambertian.hpp"
#include "material_light.hpp"
#include "material_mirror.hpp"
#include "material_glass.hpp"
#include "material_phong.hpp"
#include "texture_constant.hpp"
#include "texture_image.hpp"
#include "imgsave.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


// Hit the scene with a ray
HitRecord hit(const std::vector<Surface*>& scene, const Ray& ray)
{
    const float MinHitDistance = 0.0001f;
    const float MaxHitDistance = std::numeric_limits<float>::max();

    float amin = MinHitDistance;
    float amax = MaxHitDistance;
    float nearestA = amax;
    HitRecord hr;
    for (size_t i = 0; i < scene.size(); i++) {
        HitRecord tmpHr = scene[i]->hit(ray, amin, amax);
        if (tmpHr.haveHit && tmpHr.a < nearestA) {
            hr = tmpHr;
            amax = hr.a;
        }
    }
    return hr;
}

// Compute the radiance for one path sample
vec3 pathSample(const std::vector<Surface*>& scene, const Ray& startRay, Prng& prng)
{
    const int MaxPathSegments = 128;

    vec3 radiance(0.0f);
    vec3 throughput(1.0f);
    Ray ray = startRay;
    for (int segment = 0; segment < MaxPathSegments; segment++) {
        HitRecord hr = hit(scene, ray);
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
    int width = 800;
    int height = 600;
    std::vector<vec3> img(width * height);
    int spp = 1024;

    // Camera and scene
#if 0
    // Demo scene 1: three spheres
    Camera camera(radians(60.0f), float(width) / height);
    TextureConstant leftTextureKd(vec3(0.5f, 0.1f, 0.1f));
    TextureConstant leftTextureKs(vec3(0.5f, 0.5f, 0.5f));
    TextureConstant leftTextureS(vec3(30.0f));
    MaterialPhong leftMaterial(&leftTextureKd, &leftTextureKs, &leftTextureS);
    SurfaceSphere leftSphere(vec3(-1.025f, 0.55f, -5.0f), 1.0f, &leftMaterial);
    TextureConstant rightTexture(vec3(1.0f));
    MaterialMirror rightMaterial(&rightTexture);
    SurfaceSphere rightSphere(vec3(+1.025f, 0.55f, -5.0f), 1.0f, &rightMaterial);
    MaterialGlass centerMaterial(vec3(0.4f, 0.0f, 0.4f), 1.5f);
    SurfaceSphere centerSphere(vec3(0.0f, 0.3f, -4.0f), 0.5f, &centerMaterial);
    TextureConstant groundTexture(vec3(0.5f, 0.5f, 0.5f));
    MaterialLambertian groundMaterial(&groundTexture);
    SurfaceSphere groundSphere(vec3(0.0f, -100.0f, -5.0f), 99.5f, &groundMaterial);
    MaterialLight lightMaterial(vec3(1.0f, 1.0f, 1.0f));
    SurfaceSphere lightSphere(vec3(0.0f, 15.0f, -3.0f), 10.0f, &lightMaterial);
    std::vector<Surface*> scene;
    scene.push_back(&leftSphere);
    scene.push_back(&rightSphere);
    scene.push_back(&centerSphere);
    scene.push_back(&groundSphere);
    scene.push_back(&lightSphere);
#else
    // Demo scene 2: a random set of spheres,
    // modeled after the Ray Tracing In One Weekend scene
    Prng scenePrng(1);
    std::vector<Texture*> textures;
    std::vector<Material*> materials;
    std::vector<Surface*> surfaces;
    Material* material;
    Surface* surface;

    Texture* texGround = new TextureConstant(vec3(0.5f));
    textures.push_back(texGround);
    material = new MaterialLambertian(texGround);
    materials.push_back(material);
    surface = new SurfaceSphere(vec3(0.0f, -1000.0f, 0.0f), 1000.0f, material);
    surfaces.push_back(surface);
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float chooseMaterial = scenePrng.in01();
            vec3 center(a + 0.9f * scenePrng.in01(), 0.2f, b + 0.9f * scenePrng.in01());
            if (length(center - vec3(4.0f, 0.2f, 0.0f)) > 0.9f) {
                if (chooseMaterial < 0.4f) { // lambertian
                    Texture* texKd = new TextureConstant(vec3(
                                scenePrng.in01() * scenePrng.in01(),
                                scenePrng.in01() * scenePrng.in01(),
                                scenePrng.in01() * scenePrng.in01()));
                    textures.push_back(texKd);
                    material = new MaterialLambertian(texKd);
                    materials.push_back(material);
                } else if (chooseMaterial < 0.8f) { // phong
                    float kDFactor = 0.2f + 0.6f * scenePrng.in01();
                    float kSFactor = 1.0f - kDFactor;
                    Texture* texKd = new TextureConstant(kDFactor * vec3(
                                scenePrng.in01() * scenePrng.in01(),
                                scenePrng.in01() * scenePrng.in01(),
                                scenePrng.in01() * scenePrng.in01()));
                    textures.push_back(texKd);
                    Texture* texKs = new TextureConstant(kSFactor * vec3(scenePrng.in01()));
                    textures.push_back(texKs);
                    Texture* texS = new TextureConstant(vec3(120.0f * scenePrng.in01()));
                    textures.push_back(texS);
                    material = new MaterialPhong(texKd, texKs, texS);
                    materials.push_back(material);
                } else if (chooseMaterial < 0.9f) { // mirror
                    Texture* texC = new TextureConstant(vec3(0.5f) + 0.5f * vec3(
                                scenePrng.in01(), scenePrng.in01(), scenePrng.in01()));
                    textures.push_back(texC);
                    material = new MaterialMirror(texC);
                    materials.push_back(material);
                } else { // glass
                    material = new MaterialGlass(5.0f * vec3(
                                scenePrng.in01(),
                                scenePrng.in01(),
                                scenePrng.in01()), 1.5f);
                    materials.push_back(material);
                }
                surface = new SurfaceSphere(center, 0.2f, material);
                surfaces.push_back(surface);
            }
        }
    }
    material = new MaterialGlass(vec3(0.0f, 0.2f, 0.2f), 1.5f);
    materials.push_back(material);
    surface = new SurfaceSphere(vec3(0.0f, 1.0f, 0.0f), 1.0f, material);
    surfaces.push_back(surface);
    Texture* texKd = new TextureConstant(vec3(0.4f, 0.2f, 0.1f));
    textures.push_back(texKd);
    material = new MaterialLambertian(texKd);
    materials.push_back(material);
    surface = new SurfaceSphere(vec3(-4.0f, 1.0f, 0.0f), 1.0f, material);
    surfaces.push_back(surface);
    Texture* texC = new TextureConstant(vec3(0.7f, 0.6f, 0.5f));
    textures.push_back(texC);
    material = new MaterialMirror(texC);
    materials.push_back(material);
    surface = new SurfaceSphere(vec3(4.0f, 1.0f, 0.0f), 1.0f, material);
    surfaces.push_back(surface);
    material = new MaterialLight(vec3(1.0f, 1.0f, 1.0f));
    materials.push_back(material);
    material = new MaterialTwoSided(nullptr, material);
    materials.push_back(material);
    surface = new SurfaceSphere(vec3(0.0f, 0.0f, 0.0f), 2000.0f, material);
    surfaces.push_back(surface);

    const std::vector<Surface*>& scene = surfaces;
    AnimationConstant camAnim(Transformation(vec3(10.0f, 2.0f, 3.0f), vec3(0.0f, 0.0f, -1.0f)));
    Camera camera(radians(30.0f), float(width) / height, &camAnim);
#endif

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

    // Save the image
    saveImageAsPfm("image.pfm", img, width, height);

    return 0;
}

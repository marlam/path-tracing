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
#include "texture_checker.hpp"
#include "texture_transformer.hpp"
#include "scene.hpp"
#include "mesh.hpp"
#include "import.hpp"
#include "color.hpp"
#include "bvh.hpp"
#include "imgsave.hpp"
#include "envmap.hpp"
#include "envmap_cube.hpp"
#include "envmap_equirect.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// The power heuristic for Multiple Importance Sampling
float powerHeuristicMIS(float f, float g)
{
    f *= f;
    g *= g;
    return (f + g > 0.0f ? f / (f + g) : 0.0f);
}

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
        if (!hr.haveHit) {
            if (scene.envMap)
                radiance += throughput * scene.envMap->value(ray.direction, ray.time);
            break;
        }
        // scatter the ray at the hit point
        ScatterRecord sr = hr.material->scatter(ray, hr, prng);
        // add radiance emitted at this intersection
        radiance += throughput * hr.material->Le(hr, -ray.direction);
        if (sr.type == ScatterNone)
            break;
        // compute throughput for next segment, but keep the current one
        vec3 nextThroughput = throughput * sr.attenuation / sr.p;

        // sample light source directly for MIS
        if (sr.type == ScatterRandom && scene.lights.size() > 0) {
            // update the throughput for the next path segment
            float lightsP = 0.0f;
            for (size_t i = 0; i < scene.lights.size(); i++)
                lightsP += scene.lights[i]->p(Ray(hr.position, sr.direction, ray.time));
            lightsP /= scene.lights.size();
            nextThroughput *= powerHeuristicMIS(sr.p, lightsP);
            // choose a light source randomly
            size_t lightIndex = prng.in01() * scene.lights.size();
            // get direction to it
            vec3 lightDir = scene.lights[lightIndex]->direction(hr.position, ray.time, prng);
            // get the pdf value for this direction
            float lightDirP = 0.0f;
            for (size_t i = 0; i < scene.lights.size(); i++)
                lightDirP += scene.lights[i]->p(Ray(hr.position, lightDir, ray.time));
            lightDirP /= scene.lights.size();
            // avoid corner cases where the lightDirP is 0
            if (lightDirP > 0.0f) {
                // get information about a ray going from our current hit point in this direction
                ScatterRecord lightSR = hr.material->scatterToDirection(ray, hr, lightDir);
                // check if the direction is possible
                if (lightSR.p > 0.0f) {
                    // shoot a ray from our current hit point in this direction
                    Ray lightRay(hr.position, lightDir, ray.time);
                    HitRecord lightHR = scene.bvh.hit(lightRay, MinHitDistance, MaxHitDistance);
                    // check if we hit the hot spot we chose
                    if (lightHR.haveHit && lightHR.surface == scene.lights[lightIndex]) {
                        // add the contribution of the hot spot using the power heuristic weight
                        float weight = powerHeuristicMIS(lightDirP, lightSR.p);
                        radiance += throughput * lightSR.attenuation / lightDirP * weight *
                            lightHR.material->Le(lightHR, -lightRay.direction);
                    }
                }
            }
        }

        // update throughput and ray for the next path segment
        throughput = nextThroughput;
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
    int width = 1920 / 2;
    int height = 1080 / 2;
    std::vector<vec3> img(width * height);
    int sqrtSpp = 16;

    // Camera and scene
    Scene scene;
#if 0
    Texture* leftTexture = scene.take(new TextureConstant(vec3(1.0f, 0.2f, 0.2f)));
    Material* leftMaterial = scene.take(new MaterialLambertian(leftTexture));
    scene.take(new SurfaceSphere(vec3(-1.5f, 0.55f, -5.0f), 1.0f, leftMaterial));
    Texture* rightTexture = scene.take(new TextureConstant(vec3(0.2f, 1.0f, 0.2f)));
    Material* rightMaterial = scene.take(new MaterialLambertian(rightTexture));
    scene.take(new SurfaceSphere(vec3(+1.5f, 0.55f, -5.0f), 1.0f, rightMaterial));
    Texture* centerTexture = scene.take(new TextureImage("earth.jpg"));
    Material* centerMaterial = scene.take(new MaterialLambertian(centerTexture));
    Transformation centerTrans;
    centerTrans.translate(vec3(0.0f, 0.55f, -2.5f));
    centerTrans.rotate(quat(radians(-45.0f), vec3(1.0f, 0.0f, 0.0f)));
    Animation* centerAnim = scene.take(new AnimationConstant(centerTrans));
    scene.take(new SurfaceSphere(vec3(0.0f), 1.0f, centerMaterial, centerAnim));
    Texture* groundTexture = scene.take(new TextureConstant(vec3(0.5f, 0.5f, 0.5f)));
    Material* groundMaterial = scene.take(new MaterialLambertian(groundTexture));
    scene.take(new SurfaceSphere(vec3(0.0f, -100.0f, -5.0f), 99.5f, groundMaterial));
    Material* lightMaterialOneSided = scene.take(new MaterialLight(vec3(1.0f, 1.0f, 1.0f)));
    Material* lightMaterial = scene.take(new MaterialTwoSided(nullptr, lightMaterialOneSided));
    scene.take(new SurfaceSphere(vec3(0.0f, 0.0f, 0.0f), 50.0f, lightMaterial));
    Transformation camTrans(vec3(0.0f, 0.5f, 0.0f));
    Animation* camAnim = scene.take(new AnimationConstant(camTrans));
    Camera camera(radians(60.0f), float(width) / height, camAnim);
#else
    // a basic quad
    std::vector<vec3> quadPos;
    quadPos.push_back(vec3(-1.0f, -1.0f, 0.0f));
    quadPos.push_back(vec3(+1.0f, -1.0f, 0.0f));
    quadPos.push_back(vec3(-1.0f, +1.0f, 0.0f));
    quadPos.push_back(vec3(+1.0f, +1.0f, 0.0f));
    std::vector<vec3> quadNrm;
    quadNrm.push_back(vec3(0.0f, 0.0f, +1.0f));
    quadNrm.push_back(vec3(0.0f, 0.0f, +1.0f));
    quadNrm.push_back(vec3(0.0f, 0.0f, +1.0f));
    quadNrm.push_back(vec3(0.0f, 0.0f, +1.0f));
    std::vector<vec2> quadTc;
    quadTc.push_back(vec2(0.0f, 0.0f));
    quadTc.push_back(vec2(1.0f, 0.0f));
    quadTc.push_back(vec2(0.0f, 1.0f));
    quadTc.push_back(vec2(1.0f, 1.0f));
    std::vector<unsigned int> quadInd;
    quadInd.push_back(0);
    quadInd.push_back(1);
    quadInd.push_back(2);
    quadInd.push_back(1);
    quadInd.push_back(3);
    quadInd.push_back(2);
    // the floor
    Texture* floorTexture = scene.take(new TextureChecker(
                scene.take(new TextureConstant(vec3(0.6f))),
                scene.take(new TextureConstant(vec3(0.4f))), 200, 200));
    Material* floorMaterial = scene.take(new MaterialLambertian(floorTexture));
    Transformation floorTransformation(vec3(0.0f), quat(radians(-90.0f), vec3(1.0f, 0.0f, 0.0f)), vec3(20.0f));
    Animation* floorAnimation = scene.take(new AnimationConstant(floorTransformation));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, floorMaterial, floorAnimation));
    // the first object
    Texture* firstObjectTexture = scene.take(new TextureConstant(vec3(1.0f, 1.0f, 1.0f)));
    Material* firstObjectMaterial = scene.take(new MaterialMirror(firstObjectTexture));
    scene.take(new SurfaceSphere(vec3(-1.5f, 1.0f, 0.0f), 1.0f, firstObjectMaterial));
    // the second object
    Texture* secondObjectTextureKd = scene.take(new TextureConstant(vec3(0.4f, 0.1f, 0.1f)));
    Texture* secondObjectTextureKs = scene.take(new TextureConstant(vec3(0.6f, 0.6f, 0.6f)));
    Texture* secondObjectTextureS = scene.take(new TextureConstant(vec3(500.0f)));
    Material* secondObjectMaterial = scene.take(new MaterialPhong(secondObjectTextureKd, secondObjectTextureKs, secondObjectTextureS));
    scene.take(new SurfaceSphere(vec3(+1.5f, 1.0f, 0.0f), 1.0f, secondObjectMaterial));
    // the environment map
# if 1
    Texture* map = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small.jpg"));
    scene.take(new EnvMapEquiRect(map));
# else
    Texture* mapPosX = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small_posx.jpg"));
    Texture* mapNegX = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small_negx.jpg"));
    Texture* mapPosY = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small_posy.jpg"));
    Texture* mapNegY = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small_negy.jpg"));
    Texture* mapPosZ = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small_posz.jpg"));
    Texture* mapNegZ = scene.take(new TextureImage("kloofendal_48d_partly_cloudy_small_negz.jpg"));
    scene.take(new EnvMapCube(mapPosX, mapNegX, mapPosY, mapNegY, mapPosZ, mapNegZ));
# endif
    // the camera
    Transformation camTrans(vec3(0.0f, 5.0f, 5.0f), vec3(0.0f, 1.0f, 0.0f));
    Animation* camAnim = scene.take(new AnimationConstant(camTrans));
    Camera camera(radians(35.0f), float(width) / height, camAnim);
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
    //uniformRationalQuantization(img, 100.0f, 32.0f);
    saveImageAsPPM("image.ppm", to8Bit(img), width, height);

    return 0;
}

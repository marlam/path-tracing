#include <vector>
#include <cfloat>

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
    int width = 800;
    int height = 600;
    std::vector<vec3> img(width * height);
    int sqrtSpp = 40;

    // The scene and camera
    Scene scene;
#if 1
    Transformation sponzaTransf(
            vec3(0.0f), // translation
            quat(radians(90.0f), vec3(0.0f, 1.0f, 0.0f)), // rotation
            vec3(0.01f)); // scaling
    Animation* sponzaAnim = scene.take(new AnimationConstant(sponzaTransf));
    if (!importIntoScene(scene, "crytek-sponza/sponza.obj", sponzaAnim)) {
        return 1;
    }
    // Add an environment map light source
    Texture* map = scene.take(new TextureConstant(vec3(6.0f)));
    scene.take(new EnvMapEquiRect(map));
# if 1
    // center view
    Transformation camTransf(vec3(0.0f, 1.7f, 0.0f), vec3(0.0f, 1.7f, -1.0f));
    Animation* camAnim = scene.take(new AnimationConstant(camTransf));
    Camera camera(radians(70.0f), float(width) / height, camAnim);
# else
    // plant closeup
    Transformation camTransf(vec3(-0.93f, 1.7f, -1.82f),
            quat(radians(34.79f), vec3(-0.0877701f, -0.995764f, -0.0273793f)));
    Animation* camAnim = scene.take(new AnimationConstant(camTransf));
    Camera camera(radians(30.0f), float(width) / height, camAnim);
# endif
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

    Texture* texKd = scene.take(new TextureImage("cglogo-texture.png"));
    Texture* texKs = scene.take(new TextureConstant(vec3(0.5f)));
    Texture* texS = scene.take(new TextureConstant(vec3(120.0f)));
    Texture* texNormal = scene.take(new TextureImage("cglogo-normalmap1.png", false));
    Material* boxMat = scene.take(new MaterialPhong(texKd, texKs, texS, nullptr, texNormal));
    Transformation boxT;
    boxT.translate(vec3(-1.1f, 0.2f, 1.0f));
    boxT.rotate(quat(radians(50.0f), normalize(vec3(1.0f, 1.0f, 0.0f))));
    boxT.scale(vec3(0.7f));
    Transformation boxFrontT;
    boxFrontT.translate(vec3(0.0f, 0.0f, 1.0f));
    Animation* boxFrontAnim = scene.take(new AnimationConstant(boxT * boxFrontT));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, boxMat, boxFrontAnim));
    Transformation boxLeftT;
    boxLeftT.translate(vec3(-1.0f, 0.0f, 0.0f));
    boxLeftT.rotate(quat(radians(-90.0f), normalize(vec3(0.0f, 1.0f, 0.0f))));
    Animation* boxLeftAnim = scene.take(new AnimationConstant(boxT * boxLeftT));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, boxMat, boxLeftAnim));
    Transformation boxTopT;
    boxTopT.translate(vec3(0.0f, 1.0f, 0.0f));
    boxTopT.rotate(quat(radians(-90.0f), normalize(vec3(1.0f, 0.0f, 0.0f))));
    Animation* boxTopAnim = scene.take(new AnimationConstant(boxT * boxTopT));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, boxMat, boxTopAnim));

    Texture* sphereTexKd = scene.take(new TextureTransformer(texKd, vec2(6.0f, 3.0f), vec2(0.0f)));
    Texture* sphereTexNormal = scene.take(new TextureTransformer(texNormal, vec2(6.0f, 3.0f), vec2(0.0f)));
    Material* sphereMat = scene.take(new MaterialPhong(sphereTexKd, texKs, texS, nullptr, sphereTexNormal));
    Transformation sphereT(vec3(+1.1f, 0.0f, 0.0f), quat(radians(90.0f), vec3(0.0f, 1.0f, 0.0f)));
    Animation* sphereAnim = scene.take(new AnimationConstant(sphereT));
    scene.take(new SurfaceSphere(vec3(0.0f), 1.0f, sphereMat, sphereAnim));

    Material* lightMat = scene.take(new MaterialLight(vec3(3.0f)));
    Transformation lightT;
    lightT.translate(vec3(-2.0f, 3.0f, 7.0f));
    lightT.scale(vec3(2.0f));
    Animation* lightAnim = scene.take(new AnimationConstant(lightT));
    scene.take(new SurfaceSphere(vec3(0.0f), 1.0f, lightMat, lightAnim), true);

    Transformation camT = Transformation(vec3(-0.2f, 0.0f, 5.0f));
    Animation* camAnim = scene.take(new AnimationConstant(camT));
    Camera camera(radians(42.0f), float(width) / height, camAnim);
#endif

    // Loop over pixels in the image
    scene.buildBVH(0.0f, 0.0f);
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
    uniformRationalQuantization(img, 100.0f, 4.0f);
    saveImageAsPPM("image.ppm", to8Bit(img), width, height);

    return 0;
}

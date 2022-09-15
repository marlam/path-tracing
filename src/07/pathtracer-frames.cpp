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
    }

    return radiance;
}

class AnimationSphere : public Animation
{
public:
    Transformation T0;
    Transformation T1;

    AnimationSphere(int i)
    {
        Prng prng(123 + i);
        float x = -3.0f + 6.0f * prng.in01();
        float z = -2.0f - 6.0f * prng.in01();
        float y0 = 9.7f - 8.0f * prng.in01();
        float y1 = -9.7f + 8.0f * prng.in01();
        float s = 0.1f + prng.in01() * 0.2f;
        T0.translate(vec3(x, y0, z));
        T0.scale(vec3(s));
        T1.translate(vec3(x, y1, z));
        T1.scale(vec3(s));
    }

    virtual Transformation at(float t) const override
    {
        Transformation T = mix(T0, T1, t / 10.0f);
        return T;
    }
};

// Build the scene
void buildScene(Scene& scene)
{
    // a basic quad
    std::vector<vec3> quadPos;
    quadPos.push_back(vec3(-1.0f, -1.0f, 0.0f));
    quadPos.push_back(vec3(+1.0f, -1.0f, 0.0f));
    quadPos.push_back(vec3(+1.0f, +1.0f, 0.0f));
    quadPos.push_back(vec3(-1.0f, +1.0f, 0.0f));
    std::vector<vec3> quadNrm;
    std::vector<vec2> quadTc;
    std::vector<unsigned int> quadInd;
    quadInd.push_back(0);
    quadInd.push_back(1);
    quadInd.push_back(2);
    quadInd.push_back(0);
    quadInd.push_back(2);
    quadInd.push_back(3);

    // box material (except front side)
    Texture* boxTex = scene.take(new TextureConstant(vec3(0.6f)));
    Material* boxMat = scene.take(new MaterialLambertian(boxTex));

    // back, left, right, top, bottom sides of the box
    Transformation backT;
    backT.translate(vec3(0.0f, 0.0f, -10.0f));
    backT.scale(vec3(10.0f));
    Animation* backA = scene.take(new AnimationConstant(backT));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, boxMat, backA));
    Transformation leftT;
    leftT.translate(vec3(-10.0f, 0.0f, -5.0f));
    leftT.rotate(quat(radians(90.0f), vec3(0.0f, 1.0f, 0.0f)));
    leftT.scale(vec3(10.0f));
    Animation* leftA = scene.take(new AnimationConstant(leftT));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, boxMat, leftA));
    Transformation rightT;
    rightT.translate(vec3(+10.0f, 0.0f, -5.0f));
    rightT.rotate(quat(radians(-90.0f), vec3(0.0f, 1.0f, 0.0f)));
    rightT.scale(vec3(10.0f));
    Animation* rightA = scene.take(new AnimationConstant(rightT));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, boxMat, rightA));
    Transformation topT;
    topT.translate(vec3(0.0f, 10.0f, -5.0f));
    topT.rotate(quat(radians(90.0f), vec3(1.0f, 0.0f, 0.0f)));
    topT.scale(vec3(10.0f));
    Animation* topA = scene.take(new AnimationConstant(topT));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, boxMat, topA));
    Transformation bottomT;
    bottomT.translate(vec3(0.0f, -10.0f, -5.0f));
    bottomT.rotate(quat(radians(-90.0f), vec3(1.0f, 0.0f, 0.0f)));
    bottomT.scale(vec3(10.0f));
    Animation* bottomA = scene.take(new AnimationConstant(bottomT));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, boxMat, bottomA));

    // front side of the box (light source)
    Material* lightMat = scene.take(new MaterialLight(vec3(1.0f)));
    Transformation frontT;
    frontT.rotate(quat(radians(180.0f), vec3(0.0f, 1.0f, 0.0f)));
    frontT.scale(vec3(10.0f));
    Animation* frontA = scene.take(new AnimationConstant(frontT));
    scene.take(new Mesh(quadPos, quadNrm, quadTc, quadInd, lightMat, frontA));

    // some random falling spheres
    Prng prng(42);
    for (int i = 0; i < 100; i++) {
        float matP = prng.in01();
        Material* mat;
        if (matP < 0.1) {
            Texture* tex = scene.take(new TextureConstant(vec3(1.0f)));
            mat = new MaterialMirror(tex);
        } else if (matP < 0.2) {
            mat = new MaterialGlass(vec3(0.0f), 1.5f);
        } else {
            float kDFactor = 0.2f + 0.6f * prng.in01();
            float kSFactor = 1.0f - kDFactor;
            Texture* texKd = scene.take(new TextureConstant(kDFactor * vec3(
                            prng.in01() * prng.in01(),
                            prng.in01() * prng.in01(),
                            prng.in01() * prng.in01())));
            Texture* texKs = scene.take(new TextureConstant(kSFactor * vec3(prng.in01())));
            Texture* texS = scene.take(new TextureConstant(vec3(120.0f * prng.in01())));
            mat = new MaterialPhong(texKd, texKs, texS);
        }
        scene.take(mat);
        Animation* anim = scene.take(new AnimationSphere(i));
        scene.take(new SurfaceSphere(vec3(0.0f), 1.0f, mat, anim));
    }
}

// Path Tracing main loops
int main(int argc, char* argv[])
{
    // The image: RGB values per pixel, floating point
    int width = 1920 / 4;
    int height = 1080 / 4;
    std::vector<vec3> img(width * height);
    int spp = 2048 / 16;

    // The frame setup
    float fps = 25.0f;
    float frameDuration = 1.0f / fps;
    float totalDuration = 10.0f;
    int frames = totalDuration / frameDuration;
    int myFrame = -1;
    if (argc == 2) {
        myFrame = std::atoi(argv[1]);
        fprintf(stderr, "Rendering only frame %d\n", myFrame);
    }

    // Camera and scene
    Scene scene;
    buildScene(scene);
    Camera camera(radians(50.0f), float(width) / height);

    // Loop over the frames
    for (int frame = 0; frame < frames; frame++) {
        if (myFrame >= 0 && frame != myFrame)
            continue;

        float t0 = frame * frameDuration;
        float t1 = t0 + frameDuration;
        scene.buildBVH(t0, t1);

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
            // Add samples
            for (int i = 0; i < spp; i++) {
                float p = (x + prng.in01()) / width;
                float q = (y + prng.in01()) / height;
                Ray ray = camera.getRay(p, q, t0, t1, prng);
                img[y * width + x] += pathSample(scene, ray, prng);
            }
            // Normalize
            img[y * width + x] /= spp;
        }

        // Save the frame
        saveImageAsPPM("frame-" + std::to_string(frame) + ".ppm", to8Bit(img), width, height);
    }

    // Create a high quality video:
    // ffmpeg -i frame-%d.ppm -c:v libx265 -preset veryslow -crf 20 -vf format=yuv420p video.mp4

    return 0;
}

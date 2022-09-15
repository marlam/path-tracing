#include <vector>
#include <limits>

#include "math.hpp"
#include "ray.hpp"
#include "camera.hpp"
#include "prng.hpp"
#include "sampler.hpp"
#include "surface_sphere.hpp"
#include "material_lambertian.hpp"
#include "material_light.hpp"
#include "animation.hpp"
#include "animation_constant.hpp"
#include "imgsave.hpp"

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
    const int MaxPathSegments = 16;

    vec3 radiance(0.0f);
    vec3 throughput(1.0f);
    Ray ray = startRay;
    for (int segment = 0; segment < MaxPathSegments; segment++) {
        HitRecord hr = hit(scene, ray);
        if (!hr.haveHit)
            break;
        // add radiance emitted at this intersection
        radiance += throughput * hr.material->Le(hr, -ray.direction);
        if (hr.material->pathStopsHere())
            break;
        // get new random direction on hemisphere around hr.normal
        vec3 newDirection = Sampler::onUnitHemisphere(hr.normal, prng.onUnitSphere());
        // evaluate BRDF and the cos(theta) attenuation term
        vec3 brdf = hr.material->brdf(hr, -newDirection, -ray.direction);
        float cosTheta = dot(hr.normal, newDirection);
        throughput *= brdf * cosTheta;
        // divide by the probability of choosing the new direction
        throughput *= 2.0f * pi;
        // update the ray for the next path segment
        ray = Ray(hr.position, newDirection, ray.time);
    }

    return radiance;
}

// A simple animation for a sphere
class AnimationFallingSphere : public Animation
{
public:
    AnimationFallingSphere()
    {
    }

    virtual Transformation at(float t) const override
    {
        return Transformation(vec3(0.0f, -0.15f - t * 0.1f, -3.5f));
    }
};

// A camera animation
class AnimationCamera : public Animation
{
public:
    AnimationCamera()
    {
    }

    virtual Transformation at(float t) const override
    {
        vec3 center = vec3(0.0f, 0.0f, -5.0f);
        vec3 eye0 = vec3(0.0f, 0.0f, 0.0f);
        vec3 eye1 = vec3(0.0f, 3.0f, -3.0f);
        Transformation T0 = Transformation(eye0, center);
        Transformation T1 = Transformation(eye1, center);
        return mix(T0, T1, t);
    }
};

// Path Tracing main loops
int main(void)
{
    // The image: RGB values per pixel, floating point
    int width = 800;
    int height = 600;
    std::vector<vec3> img(width * height);
    int spp = 256;

    // The camera
    bool animateCamera = false;
    float animateCameraTime = 0.0f;
    AnimationCamera camAnim;
    Camera camera(radians(60.0f), float(width) / height, animateCamera ? &camAnim : nullptr);

    // The scene
    MaterialLambertian leftMaterial(vec3(1.0f, 0.2f, 0.2f));
    SurfaceSphere leftSphere(vec3(-1.025f, 0.55f, -5.0f), 1.0f, &leftMaterial);
    MaterialLambertian rightMaterial(vec3(0.2f, 1.0f, 0.2f));
    SurfaceSphere rightSphere(vec3(+1.025f, 0.55f, -5.0f), 1.0f, &rightMaterial);
    AnimationFallingSphere centerAnimation;
    MaterialLambertian centerMaterial(vec3(0.2f, 0.2f, 1.0f));
    SurfaceSphere centerSphere(vec3(0.0f, 0.0f, 0.0f), 0.25f, &centerMaterial, &centerAnimation);
    MaterialLambertian groundMaterial(vec3(0.5f, 0.5f, 0.5f));
    SurfaceSphere groundSphere(vec3(0.0f, -100.0f, -5.0f), 99.5f, &groundMaterial);
    MaterialLight lightMaterial(vec3(1.0f, 1.0f, 1.0f));
    SurfaceSphere lightSphere(vec3(0.0f, 0.0f, -5.0f), 50.0f, &lightMaterial);
    std::vector<Surface*> scene;
    scene.push_back(&leftSphere);
    scene.push_back(&rightSphere);
    scene.push_back(&centerSphere);
    scene.push_back(&groundSphere);
    scene.push_back(&lightSphere);

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
            Ray ray = camera.getRay(p, q,
                    animateCamera ? animateCameraTime : 0.0f,
                    animateCamera ? animateCameraTime : 1.0f,
                    prng);
            img[y * width + x] += pathSample(scene, ray, prng);
        }
        // Normalize
        img[y * width + x] /= spp;
    }

    // Save the image
    saveImageAsPfm("image.pfm", img, width, height);

    return 0;
}

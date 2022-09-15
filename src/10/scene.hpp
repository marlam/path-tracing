#pragma once

#include <memory>

#include "animation.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "surface.hpp"
#include "mesh.hpp"
#include "bvh.hpp"

class Scene
{
public:
    std::vector<std::unique_ptr<Animation>> animations;
    std::vector<std::unique_ptr<Texture>> textures;
    std::vector<std::unique_ptr<Material>> materials;
    std::vector<std::unique_ptr<Surface>> surfaces;
    std::vector<const Surface*> lights;
    std::vector<std::unique_ptr<Mesh>> meshes;
    BVHTreeLinear bvh;

    Scene()
    {
    }

    Animation* take(Animation* anim)
    {
        animations.push_back(std::unique_ptr<Animation>(anim));
        return anim;
    }

    Texture* take(Texture* tex)
    {
        textures.push_back(std::unique_ptr<Texture>(tex));
        return tex;
    }

    Material* take(Material* max)
    {
        materials.push_back(std::unique_ptr<Material>(max));
        return max;
    }

    Surface* take(Surface* surf, bool isLight = false)
    {
        surfaces.push_back(std::unique_ptr<Surface>(surf));
        if (isLight)
            lights.push_back(surf);
        return surf;
    }

    Mesh* take(Mesh* mesh, bool isLight = false)
    {
        meshes.push_back(std::unique_ptr<Mesh>(mesh));
        for (size_t i = 0; i < mesh->surfaces(); i++)
            take(mesh->createSurface(i), isLight);
        return mesh;
    }

    void buildBVH(float t0, float t1)
    {
        bvh.build(surfaces, t0, t1);
    }
};

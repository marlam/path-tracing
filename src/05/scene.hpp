#pragma once

#include <memory>

#include "animation.hpp"
#include "texture.hpp"
#include "material.hpp"
#include "surface.hpp"
#include "mesh.hpp"

class Scene
{
public:
    std::vector<std::unique_ptr<Animation>> animations;
    std::vector<std::unique_ptr<Texture>> textures;
    std::vector<std::unique_ptr<Material>> materials;
    std::vector<std::unique_ptr<Surface>> surfaces;
    std::vector<std::unique_ptr<Mesh>> meshes;

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

    Surface* take(Surface* surf)
    {
        surfaces.push_back(std::unique_ptr<Surface>(surf));
        return surf;
    }

    Mesh* take(Mesh* mesh)
    {
        meshes.push_back(std::unique_ptr<Mesh>(mesh));
        for (size_t i = 0; i < mesh->surfaces(); i++)
            take(mesh->createSurface(i));
        return mesh;
    }
};

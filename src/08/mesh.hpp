#pragma once

#include "animation.hpp"
#include "math.hpp"
#include "material.hpp"

class Mesh;

// helper function, defined in surface_triangle.hpp
// (but we cannot include that header because that in turn include this header)
Surface* createSurfaceTriangle(const Mesh& mesh, const unsigned int* indices);

class Mesh
{
public:
    std::vector<vec3> positions;
    std::vector<vec3> normals;          // may be empty
    std::vector<vec2> texcoords;        // may be empty
    std::vector<unsigned int> indices;
    const Material* material;
    const Animation* animation;

    // Create a new mesh.
    Mesh(const std::vector<vec3>& pos,
            const std::vector<vec3>& nrm,
            const std::vector<vec2>& tc,
            const std::vector<unsigned int>& ind,
            const Material* mat,
            const Animation* anim = nullptr) :
        positions(pos),
        normals(nrm),
        texcoords(tc),
        indices(ind),
        material(mat),
        animation(anim)
    {
    }

    // return the number of surfaces (triangles)
    size_t surfaces() const
    {
        return indices.size() / 3;
    }

    // create surface with index i (to be called by the Scene)
    Surface* createSurface(size_t i) const
    {
        return createSurfaceTriangle(*this, indices.data() + 3 * i);
    }
};

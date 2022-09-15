#pragma once

#include "animation.hpp"
#include "math.hpp"
#include "material.hpp"

class Mesh;

/* Compute tangents from positions, normals, and texcoords. */
inline std::vector<vec3> computeTangents(
        const std::vector<vec3>& positions,
        const std::vector<vec3>& normals,
        const std::vector<vec2>& texcoords,
        const std::vector<unsigned int> indices)
{
    size_t vertexCount = positions.size();
    size_t triangleCount = indices.size() / 3;
    std::vector<vec3> tangents(vertexCount, vec3(0.0f));
    for (size_t t = 0; t < triangleCount; t++) {
        unsigned int i0 = indices[3 * t + 0];
        unsigned int i1 = indices[3 * t + 1];
        unsigned int i2 = indices[3 * t + 2];
        const vec3& P0 = positions[i0];
        const vec3& P1 = positions[i1];
        const vec3& P2 = positions[i2];
        const vec2& tc0 = texcoords[i0];
        const vec2& tc1 = texcoords[i1];
        const vec2& tc2 = texcoords[i2];
        vec3 e1 = P1 - P0;
        vec3 e2 = P2 - P0;
        float s1 = tc1.x() - tc0.x();
        float t1 = tc1.y() - tc0.y();
        float s2 = tc2.x() - tc0.x();
        float t2 = tc2.y() - tc0.y();
        float det = (s1 * t2 - s2 * t1);
        if (std::abs(det) > std::numeric_limits<float>::epsilon()) {
            vec3 tp = 1.0f / det * (t2 * e1 - t1 * e2);
            tangents[i0] += tp;
            tangents[i1] += tp;
            tangents[i2] += tp;
        }
    }
    for (size_t i = 0; i < vertexCount; i++) {
        const vec3& n = normals[i];
        const vec3& tp = tangents[i];
        vec3 t(1.0f, 0.0f, 0.0f); // fall back to a valid tangent of length 1
        // We only have valid tp if we have valid texture coordinates:
        if (dot(tp, tp) > 0.0f) {
            // Gram-Schmidt orthonormalization:
            t = normalize(tp - dot(n, tp) * n);
        }
        tangents[i] = t;
    }
    return tangents;
}

// helper function, defined in surface_triangle.hpp
// (but we cannot include that header because that in turn include this header)
Surface* createSurfaceTriangle(const Mesh& mesh, const unsigned int* indices);

class Mesh
{
public:
    std::vector<vec3> positions;
    std::vector<vec3> normals;          // may be empty
    std::vector<vec2> texcoords;        // may be empty
    std::vector<vec3> tangents;         // may be empty
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
        if (normals.size() > 0 && texcoords.size() > 0) {
            tangents = computeTangents(positions, normals, texcoords, indices);
        }
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

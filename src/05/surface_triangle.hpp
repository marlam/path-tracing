#pragma once

#include "math.hpp"
#include "surface.hpp"
#include "animation.hpp"
#include "mesh.hpp"

class SurfaceTriangle : public Surface
{
public:
    const Mesh& mesh;
    const unsigned int* indices;

    SurfaceTriangle(const Mesh& mesh, const unsigned int* indices) :
        mesh(mesh), indices(indices)
    {
    }

    virtual HitRecord hit(const Ray& ray, float amin, float amax) const override
    {
        // get indices of this triangle
        unsigned int i0 = indices[0];
        unsigned int i1 = indices[1];
        unsigned int i2 = indices[2];

        // get vertex positions
        vec3 A = mesh.positions[i0];
        vec3 B = mesh.positions[i1];
        vec3 C = mesh.positions[i2];

        // transform if necessary
        Transformation T;
        if (mesh.animation) {
            T = mesh.animation->at(ray.time);
            A = T * A;
            B = T * B;
            C = T * C;
        }

        /* Möller-Trumbore ray/triangle intersection algorithm */

        // get relevant vectors
        const vec3& d = ray.direction;
        vec3 e1 = B - A;
        vec3 e2 = C - A;
        vec3 c2 = cross(d, e2);

        // compute first determinant, for early exit test
        float Dpre = dot(c2, e1);
        if (std::abs(Dpre) < std::numeric_limits<float>::epsilon()) {
            // ray and triangle are (nearly) parallel; no hit
            return HitRecord();
        }
        bool backside = (Dpre < 0.0f);
        float invD = 1.0f / Dpre;

        // compute remaining relevant vectors
        vec3 t = ray.origin - A;
        vec3 c1 = cross(t, e1);

        // compute barycentric coordinates
        float D2 = dot(c2, t);
        float u = D2 * invD;
        if (u < 0.0f || u > 1.0f) {
            // barycentric coordinate outside the triangle
            return HitRecord();
        }
        float D3 = dot(c1, d);
        float v = D3 * invD;
        if (v < 0.0f || u + v > 1.0f) {
            // barycentric coordinate outside the triangle
            return HitRecord();
        }

        // at this point we know we have a hit, but is it valid?
        float D1 = dot(c1, e2);
        float alpha = D1 * invD;
        if (alpha < amin || alpha > amax)
            return HitRecord();

        // a valid hit: use barycentric coordinates to interpolate vertex attributes
        float w = 1.0f - u - v;
        vec3 pos = ray.at(alpha);
        vec3 nrm;
        if (mesh.normals.size() > 0) {
            nrm = w * mesh.normals[i0] + u * mesh.normals[i1] + v * mesh.normals[i2];
            if (mesh.animation)
                nrm = T.rotation * nrm;
        } else {
            // no normals in the mesh; use the face normal
            nrm = cross(e1, e2);
        }
        nrm = normalize(nrm);
        if (backside)
            nrm = -nrm;
        vec2 tc;
        if (mesh.texcoords.size() > 0)
            tc = w * mesh.texcoords[i0] + u * mesh.texcoords[i1] + v * mesh.texcoords[i2];
        else
            tc = vec2(0.0f);

        return HitRecord(alpha, pos, nrm, tc, backside, mesh.material);
    }
};

Surface* createSurfaceTriangle(const Mesh& mesh, const unsigned int* indices)
{
    return new SurfaceTriangle(mesh, indices);
}

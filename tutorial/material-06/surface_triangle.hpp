#pragma once

#include "math.hpp"
#include "surface.hpp"
#include "animation.hpp"
#include "mesh.hpp"

class SurfaceTriangle : public Surface
{
private:
    static AABB aabb(const vec3& A, const vec3& B, const vec3& C)
    {
        return AABB(vec3(
                    std::min(A.x(), std::min(B.x(), C.x())),
                    std::min(A.y(), std::min(B.y(), C.y())),
                    std::min(A.z(), std::min(B.z(), C.z()))),
                    vec3(
                    std::max(A.x(), std::max(B.x(), C.x())),
                    std::max(A.y(), std::max(B.y(), C.y())),
                    std::max(A.z(), std::max(B.z(), C.z()))));
    }

public:
    const Mesh& mesh;
    const unsigned int* indices;

    SurfaceTriangle(const Mesh& mesh, const unsigned int* indices) :
        mesh(mesh), indices(indices)
    {
    }

    virtual AABB aabb(float t0, float t1) const override
    {
        // get indices of this triangle
        unsigned int i0 = indices[0];
        unsigned int i1 = indices[1];
        unsigned int i2 = indices[2];

        // get vertex positions
        vec3 A = mesh.positions[i0];
        vec3 B = mesh.positions[i1];
        vec3 C = mesh.positions[i2];

        if (!mesh.animation) {
            return aabb(A, B, C);
        } else {
            Transformation T0 = mesh.animation->at(t0);
            vec3 A0 = T0 * A;
            vec3 B0 = T0 * B;
            vec3 C0 = T0 * C;
            AABB box = aabb(A0, B0, C0);
            const int steps = 16;
            for (int i = 1; i < steps; i++) {
                float t = mix(t0, t1, i / (steps - 1.0f)); // end at t1!
                Transformation T = mesh.animation->at(t);
                vec3 At = T * A;
                vec3 Bt = T * B;
                vec3 Ct = T * C;
                box = merge(box, aabb(At, Bt, Ct));
            }
            return box;
        }
    }

    void getVerticesUntransformed(unsigned int& i0, unsigned int& i1, unsigned int& i2,
            vec3& A, vec3& B, vec3& C) const
    {
        // get indices of this triangle
        i0 = indices[0];
        i1 = indices[1];
        i2 = indices[2];

        // get vertex positions
        A = mesh.positions[i0];
        B = mesh.positions[i1];
        C = mesh.positions[i2];
    }

    void getVertices(float t, Transformation& T,
            unsigned int& i0, unsigned int& i1, unsigned int& i2,
            vec3& A, vec3& B, vec3& C) const
    {
        getVerticesUntransformed(i0, i1, i2, A, B, C);

        // transform if necessary
        if (mesh.animation) {
            T = mesh.animation->at(t);
            A = T * A;
            B = T * B;
            C = T * C;
        }
    }

    virtual HitRecord hit(const Ray& ray, float amin, float amax) const override
    {
        Transformation T;
        unsigned int i0, i1, i2;
        vec3 A, B, C;
        getVertices(ray.time, T, i0, i1, i2, A, B, C);

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

        return HitRecord(alpha, pos, nrm, tc, backside, this, mesh.material);
    }

    virtual vec3 direction(const vec3& origin, float t, Prng& prng) const override
    {
        // TODO
        return vec3(0.0f);
    }

    virtual float p(const Ray& ray) const override
    {
        // TODO
        return 0.0f;
    }
};

Surface* createSurfaceTriangle(const Mesh& mesh, const unsigned int* indices)
{
    return new SurfaceTriangle(mesh, indices);
}

#pragma once

// Vectors in tangent space are relative to the normal vector (0,0,1).
// Given a normal, tangent, and bitangent vector, we can transform
// vectors between this tangent space and world space.
class TangentSpace
{
public:
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;

    // Construct a world space reference from normal and tangent (both normalized)
    TangentSpace(const vec3& n, const vec3& t) :
        normal(n), tangent(t), bitangent(cross(n, t))
    {
    }

    // Construct a world space reference from just a normal
    TangentSpace(const vec3& n)
    {
        normal = n;
        // We have to fake a tangent: create some vector that is orthogonal to n.
        vec3 w;
        if (std::abs(n.x()) > std::abs(n.z()) && std::abs(n.y()) > std::abs(n.z())) {
            w = vec3(-n.y(), n.x(), 0.0f);
        } else if (std::abs(n.y()) > std::abs(n.x())) {
            w = vec3(0.0f, -n.z(), n.y());
        } else {
            w = vec3(-n.z(), 0.0f, n.x());
        }
        tangent = normalize(w);
        bitangent = cross(normal, tangent);
        // For a much more efficient and precise method in fewer lines of code, see
        // "Building an Orthonormal Basis, Revisited" by Duff et al., JCGT vol 6 no 1, 2017
    }

    // Transforms a vector v in tangent space to world space
    vec3 toWorldSpace(const vec3& v)
    {
        // TODO: Multiply the vector with the matrix consisting of the rows t, b, n
        return vec3(0.0f);
    }
};

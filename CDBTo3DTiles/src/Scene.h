#pragma once

#include "glm/glm.hpp"
#include "osg/Image"
#include <filesystem>
#include <optional>
#include <vector>

namespace CDBTo3DTiles {
struct AABB
{
    AABB();

    AABB(const glm::dvec3 &minimum, const glm::dvec3 &maximum);

    glm::dvec3 center() const;

    void merge(const glm::dvec3 &point);

    glm::dvec3 min;
    glm::dvec3 max;
};

enum class PrimitiveType
{
    Points,
    Lines,
    LineLoop,
    LineStrip,
    Triangles,
    TriangleStrip,
    TriangleFan
};

enum class TextureFilter
{
    LINEAR,
    LINEAR_MIPMAP_LINEAR,
    LINEAR_MIPMAP_NEAREST,
    NEAREST,
    NEAREST_MIPMAP_LINEAR,
    NEAREST_MIPMAP_NEAREST,

};

struct Mesh
{
    Mesh();

    int material;
    PrimitiveType primitiveType;
    std::optional<AABB> aabb;
    std::vector<uint32_t> indices;
    std::vector<glm::dvec3> positions;
    std::vector<glm::vec3> positionRTCs;
    std::vector<glm::vec2> UVs;
    std::vector<glm::vec3> normals;
    std::vector<float> batchIDs;
};

struct Texture
{
    Texture();

    std::string uri;
    TextureFilter minFilter;
    TextureFilter magFilter;
};

struct Material
{
    Material();

    int texture;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 emission;
    float shininess;
    float alpha;
    bool unlit;
    bool doubleSided;
};
} // namespace CDBTo3DTiles

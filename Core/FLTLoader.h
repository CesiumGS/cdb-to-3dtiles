#pragma once

#include "AABB.h"
#include "BoundRegion.h"
#include "boost/filesystem.hpp"
#include "glm/glm.hpp"
#include <array>
#include <vector>

struct Texture;
struct Material;
struct Mesh;

struct Scene
{
    std::vector<Material> materials;
    std::vector<Texture> textures;
    std::vector<Mesh> meshes;
};

struct Mesh
{
    Mesh()
        : material{-1}
        , texture{-1}
    {}

    int material;
    int texture;
    AABB boundBox;
    BoundRegion region;
    std::vector<uint32_t> indices;
    std::vector<glm::vec4> colors;
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uvs;
};

struct Texture
{
    boost::filesystem::path path;
};

struct Material
{
    Material()
        : ambient{glm::vec3(1.0f)}
        , diffuse{glm::vec3(1.0f)}
        , specular{glm::vec3(1.0f)}
        , emissive{glm::vec3(1.0f)}
        , shininess{1.0f}
        , alpha{1.0f}
    {}

    std::string name;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 emissive;
    float shininess;
    float alpha;
};

Scene LoadFLTFile(const boost::filesystem::path &filePath);

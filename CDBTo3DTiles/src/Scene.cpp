#include "Scene.h"

namespace CDBTo3DTiles {
Mesh::Mesh()
    : material{-1}
    , primitiveType{PrimitiveType::Triangles}
{}

Material::Material()
    : texture{-1}
    , ambient{glm::vec3(1.0f)}
    , diffuse{glm::vec3(1.0f)}
    , specular{glm::vec3(1.0f)}
    , emission{glm::vec3(0.0f)}
    , shininess{32.0f}
    , alpha{1.0f}
    , unlit{false}
    , doubleSided{false}
{}

AABB::AABB()
    : min(std::numeric_limits<double>::max())
    , max(std::numeric_limits<double>::lowest())
{}

AABB::AABB(const glm::dvec3 &minimum, const glm::dvec3 &maximum)
    : min{minimum}
    , max{maximum}
{}

glm::dvec3 AABB::center() const
{
    return (min + max) * 0.5;
}

void AABB::merge(const glm::dvec3 &point)
{
    min = glm::min(point, min);
    max = glm::max(point, max);
}

Texture::Texture()
    : minFilter{TextureFilter::LINEAR_MIPMAP_LINEAR}
    , magFilter{TextureFilter::LINEAR}
{}

} // namespace CDBTo3DTiles

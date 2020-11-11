#include "Plane.h"

namespace Core {
Plane::Plane(const glm::dvec3 &normal, double distance)
    : m_normal(normal)
    , m_distance(distance)
{}

Plane::Plane(const glm::dvec3 &point, const glm::dvec3 &normal)
    : Plane(normal, -glm::dot(normal, point))
{}

} // namespace Core

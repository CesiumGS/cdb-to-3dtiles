#pragma once

#include "glm/glm.hpp"

namespace Core {
class Plane
{
public:
    Plane(const glm::dvec3 &normal, double distance);

    Plane(const glm::dvec3 &point, const glm::dvec3 &normal);

    const glm::dvec3 &getNormal() const { return m_normal; }

    double getDistance() const { return m_distance; }

private:
    glm::dvec3 m_normal;
    double m_distance;
};
} // namespace Core

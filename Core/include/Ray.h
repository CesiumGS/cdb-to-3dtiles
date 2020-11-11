#pragma once

#include "glm/glm.hpp"

namespace Core {
class Ray
{
public:
    Ray(const glm::dvec3 &origin, const glm::dvec3 &direction);

    const glm::dvec3 &getOrigin() const { return m_origin; }

    const glm::dvec3 &getDirection() const { return m_direction; }

    Ray operator-() const;

private:
    glm::dvec3 m_origin;
    glm::dvec3 m_direction;
};
} // namespace Core

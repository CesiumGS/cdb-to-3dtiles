#include "Ray.h"

namespace Core {
Ray::Ray(const glm::dvec3 &origin, const glm::dvec3 &direction)
    : m_origin(origin)
    , m_direction(direction)
{}

Ray Ray::operator-() const
{
    return Ray(m_origin, -m_direction);
}

} // namespace Core

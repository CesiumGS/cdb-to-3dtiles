#pragma once

#include "Ellipsoid.h"
#include "Plane.h"
#include "glm/glm.hpp"

namespace Core {
class EllipsoidTangentPlane
{
public:
    EllipsoidTangentPlane(const glm::dvec3 &origin, const Ellipsoid &ellipsoid = Ellipsoid::WGS84);

    EllipsoidTangentPlane(const glm::dmat4 &eastNorthUpToFixedFrame,
                          const Ellipsoid &ellipsoid = Ellipsoid::WGS84);

    glm::dvec2 projectPointToNearestOnPlane(const glm::dvec3 &cartesian);

private:
    Ellipsoid m_ellipsoid;
    glm::dvec3 m_origin;
    glm::dvec3 m_xAxis;
    glm::dvec3 m_yAxis;
    Core::Plane m_plane;
};

} // namespace Core

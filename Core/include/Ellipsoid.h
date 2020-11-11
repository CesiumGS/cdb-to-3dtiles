#pragma once

#include "Cartographic.h"
#include "glm/glm.hpp"
#include <optional>

namespace Core {
class Ellipsoid
{
public:
    static const Ellipsoid WGS84;

    Ellipsoid(double x, double y, double z);

    Ellipsoid(const glm::dvec3 &radii);

    inline const glm::dvec3 &getRadii() const { return m_radii; }

    glm::dvec3 geodeticSurfaceNormal(const glm::dvec3 &position) const;

    glm::dvec3 geodeticSurfaceNormal(const Cartographic &cartographic) const;

    glm::dvec3 cartographicToCartesian(const Cartographic &cartographic) const;

    std::optional<Cartographic> cartesianToCartographic(const glm::dvec3 &cartesian) const;

    std::optional<glm::dvec3> scaleToGeodeticSurface(const glm::dvec3 &cartesian) const;

    double getMaximumRadius() const;

    double getMinimumRadius() const;

    bool operator==(const Ellipsoid &rhs) const { return this->m_radii == rhs.m_radii; };

    bool operator!=(const Ellipsoid &rhs) const { return this->m_radii != rhs.m_radii; };

private:
    glm::dvec3 m_radii;
    glm::dvec3 m_radiiSquared;
    glm::dvec3 m_oneOverRadii;
    glm::dvec3 m_oneOverRadiiSquared;
    double m_centerToleranceSquared;
};
} // namespace Core

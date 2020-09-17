#pragma once

#include "Cartographic.h"
#include "glm/glm.hpp"

class Ellipsoid
{
public:
    static Ellipsoid WGS84();

    Ellipsoid(double xRadius, double yRadius, double zRadius);

    const glm::dvec3 &GetRadii() const;

    const glm::dvec3 &GetRadiiSquared() const;

    const glm::dvec3 &GetOneOverRadii() const;

    const glm::dvec3 &GetOneOverRadiiSquared() const;

    glm::dvec3 GeodeticSurfaceNormalFromCartographic(const Cartographic &cartographic) const;

    glm::dvec3 GeodeticSurfaceNormalFromCartesian(const glm::dvec3 &cartesian) const;

    glm::dvec3 ScaleToGeodeticSurface(const glm::dvec3 &cartesian) const;

    glm::dvec3 CartographicToCartesian(const Cartographic &cartographic) const;

    glm::dvec3 CartesianToCartographic(const glm::dvec3 &cartesian) const;

private:
    glm::dvec3 m_radii;
    glm::dvec3 m_radiiSquared;
    glm::dvec3 m_oneOverRadii;
    glm::dvec3 m_oneOverRadiiSquared;
};

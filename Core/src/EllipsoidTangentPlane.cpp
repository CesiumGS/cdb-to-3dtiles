#include "EllipsoidTangentPlane.h"
#include "IntersectionTests.h"
#include "Transforms.h"

namespace Core {
EllipsoidTangentPlane::EllipsoidTangentPlane(const glm::dvec3 &origin, const Ellipsoid &ellipsoid)
    : EllipsoidTangentPlane(
        Transforms::eastNorthUpToFixedFrame(ellipsoid.scaleToGeodeticSurface(origin).value(), ellipsoid))
{}

EllipsoidTangentPlane::EllipsoidTangentPlane(const glm::dmat4 &eastNorthUpToFixedFrame,
                                             const Ellipsoid &ellipsoid)
    : m_ellipsoid(ellipsoid)
    , m_origin(eastNorthUpToFixedFrame[3])
    , m_xAxis(eastNorthUpToFixedFrame[0])
    , m_yAxis(eastNorthUpToFixedFrame[1])
    , m_plane(eastNorthUpToFixedFrame[3], eastNorthUpToFixedFrame[2])
{}

glm::dvec2 EllipsoidTangentPlane::projectPointToNearestOnPlane(const glm::dvec3 &cartesian)
{
    Ray ray(cartesian, m_plane.getNormal());

    std::optional<glm::dvec3> intersectionPoint = IntersectionTests::rayPlane(ray, m_plane);
    if (!intersectionPoint) {
        intersectionPoint = IntersectionTests::rayPlane(-ray, m_plane);
        if (!intersectionPoint) {
            intersectionPoint = cartesian;
        }
    }

    glm::dvec3 v = intersectionPoint.value() - m_origin;
    return glm::dvec2(glm::dot(m_xAxis, v), glm::dot(m_yAxis, v));
}

} // namespace Core

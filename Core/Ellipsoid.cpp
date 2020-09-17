#include "Ellipsoid.h"

Ellipsoid Ellipsoid::WGS84()
{
    const double semimajorAxis = 6378137.0;
    const double reciprocalOfFlattening = 298.257223563;
    const double semiminorAxis = (1.0 - 1.0 / reciprocalOfFlattening) * semimajorAxis;
    return Ellipsoid(semimajorAxis, semimajorAxis, semiminorAxis);
}

Ellipsoid::Ellipsoid(double xRadius, double yRadius, double zRadius)
    : m_radii(xRadius, yRadius, zRadius)
    , m_radiiSquared(xRadius * xRadius, yRadius * yRadius, zRadius * zRadius)
    , m_oneOverRadii(1.0 / xRadius, 1.0 / yRadius, 1.0 / zRadius)
    , m_oneOverRadiiSquared(1.0 / (xRadius * xRadius), 1.0 / (yRadius * yRadius), 1.0 / (zRadius * zRadius))
{}

const glm::dvec3 &Ellipsoid::GetRadii() const
{
    return m_radii;
}

const glm::dvec3 &Ellipsoid::GetRadiiSquared() const
{
    return m_radiiSquared;
}

const glm::dvec3 &Ellipsoid::GetOneOverRadii() const
{
    return m_oneOverRadii;
}

const glm::dvec3 &Ellipsoid::GetOneOverRadiiSquared() const
{
    return m_oneOverRadiiSquared;
}

glm::dvec3 Ellipsoid::GeodeticSurfaceNormalFromCartographic(const Cartographic &cartographic) const
{
    double longitude = cartographic.longitude; // Longitude
    double latitude = cartographic.latitude;   // Latitude
    double cosLatitude = cos(latitude);
    return glm::normalize(
        glm::dvec3(cosLatitude * cos(longitude), cosLatitude * sin(longitude), sin(latitude)));
}

glm::dvec3 Ellipsoid::GeodeticSurfaceNormalFromCartesian(const glm::dvec3 &cartesian) const
{
    return glm::normalize(cartesian * m_oneOverRadiiSquared);
}

glm::dvec3 Ellipsoid::ScaleToGeodeticSurface(const glm::dvec3 &cartesian) const
{
    double positionX = cartesian.x;
    double positionY = cartesian.y;
    double positionZ = cartesian.z;

    double x2 = positionX * positionX * m_oneOverRadii.x * m_oneOverRadii.x;
    double y2 = positionY * positionY * m_oneOverRadii.y * m_oneOverRadii.y;
    double z2 = positionZ * positionZ * m_oneOverRadii.z * m_oneOverRadii.z;

    // Compute the squared ellipsoid norm.
    double squaredNorm = x2 + y2 + z2;
    double ratio = sqrt(1.0 / squaredNorm);

    // As an initial approximation, assume that the radial intersection is the projection point.
    glm::dvec3 intersection = cartesian * ratio;

    ////* If the position is near the center, the iteration will not converge.
    //if (squaredNorm < m_centerToleranceSquared) {
    //    return !isFinite(ratio) ? undefined : intersection;
    //}

    // Use the gradient at the intersection point in place of the true unit normal.
    // The difference in magnitude will be absorbed in the multiplier.
    glm::dvec3 gradient(intersection.x * m_oneOverRadiiSquared.x * 2.0,
                        intersection.y * m_oneOverRadiiSquared.y * 2.0,
                        intersection.z * m_oneOverRadiiSquared.z * 2.0);

    // Compute the initial guess at the normal vector multiplier, lambda.

    double lambda = (1.0 - ratio) * glm::length(cartesian) / (0.5 * glm::length(gradient));
    double correction = 0.0;

    double func;
    double denominator;
    double xMultiplier;
    double yMultiplier;
    double zMultiplier;
    double xMultiplier2;
    double yMultiplier2;
    double zMultiplier2;
    double xMultiplier3;
    double yMultiplier3;
    double zMultiplier3;

    do {
        lambda -= correction;

        xMultiplier = 1.0 / (1.0 + lambda * m_oneOverRadiiSquared.x);
        yMultiplier = 1.0 / (1.0 + lambda * m_oneOverRadiiSquared.y);
        zMultiplier = 1.0 / (1.0 + lambda * m_oneOverRadiiSquared.z);

        xMultiplier2 = xMultiplier * xMultiplier;
        yMultiplier2 = yMultiplier * yMultiplier;
        zMultiplier2 = zMultiplier * zMultiplier;

        xMultiplier3 = xMultiplier2 * xMultiplier;
        yMultiplier3 = yMultiplier2 * yMultiplier;
        zMultiplier3 = zMultiplier2 * zMultiplier;

        func = x2 * xMultiplier2 + y2 * yMultiplier2 + z2 * zMultiplier2 - 1.0;

        // "denominator" here refers to the use of this expression in the velocity and acceleration
        // computations in the sections to follow.
        denominator = x2 * xMultiplier3 * m_oneOverRadiiSquared.x
                      + y2 * yMultiplier3 * m_oneOverRadiiSquared.y
                      + z2 * zMultiplier3 * m_oneOverRadiiSquared.z;

        double derivative = -2.0 * denominator;

        correction = func / derivative;
    } while (glm::abs(func) > 1e-12);

    return glm::dvec3(positionX * xMultiplier, positionY * yMultiplier, positionZ * zMultiplier);
}

glm::dvec3 Ellipsoid::CartographicToCartesian(const Cartographic &cartographic) const
{
    glm::dvec3 n = GeodeticSurfaceNormalFromCartographic(cartographic);
    glm::dvec3 k = m_radiiSquared * n;
    double gamma = sqrt(glm::dot(n, k));
    k /= gamma;
    n *= cartographic.height; // Altitude
    return k + n;
}

glm::dvec3 Ellipsoid::CartesianToCartographic(const glm::dvec3 &cartesian) const
{
    glm::dvec3 p = ScaleToGeodeticSurface(cartesian);

    glm::dvec3 n = GeodeticSurfaceNormalFromCartesian(p);
    glm::dvec3 h = cartesian - p;

    double longitude = atan2(n.y, n.x);
    double latitude = asin(n.z);
    double dot = glm::dot(h, cartesian);
    double height = ((dot > 0.0) - (dot < 0.0)) * glm::length(h);

    return glm::dvec3(longitude, latitude, height);
}

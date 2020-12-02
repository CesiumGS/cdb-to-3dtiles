#include "GlobeRectangle.h"
#include "Math.h"

namespace Core {

GlobeRectangle::GlobeRectangle(double west, double south, double east, double north)
    : m_west(west)
    , m_south(south)
    , m_east(east)
    , m_north(north)
{}

double GlobeRectangle::computeWidth() const
{
    double east = m_east;
    double west = m_west;
    if (east < west) {
        east += Math::TWO_PI;
    }
    return east - west;
}

double GlobeRectangle::computeHeight() const
{
    return m_north - m_south;
}

Cartographic GlobeRectangle::computeCenter() const
{
    double east = m_east;
    double west = m_west;

    if (east < west) {
        east += Math::TWO_PI;
    }

    double longitude = Math::negativePiToPi((west + east) * 0.5);
    double latitude = (m_south + m_north) * 0.5;

    return Cartographic(longitude, latitude, 0.0);
}

bool GlobeRectangle::contains(const Cartographic &cartographic) const
{
    double longitude = cartographic.longitude;
    double latitude = cartographic.latitude;

    double west = m_west;
    double east = m_east;

    if (east < west) {
        east += Math::TWO_PI;
        if (longitude < 0.0) {
            longitude += Math::TWO_PI;
        }
    }
    return ((longitude > west || Math::equalsEpsilon(longitude, west, Math::EPSILON14))
            && (longitude < east || Math::equalsEpsilon(longitude, east, Math::EPSILON14))
            && latitude >= m_south && latitude <= m_north);
}

GlobeRectangle GlobeRectangle::computeUnion(const GlobeRectangle &other) const
{
    double rectangleEast = m_east;
    double rectangleWest = m_west;

    double otherRectangleEast = other.m_east;
    double otherRectangleWest = other.m_west;

    if (rectangleEast < rectangleWest && otherRectangleEast > 0.0) {
        rectangleEast += Math::TWO_PI;
    } else if (otherRectangleEast < otherRectangleWest && rectangleEast > 0.0) {
        otherRectangleEast += Math::TWO_PI;
    }

    if (rectangleEast < rectangleWest && otherRectangleWest < 0.0) {
        otherRectangleWest += Math::TWO_PI;
    } else if (otherRectangleEast < otherRectangleWest && rectangleWest < 0.0) {
        rectangleWest += Math::TWO_PI;
    }

    double west = Math::convertLongitudeRange(glm::min(rectangleWest, otherRectangleWest));
    double east = Math::convertLongitudeRange(glm::max(rectangleEast, otherRectangleEast));

    return GlobeRectangle(west, glm::min(m_south, other.m_south), east, glm::max(m_north, other.m_north));
}

void GlobeRectangle::expand(const Cartographic &cartographic)
{
    m_west = glm::min(m_west, cartographic.longitude);
    m_south = glm::min(m_south, cartographic.latitude);
    m_east = glm::max(m_east, cartographic.longitude);
    m_north = glm::max(m_north, cartographic.latitude);
}
} // namespace Core

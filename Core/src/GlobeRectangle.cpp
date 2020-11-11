#include "GlobeRectangle.h"
#include "MathUtility.h"

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
} // namespace Core

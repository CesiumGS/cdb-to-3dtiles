#pragma once

#include "Cartographic.h"

namespace Core {

class GlobeRectangle
{
public:
    GlobeRectangle(double west, double south, double east, double north);

    double getWest() const { return m_west; }

    double getSouth() const { return m_south; }

    double getEast() const { return m_east; }

    double getNorth() const { return m_north; }

    Cartographic getSouthwest() const { return Cartographic(m_west, m_south); }

    Cartographic getSoutheast() const { return Cartographic(m_east, m_south); }

    Cartographic getNorthwest() const { return Cartographic(m_west, m_north); }

    Cartographic getNortheast() const { return Cartographic(m_east, m_north); }

    double computeWidth() const;

    double computeHeight() const;

    Cartographic computeCenter() const;

    bool contains(const Cartographic &cartographic) const;

private:
    double m_west;
    double m_south;
    double m_east;
    double m_north;
};
} // namespace Core

#pragma once

#include "Cartographic.h"
#include "GlobeRectangle.h"

namespace Core {

class BoundingRegion
{
public:
    BoundingRegion(const GlobeRectangle &rectangle, double minimumHeight, double maximumHeight);

    const GlobeRectangle &getRectangle() const { return m_rectangle; }

    double getMinimumHeight() const { return m_minimumHeight; }

    double getMaximumHeight() const { return m_maximumHeight; }

    BoundingRegion computeUnion(const BoundingRegion &other) const;

    void expand(const Cartographic &cartographic);

private:
    GlobeRectangle m_rectangle;
    double m_minimumHeight;
    double m_maximumHeight;
};

} // namespace Core

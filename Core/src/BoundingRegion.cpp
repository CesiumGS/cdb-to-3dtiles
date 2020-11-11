#include "BoundingRegion.h"

namespace Core {

BoundingRegion::BoundingRegion(const GlobeRectangle &rectangle, double minimumHeight, double maximumHeight)
    : m_rectangle{rectangle}
    , m_minimumHeight{minimumHeight}
    , m_maximumHeight{maximumHeight}
{}

} // namespace Core

#include "BoundingRegion.h"
#include "glm/glm.hpp"

namespace Core {

BoundingRegion::BoundingRegion(const GlobeRectangle &rectangle, double minimumHeight, double maximumHeight)
    : m_rectangle{rectangle}
    , m_minimumHeight{minimumHeight}
    , m_maximumHeight{maximumHeight}
{}

BoundingRegion BoundingRegion::computeUnion(const BoundingRegion &other) const
{
    return BoundingRegion(m_rectangle.computeUnion(other.m_rectangle),
                          glm::min(m_minimumHeight, other.m_minimumHeight),
                          glm::max(m_maximumHeight, other.m_maximumHeight));
}
} // namespace Core

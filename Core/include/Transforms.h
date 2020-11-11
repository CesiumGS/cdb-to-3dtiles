#pragma once
#include "Ellipsoid.h"
#include "glm/glm.hpp"

namespace Core {
class Transforms
{
public:
    static glm::dmat4x4 eastNorthUpToFixedFrame(const glm::dvec3 &origin,
                                                const Ellipsoid &ellipsoid = Ellipsoid::WGS84);
};
} // namespace Core

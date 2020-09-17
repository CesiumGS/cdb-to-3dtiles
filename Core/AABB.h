#pragma once

#include "glm/glm.hpp"
#include <vector>

struct AABB
{
    glm::dvec3 min;
    glm::dvec3 max;

    AABB()
        : min(std::numeric_limits<double>::max())
        , max(std::numeric_limits<double>::lowest())
    {}

    AABB(const glm::dvec3 &minimum, const glm::dvec3 &maximum)
        : min{minimum}
        , max{maximum}
    {}

    inline glm::dvec3 center() const { return (min + max) * 0.5; }

    inline void mergePoint(const glm::dvec3 &point)
    {
        min = glm::min(point, min);
        max = glm::max(point, max);
    }
};

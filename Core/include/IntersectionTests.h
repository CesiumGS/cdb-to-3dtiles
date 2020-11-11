#pragma once

#include "Plane.h"
#include "Ray.h"
#include "glm/glm.hpp"
#include <optional>

namespace Core {
class IntersectionTests
{
public:
    static std::optional<glm::dvec3> rayPlane(const Ray &ray, const Plane &plane);
};
} // namespace Core

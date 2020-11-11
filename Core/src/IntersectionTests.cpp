#include "IntersectionTests.h"
#include "Math.h"

namespace Core {

/*static*/ std::optional<glm::dvec3> IntersectionTests::rayPlane(const Ray &ray, const Plane &plane)
{
    double denominator = glm::dot(plane.getNormal(), ray.getDirection());

    if (glm::abs(denominator) < Math::EPSILON15) {
        // Ray is parallel to plane.  The ray may be in the polygon's plane.
        return std::optional<glm::dvec3>();
    }

    double t = (-plane.getDistance() - glm::dot(plane.getNormal(), ray.getOrigin())) / denominator;

    if (t < 0) {
        return std::optional<glm::dvec3>();
    }

    return ray.getOrigin() + ray.getDirection() * t;
}
} // namespace Core

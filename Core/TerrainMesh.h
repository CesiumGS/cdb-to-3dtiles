#pragma once

#include "AABB.h"
#include "BoundRegion.h"
#include <vector>

struct TerrainMesh
{
    AABB boundBox;
    BoundRegion region;
    std::vector<uint32_t> indices;
    std::vector<glm::vec3> positionsRtc;
    std::vector<glm::dvec3> positions;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> uv;
};

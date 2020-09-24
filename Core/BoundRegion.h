#pragma once

#include "Cartographic.h"
#include "glm/glm.hpp"
#include <limits>

struct BoundRegion
{
    BoundRegion()
        : minHeight{std::numeric_limits<double>::max()}
        , maxHeight{std::numeric_limits<double>::lowest()}
        , west{std::numeric_limits<double>::max()}
        , south{std::numeric_limits<double>::max()}
        , east{std::numeric_limits<double>::lowest()}
        , north{std::numeric_limits<double>::lowest()}
    {}

    BoundRegion(double minHeight, double maxHeight, double west, double south, double east, double north)
        : minHeight{minHeight}
        , maxHeight{maxHeight}
        , west{west}
        , south{south}
        , east{east}
        , north{north}
    {}

    inline void merge(Cartographic point)
    {
        minHeight = glm::min(point.height, minHeight);
        maxHeight = glm::max(point.height, maxHeight);
        west = glm::min(point.longitude, west);
        east = glm::max(point.longitude, east);
        north = glm::max(point.latitude, north);
        south = glm::min(point.latitude, south);
    }

    double minHeight;
    double maxHeight;
    double west;
    double south;
    double east;
    double north;
};

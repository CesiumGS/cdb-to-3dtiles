#pragma once

struct BoundRegion
{
    BoundRegion()
        : minHeight{0.0}
        , maxHeight{0.0}
        , west{0.0}
        , south{0.0}
        , east{0.0}
        , north{0.0}
    {}

    BoundRegion(double minHeight, double maxHeight, double west, double south, double east, double north)
        : minHeight{minHeight}
        , maxHeight{maxHeight}
        , west{west}
        , south{south}
        , east{east}
        , north{north}
    {}

    double minHeight;
    double maxHeight;
    double west;
    double south;
    double east;
    double north;
};

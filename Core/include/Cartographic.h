#pragma once

namespace Core {
class Cartographic
{
public:
    explicit Cartographic(double longitudeRadians, double latitudeRadians, double heightMeters = 0.0);

    double longitude;
    double latitude;
    double height;
};
} // namespace Core

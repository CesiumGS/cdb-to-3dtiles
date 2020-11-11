#include "Cartographic.h"

namespace Core {
Cartographic::Cartographic(double longitudeParam, double latitudeParam, double heightParam)
    : longitude(longitudeParam)
    , latitude(latitudeParam)
    , height(heightParam)
{}
} // namespace Core

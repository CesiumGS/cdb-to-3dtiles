#pragma once

struct Cartographic
{
    Cartographic()
        : longitude{0.0}
        , latitude{0.0}
        , height{0.0}
    {}

    Cartographic(double longitude, double latitude, double height)
        : longitude{longitude}
        , latitude{latitude}
        , height{height}
    {}

    double longitude;
    double latitude;
    double height;
};

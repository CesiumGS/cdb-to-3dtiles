#pragma once

#include "Ellipsoid.h"
#include "TerrainMesh.h"
#include "boost/filesystem.hpp"

struct TIFOption
{
    TIFOption()
        : ellipsoid{nullptr}
        , topLeft{nullptr}
    {}

    const Ellipsoid *ellipsoid;
    const Cartographic *topLeft;
};

TerrainMesh LoadTIFFile(const boost::filesystem::path &filePath, const TIFOption &option);

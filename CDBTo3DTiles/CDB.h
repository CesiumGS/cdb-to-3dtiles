#pragma once

#include "GDALImage.h"
#include "TerrainMesh.h"
#include "boost/filesystem.hpp"
#include <array>
#include <functional>
#include <string>

struct CDBGeoCell;
struct CDBTile;
struct CDBTerrain;
struct CDBImagery;

class CDB
{
public:
    CDB(const boost::filesystem::path &CDBPath);

    void ForEachGeoCell(
        std::function<void(const boost::filesystem::path &CDBGeoCellPath, const CDBGeoCell &geoCell)> process);

    void ForEachElevationTile(const boost::filesystem::path &CDBGeoCellPath,
                              std::function<void(CDBTerrain &terrain)> process);

    void ForEachImageryTile(const boost::filesystem::path &CDBGeoCellPath,
                            std::function<void(CDBImagery &imagery)> process);

private:
    static CDBTile ParseEncodedCDBFullName(const std::string &encodedCDBFullName, bool negativeLevel);

    static void ForEachDatasetTile(
        const boost::filesystem::path &CDBGeoCellPath,
        const boost::filesystem::path &datasetPath,
        std::function<void(const boost::filesystem::path &, bool negativeLevel)> process);

    static int ParseGeoCellLatitudePath(const std::string &geoCellLatitude);

    static int ParseGeoCellLongitudePath(const std::string &geoCellLongitude);

    static const boost::filesystem::path TILES_PATH;
    static const boost::filesystem::path ELEVATION_PATH;
    static const boost::filesystem::path IMAGERY_PATH;

    boost::filesystem::path _CDBPath;
};

struct CDBGeoCell
{
    CDBGeoCell()
        : longitude{0}
        , latitude{0}
    {}

    CDBGeoCell(int longitude, int latitude)
        : longitude{longitude}
        , latitude{latitude}
    {}

    int longitude;
    int latitude;
};

struct CDBTile
{
    CDBTile()
        : encodedCDBDataset{0}
        , componentSelectors({0, 0})
        , x{0}
        , y{0}
        , level{0}
    {}

    std::string encodedCDBFullName;
    int encodedCDBDataset;
    std::array<int, 2> componentSelectors;
    CDBGeoCell geoCell;
    int x;
    int y;
    int level;
};

struct CDBTerrain
{
    TerrainMesh mesh;
    CDBTile tile;
};

struct CDBImagery
{
    GDALImage image;
    CDBTile tile;
};

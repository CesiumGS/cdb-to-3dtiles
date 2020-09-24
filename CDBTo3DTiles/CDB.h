#pragma once

#include "Cartographic.h"
#include "FLTLoader.h"
#include "GDALDatasetWrapper.h"
#include "TerrainMesh.h"
#include "boost/filesystem.hpp"
#include <array>
#include <functional>
#include <string>
#include <unordered_map>

struct CDBGeoCell;
struct CDBTile;
struct CDBTerrain;
struct CDBImagery;
struct CDBGSModel;
struct CDBInstanceGSFeature;
struct CDBClassGSFeature;

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

    void ForEachGSModelTile(const boost::filesystem::path &CDBGeoCellPath,
                            std::function<void(CDBGSModel &model)> process);

    static BoundRegion CalculateTileExtent(const CDBGeoCell &cell, int level, int x, int y);

private:
    static const int MIN_LOD;
    static const boost::filesystem::path TILES_PATH;
    static const boost::filesystem::path ELEVATION_PATH;
    static const boost::filesystem::path IMAGERY_PATH;
    static const boost::filesystem::path GSMODEL_GEOMETRY_PATH;
    static const boost::filesystem::path GSFEATURE_PATH;

    static CDBTile ParseEncodedCDBFullName(const std::string &encodedCDBFullName, bool negativeLevel);

    static void ForEachDatasetTile(
        const boost::filesystem::path &CDBGeoCellPath,
        const boost::filesystem::path &datasetPath,
        std::function<void(const boost::filesystem::path &, bool negativeLevel)> process);

    static int ParseGeoCellLatitudePath(const std::string &geoCellLatitude);

    static int ParseGeoCellLongitudePath(const std::string &geoCellLongitude);

    static std::unordered_map<std::string, CDBInstanceGSFeature> ParseInstanceGSModelFeature(
        int currentLOD,
        const std::string &encodedCDBModelTileName,
        const boost::filesystem::path &instanceFeaturePath);

    static std::unordered_map<std::string, CDBClassGSFeature> ParseClassGSModelFeature(
        const boost::filesystem::path &classFeaturePath);

    static boost::filesystem::path CDBTileToPath(const CDBTile &tile);

    static boost::filesystem::path TileLevelToPath(int level);

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

    std::string encodedCDBName;
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
    GDALDatasetWrapper image;
    CDBTile tile;
};

struct CDBGSModel
{
    std::vector<Scene> scenes;
    std::vector<glm::dvec3> positions;
    std::vector<double> angleOfOrientations;
    CDBTile tile;
    BoundRegion region;
};

struct CDBInstanceGSFeature
{
    bool modelTypical;
    bool AHGT;
    std::string CNAM;
    std::string FACC;
    std::string MODL;
    int FSC;
};

struct CDBClassGSFeature
{
    std::string CNAM;
    Cartographic point;
    double angleOrientation{};
};

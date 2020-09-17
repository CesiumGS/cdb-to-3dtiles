#include "CDB.h"
#include "TIFLoader.h"

const boost::filesystem::path CDB::TILES_PATH = "Tiles";
const boost::filesystem::path CDB::ELEVATION_PATH = "001_Elevation";
const boost::filesystem::path CDB::IMAGERY_PATH = "004_Imagery";

CDBTile CDB::ParseEncodedCDBFullName(const std::string &encodedCDBFullName, bool negativeLevel)
{
    int encodedDataset, componentSelector1, componentSelector2;
    int level, x, y;
    char NS, WE;
    int geocellLatitude = 0, geocellLongitude = 0;
    if (negativeLevel) {
        sscanf(encodedCDBFullName.c_str(),
               "%c%d%c%d_D%d_S%d_T%d_LC%d_U%d_R%d",
               &NS,
               &geocellLatitude,
               &WE,
               &geocellLongitude,
               &encodedDataset,
               &componentSelector1,
               &componentSelector2,
               &level,
               &x,
               &y);
    } else {
        sscanf(encodedCDBFullName.c_str(),
               "%c%d%c%d_D%d_S%d_T%d_L%d_U%d_R%d",
               &NS,
               &geocellLatitude,
               &WE,
               &geocellLongitude,
               &encodedDataset,
               &componentSelector1,
               &componentSelector2,
               &level,
               &x,
               &y);
    }

    geocellLatitude = NS == 'S' ? -geocellLatitude : geocellLatitude;
    geocellLongitude = WE == 'W' ? -geocellLongitude : geocellLongitude;

    CDBGeoCell geoCell(geocellLongitude, geocellLatitude);
    CDBTile tile;
    tile.geoCell = geoCell;
    tile.encodedCDBFullName = encodedCDBFullName;
    tile.encodedCDBDataset = encodedDataset;
    tile.componentSelectors[0] = componentSelector1;
    tile.componentSelectors[1] = componentSelector2;
    tile.level = negativeLevel ? -level : level;
    tile.x = x;
    tile.y = y;

    return tile;
}

CDB::CDB(const boost::filesystem::path &CDBPath)
    : _CDBPath{CDBPath}
{}

void CDB::ForEachGeoCell(
    std::function<void(const boost::filesystem::path &, const CDBGeoCell &geoCell)> process)
{
    auto geoCellsPath = _CDBPath / TILES_PATH;

    if (!boost::filesystem::exists(geoCellsPath) || !boost::filesystem::is_directory(geoCellsPath)) {
        throw std::runtime_error(geoCellsPath.string() + " directory does not exist");
    }

    int cellLongitude = 0, cellLatitude = 0;
    for (boost::filesystem::directory_entry geoCellLatitude :
         boost::filesystem::directory_iterator(geoCellsPath)) {
        cellLatitude = ParseGeoCellLatitudePath(geoCellLatitude.path().filename().string());
        for (boost::filesystem::directory_entry geoCellLongitude :
             boost::filesystem::directory_iterator(geoCellLatitude)) {
            cellLongitude = ParseGeoCellLongitudePath(geoCellLongitude.path().filename().string());
            process(geoCellLongitude, CDBGeoCell(cellLongitude, cellLatitude));
        }
    }
}

void CDB::ForEachElevationTile(const boost::filesystem::path &CDBGeoCellPath,
                               std::function<void(CDBTerrain &)> process)
{
    ForEachDatasetTile(CDBGeoCellPath,
                       ELEVATION_PATH,
                       [&](const boost::filesystem::path &elevationTilePath, bool negativeLevel) {
                           if (elevationTilePath.extension() != ".tif") {
                               return;
                           }

                           auto tile = ParseEncodedCDBFullName(elevationTilePath.filename().stem().string(),
                                                               negativeLevel);

                           TIFOption TIFOptionLoader;
                           Cartographic topLeft;
                           if (negativeLevel) {
                               topLeft = Cartographic(tile.geoCell.longitude,
                                                      tile.geoCell.latitude + 1.0,
                                                      0.0);
                               TIFOptionLoader.topLeft = &topLeft;
                           }
                           CDBTerrain terrain;
                           terrain.mesh = LoadTIFFile(elevationTilePath, TIFOptionLoader);
                           terrain.tile = tile;
                           process(terrain);
                       });
}

void CDB::ForEachImageryTile(const boost::filesystem::path &CDBGeoCellPath,
                             std::function<void(CDBImagery &)> process)
{
    ForEachDatasetTile(CDBGeoCellPath,
                       IMAGERY_PATH,
                       [&](const boost::filesystem::path &imageryTilePath, bool negativeLevel) {
                           if (imageryTilePath.extension() != ".jp2") {
                               return;
                           }

                           auto tile = ParseEncodedCDBFullName(imageryTilePath.filename().stem().string(),
                                                               negativeLevel);
                           CDBImagery imagery;
                           imagery.image = GDALImage(imageryTilePath, GDALAccess::GA_ReadOnly);
                           imagery.tile = tile;
                           process(imagery);
                       });
}

void CDB::ForEachDatasetTile(const boost::filesystem::path &CDBGeoCellPath,
                             const boost::filesystem::path &datasetPath,
                             std::function<void(const boost::filesystem::path &, bool negativeLevel)> process)
{
    auto dataset = CDBGeoCellPath / datasetPath;
    if (!boost::filesystem::exists(dataset)) {
        return;
    }

    for (boost::filesystem::directory_entry level : boost::filesystem::directory_iterator(dataset)) {
        for (boost::filesystem::directory_entry U : boost::filesystem::directory_iterator(level)) {
            bool negativeLevel = level.path().filename() == "LC";
            for (boost::filesystem::directory_entry R : boost::filesystem::directory_iterator(U)) {
                process(R, negativeLevel);
            }
        }
    }
}

int CDB::ParseGeoCellLatitudePath(const std::string &geoCellLatitude)
{
    int latitude;
    char NS;
    sscanf(geoCellLatitude.c_str(), "%c%d", &NS, &latitude);
    return NS == 'S' ? -latitude : latitude;
}

int CDB::ParseGeoCellLongitudePath(const std::string &geoCellLongitude)
{
    int longitude;
    char WE;
    sscanf(geoCellLongitude.c_str(), "%c%d", &WE, &longitude);
    return WE == 'W' ? -longitude : longitude;
}

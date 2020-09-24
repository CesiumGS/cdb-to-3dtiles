#include "CDB.h"
#include "TIFLoader.h"
#include "ogrsf_frmts.h"
#include "zip.h"
#include <iostream>

const int CDB::MIN_LOD = -10;
const boost::filesystem::path CDB::TILES_PATH = "Tiles";
const boost::filesystem::path CDB::ELEVATION_PATH = "001_Elevation";
const boost::filesystem::path CDB::IMAGERY_PATH = "004_Imagery";
const boost::filesystem::path CDB::GSMODEL_GEOMETRY_PATH = "300_GSModelGeometry";
const boost::filesystem::path CDB::GSFEATURE_PATH = "100_GSFeature";

static void CloseZipArchive(zip_t *) noexcept;
static void CloseZipFile(zip_file *) noexcept;

using ZipArchiveWrapper = std::unique_ptr<zip_t, decltype(&CloseZipArchive)>;
using ZipFileWrapper = std::unique_ptr<zip_file, decltype(&CloseZipFile)>;

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

                           auto tile = ParseEncodedCDBFullName(elevationTilePath.stem().string(),
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

                           auto tile = ParseEncodedCDBFullName(imageryTilePath.stem().string(), negativeLevel);
                           CDBImagery imagery;
                           imagery.image = (GDALDataset *) GDALOpen(imageryTilePath.c_str(),
                                                                    GDALAccess::GA_ReadOnly);
                           imagery.tile = tile;
                           process(imagery);
                       });
}

void CDB::ForEachGSModelTile(const boost::filesystem::path &CDBGeoCellPath,
                             std::function<void(CDBGSModel &)> process)
{
    ForEachDatasetTile(CDBGeoCellPath,
                       GSMODEL_GEOMETRY_PATH,
                       [&](const boost::filesystem::path &GSModelPath, bool negativeLevel) {
                           if (GSModelPath.extension() != ".zip") {
                               return;
                           }

                           auto filename = GSModelPath.stem().string();
                           auto tile = ParseEncodedCDBFullName(filename, negativeLevel);

                           CDBGSModel model;
                           model.tile = tile;
                           model.region = CalculateTileExtent(tile.geoCell, tile.level, tile.x, tile.y);

                           // query GSFeature since geometry's positions are defined in there
                           // GSFeature code is 100 and class based component selector is 1. Refactor to be more clear
                           auto classFeatureTile = tile;
                           classFeatureTile.encodedCDBDataset = 100;
                           classFeatureTile.componentSelectors[1] = 1;
                           auto classGSFeaturePath = CDBGeoCellPath / GSFEATURE_PATH
                                                     / TileLevelToPath(classFeatureTile.level)
                                                     / ("U" + std::to_string(classFeatureTile.x))
                                                     / (CDBTileToPath(classFeatureTile).string() + ".dbf");
                           auto classGSFeatures = ParseClassGSModelFeature(classGSFeaturePath);

                           // parse instance features
                           auto instanceFeatureTile = tile;
                           instanceFeatureTile.encodedCDBDataset = 100;
                           instanceFeatureTile.componentSelectors[1] = 2;
                           auto instanceGSFeaturePath = CDBGeoCellPath / GSFEATURE_PATH
                                                        / TileLevelToPath(instanceFeatureTile.level)
                                                        / ("U" + std::to_string(instanceFeatureTile.x))
                                                        / (CDBTileToPath(instanceFeatureTile).string()
                                                           + ".dbf");
                           auto instanceGSFeatures = ParseInstanceGSModelFeature(tile.level,
                                                                                 tile.encodedCDBName,
                                                                                 instanceGSFeaturePath);

                           // extract zip files to temp directory
                           int zipError;
                           struct zip_stat zipStat;
                           ZipArchiveWrapper zipArchive = ZipArchiveWrapper(zip_open(GSModelPath.c_str(),
                                                                                     0,
                                                                                     &zipError),
                                                                            CloseZipArchive);
                           if (zipArchive) {
                               int totalZipEntries = zip_get_num_entries(zipArchive.get(), 0);
                               model.scenes.reserve(totalZipEntries);
                               for (int i = 0; i < totalZipEntries; ++i) {
                                   if (zip_stat_index(zipArchive.get(), i, 0, &zipStat) == 0) {
                                       // Saving the buffer to the disk is possibly the worst thing to do ever, instead of
                                       // processing the stream right away.But flt loader library only takes filename :(
                                       ZipFileWrapper fd = ZipFileWrapper(zip_fopen_index(zipArchive.get(),
                                                                                          i,
                                                                                          0),
                                                                          CloseZipFile);

                                       auto extractedPath = GSModelPath.parent_path() / zipStat.name;
                                       std::ofstream fs(extractedPath.string(), std::ios::binary);
                                       const int MAX_BUFFER = 400;
                                       char buff[MAX_BUFFER];
                                       uint64_t totalRead = 0;
                                       while (totalRead < zipStat.size) {
                                           int nread = zip_fread(fd.get(), buff, MAX_BUFFER);
                                           if (nread < 0) {
                                               std::cerr << "Error::Cannot extract file "
                                                         << extractedPath.string() << "\n";
                                               break;
                                           }

                                           fs.write(buff, nread);
                                           totalRead += nread;
                                       }

                                       fs.close();

                                       // successfully read the file
                                       if (totalRead == zipStat.size) {
                                           Scene scene = LoadFLTFile(extractedPath);
                                           auto CNAM = instanceGSFeatures.find(extractedPath.stem().string());
                                           if (CNAM != instanceGSFeatures.end()) {
                                               auto classFeature = classGSFeatures.find(CNAM->second.CNAM);
                                               if (classFeature != classGSFeatures.end()) {
                                                   auto cartographic = classFeature->second.point;
                                                   auto point = Ellipsoid::WGS84().CartographicToCartesian(
                                                       cartographic);
                                                   model.positions.emplace_back(point);
                                                   model.angleOfOrientations.emplace_back(
                                                       classFeature->second.angleOrientation);
                                                   model.scenes.emplace_back(scene);
                                               }
                                           }
                                       }

                                       boost::filesystem::remove(extractedPath);
                                   }
                               }
                           }

                           zipArchive = nullptr;
                           process(model);
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

std::unordered_map<std::string, CDBInstanceGSFeature> CDB::ParseInstanceGSModelFeature(
    int currentLOD,
    const std::string &encodedCDBModelTileName,
    const boost::filesystem::path &instanceFeaturePath)
{
    std::unordered_map<std::string, CDBInstanceGSFeature> CDBFeatures;

    GDALDatasetWrapper dataWrapper = (GDALDataset *)
        GDALOpenEx(instanceFeaturePath.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    if (dataWrapper.data()) {
        auto data = dataWrapper.data();
        for (int i = 0; i < data->GetLayerCount(); ++i) {
            OGRLayer *layer = data->GetLayer(i);

            CDBFeatures.reserve(layer->GetFeatureCount());
            for (auto &feature : *layer) {
                auto MODLidx = feature->GetFieldIndex("MODL");
                if (MODLidx < 0) {
                    continue;
                }

                auto MLODidx = feature->GetFieldIndex("MLOD");
                if (MLODidx >= 0) {
                    auto MLOD = feature->GetFieldAsInteger(MLODidx);
                    if (MLOD != currentLOD) {
                        continue;
                    }
                }

                CDBInstanceGSFeature CDBFeature;

                auto MODL = feature->GetFieldAsString(MODLidx);
                CDBFeature.MODL = MODL;

                auto AHGTidx = feature->GetFieldIndex("AHGT");
                if (AHGTidx >= 0) {
                    auto AHGT = feature->GetFieldAsString(AHGTidx);
                    CDBFeature.AHGT = AHGT[0] != 'F';
                }

                auto CNAMidx = feature->GetFieldIndex("CNAM");
                if (CNAMidx >= 0) {
                    auto CNAM = feature->GetFieldAsString(CNAMidx);
                    CDBFeature.CNAM = CNAM;
                }

                auto FACCidx = feature->GetFieldIndex("FACC");
                if (FACCidx >= 0) {
                    auto FACC = feature->GetFieldAsString(FACCidx);
                    CDBFeature.FACC = FACC;
                }

                auto FSCidx = feature->GetFieldIndex("FSC");
                if (FSCidx >= 0) {
                    auto FSC = feature->GetFieldAsInteger("FSC");
                    CDBFeature.FSC = FSC;
                }

                // pad 0 for FSC
                char FSC[8] = "";
                sprintf(FSC, "%03d", CDBFeature.FSC);

                std::string encodedCDBModel = encodedCDBModelTileName + "_" + CDBFeature.FACC + "_" + FSC
                                              + "_" + CDBFeature.MODL;
                CDBFeatures[encodedCDBModel] = CDBFeature;
            }
        }
    }

    return CDBFeatures;
}

std::unordered_map<std::string, CDBClassGSFeature> CDB::ParseClassGSModelFeature(
    const boost::filesystem::path &classFeaturePath)
{
    std::unordered_map<std::string, CDBClassGSFeature> CDBFeatures;

    GDALDatasetWrapper dataWrapper = (GDALDataset *)
        GDALOpenEx(classFeaturePath.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr);
    if (dataWrapper.data()) {
        auto data = dataWrapper.data();
        for (int i = 0; i < data->GetLayerCount(); ++i) {
            OGRLayer *layer = data->GetLayer(i);

            CDBFeatures.reserve(layer->GetFeatureCount());
            for (auto &feature : *layer) {
                CDBClassGSFeature CDBFeature;

                auto CNAMidx = feature->GetFieldIndex("CNAM");
                if (CNAMidx >= 0) {
                    auto CNAM = feature->GetFieldAsString(CNAMidx);
                    CDBFeature.CNAM = CNAM;
                }

                auto AO1idx = feature->GetFieldIndex("AO1");
                if (AO1idx >= 0) {
                    auto AO1 = feature->GetFieldAsDouble(AO1idx);
                    CDBFeature.angleOrientation = AO1;
                }

                const OGRGeometry *geometry = feature->GetGeometryRef();
                if (geometry != nullptr && wkbFlatten(geometry->getGeometryType()) == wkbPoint) {
                    const OGRPoint *point = geometry->toPoint();
                    CDBFeature.point = Cartographic(glm::radians(point->getX()),
                                                    glm::radians(point->getY()),
                                                    point->getZ());
                }

                CDBFeatures.insert({CDBFeature.CNAM, CDBFeature});
            }
        }
    }

    return CDBFeatures;
}

boost::filesystem::path CDB::CDBTileToPath(const CDBTile &tile)
{
    std::string format;

    // format latitude
    if (tile.geoCell.latitude < 0) {
        format += "S%d";
    } else {
        format += "N%d";
    }

    // format longitude
    if (tile.geoCell.longitude < 0) {
        format += "W%d";
    } else {
        format += "E%d";
    }

    // format for dataset and component selectors
    format += "_D%03d_S%03d_T%03d_";

    // level
    if (tile.level < 0) {
        format += "LC%02d_";
    } else {
        format += "L%02d_";
    }

    // U and R
    format += "U%d_R%d";

    char filename[40] = "";
    sprintf(filename,
            format.c_str(),
            glm::abs(tile.geoCell.latitude),
            glm::abs(tile.geoCell.longitude),
            tile.encodedCDBDataset,
            tile.componentSelectors[0],
            tile.componentSelectors[1],
            glm::abs(tile.level),
            tile.x,
            tile.y);

    return filename;
}

boost::filesystem::path CDB::TileLevelToPath(int level)
{
    if (level < 0) {
        return "LC";
    }

    char path[16] = "";
    sprintf(path, "L%02d", level);
    return path;
}

BoundRegion CDB::CalculateTileExtent(const CDBGeoCell &geoCell, int level, int x, int y)
{
    double dist = 1.0;
    if (level > 0) {
        dist = glm::pow(2.0, -level);
    }

    double minLongitudeDegree = geoCell.longitude + y * dist;
    double minLatitudeDegree = geoCell.latitude + x * dist;
    Cartographic min(glm::radians(minLongitudeDegree), glm::radians(minLatitudeDegree), 0.0);
    Cartographic max(glm::radians(minLongitudeDegree + dist), glm::radians(minLatitudeDegree + dist), 0.0);
    BoundRegion region;
    region.merge(min);
    region.merge(max);

    return region;
}

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
    tile.encodedCDBName = encodedCDBFullName;
    tile.encodedCDBDataset = encodedDataset;
    tile.componentSelectors[0] = componentSelector1;
    tile.componentSelectors[1] = componentSelector2;
    tile.level = negativeLevel ? -level : level;
    tile.x = x;
    tile.y = y;

    return tile;
}

void CloseZipArchive(zip_t *archive) noexcept
{
    if (archive) {
        zip_close(archive);
    }
}

void CloseZipFile(zip_file *file) noexcept
{
    if (file) {
        zip_fclose(file);
    }
}

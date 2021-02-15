#include "CDB.h"
#include <iostream>
#include <string.h>
#include <unordered_set>
#include <utility>
#include <vector>

namespace CDBTo3DTiles {

const std::filesystem::path CDB::TILES = "Tiles";
const std::filesystem::path CDB::METADATA = "Metadata";
const std::filesystem::path CDB::GTModel = "GTModel";

CDB::CDB(const std::filesystem::path &path)
    : m_path{path}
{
    m_GTModelCache = CDBGTModelCache(path);
}

void CDB::forEachGeoCell(std::function<void(CDBGeoCell)> process)
{
    std::filesystem::path tilesPath = m_path / TILES;

    if (!std::filesystem::exists(tilesPath) || !std::filesystem::is_directory(tilesPath)) {
        throw std::runtime_error(tilesPath.string() + " directory does not exist");
    }

    for (std::filesystem::directory_entry geoCellLatDir : std::filesystem::directory_iterator(tilesPath)) {
        auto geoCellLatitude = CDBGeoCell::parseLatFromFilename(geoCellLatDir.path().filename().string());
        if (!geoCellLatitude) {
            continue;
        }

        for (std::filesystem::directory_entry geoCellLongDir :
             std::filesystem::directory_iterator(geoCellLatDir)) {
            auto geoCellLongitude = CDBGeoCell::parseLongFromFilename(
                geoCellLongDir.path().filename().string());
            if (!geoCellLongitude) {
                continue;
            }
            std::cout << "processing geocell" << std::endl;

            process(CDBGeoCell(*geoCellLatitude, *geoCellLongitude));
            std::cout << "done processing" << std::endl;
        }
    }
}

void CDB::forEachElevationTile(const CDBGeoCell &geoCell, std::function<void(CDBElevation)> process)
{
    forEachDatasetTile(geoCell, CDBDataset::Elevation, [&](const std::filesystem::path &elevationTilePath) {
        std::optional<CDBElevation> elevation = CDBElevation::createFromFile(elevationTilePath);
        if (elevation) {
            process(std::move(*elevation));
        }
    });
}

void CDB::forEachGTModelTile(const CDBGeoCell &geoCell, std::function<void(CDBGTModels)> process)
{
    std::unordered_map<size_t, CDBTileset> tilesets;
    forEachDatasetTile(geoCell, CDBDataset::GTFeature, [&](const std::filesystem::path &GTFeaturePath) {
        if (GTFeaturePath.extension() != ".dbf") {
            return;
        }

        auto tile = CDBTile::createFromFile(GTFeaturePath.stem().string());
        if (!tile) {
            return;
        }

        // we only supports point features for now
        if (tile->getCS_2() == static_cast<int>(CDBVectorCS2::PointFeature)) {
            tile->setCustomContentURI(GTFeaturePath);

            size_t CSHash = 0;
            hashCombine(CSHash, tile->getCS_1());
            hashCombine(CSHash, tile->getCS_2());
            tilesets[CSHash].insertTile(*tile);
        }
    });

    for (const auto &tileset : tilesets) {
        traverseModelsAttributes(tileset.second.getRoot(),
                                 nullptr,
                                 nullptr,
                                 [&](CDBModelsAttributes modelsAttributes) {
                                     auto models = CDBGTModels::createFromModelsAttributes(modelsAttributes,
                                                                                           &*m_GTModelCache);
                                     if (models) {
                                         process(std::move(*models));
                                     }
                                 });
    }
}

void CDB::forEachGSModelTile(const CDBGeoCell &geoCell, std::function<void(CDBGSModels)> process)
{
    std::unordered_map<size_t, CDBTileset> tilesets;
    forEachDatasetTile(geoCell, CDBDataset::GSFeature, [&](const std::filesystem::path &GSFeaturePath) {
        if (GSFeaturePath.extension() != ".dbf") {
            return;
        }

        auto tile = CDBTile::createFromFile(GSFeaturePath.stem().string());
        if (!tile) {
            return;
        }

        // we only supports point features for now
        if (tile->getCS_2() == static_cast<int>(CDBVectorCS2::PointFeature)) {
            tile->setCustomContentURI(GSFeaturePath);

            size_t CSHash = 0;
            hashCombine(CSHash, tile->getCS_1());
            hashCombine(CSHash, tile->getCS_2());
            tilesets[CSHash].insertTile(*tile);
        }
    });

    for (const auto &tileset : tilesets) {
        traverseModelsAttributes(tileset.second.getRoot(),
                                 nullptr,
                                 nullptr,
                                 [&](CDBModelsAttributes modelAttribute) {
                                     auto models = CDBGSModels::createFromModelsAttributes(modelAttribute,
                                                                                           m_path);
                                     if (models) {
                                         process(std::move(*models));
                                     }
                                 });
    }
}

void CDB::forEachRoadNetworkTile(const CDBGeoCell &geoCell, std::function<void(CDBGeometryVectors)> process)
{
    forEachDatasetTile(geoCell, CDBDataset::RoadNetwork, [&](const std::filesystem::path &roadNetworkTilePath) {
        std::optional<CDBGeometryVectors> roadNetwork = CDBGeometryVectors::createFromFile(roadNetworkTilePath,
                                                                                           m_path);
        if (roadNetwork) {
            process(std::move(*roadNetwork));
        }
    });
}

void CDB::forEachRailRoadNetworkTile(const CDBGeoCell &geoCell,
                                     std::function<void(CDBGeometryVectors)> process)
{
    forEachDatasetTile(geoCell,
                       CDBDataset::RailRoadNetwork,
                       [&](const std::filesystem::path &railRoadNetworkTilePath) {
                           std::optional<CDBGeometryVectors> railRoadNetwork
                               = CDBGeometryVectors::createFromFile(railRoadNetworkTilePath, m_path);
                           if (railRoadNetwork) {
                               process(std::move(*railRoadNetwork));
                           }
                       });
}

void CDB::forEachPowerlineNetworkTile(const CDBGeoCell &geoCell,
                                      std::function<void(CDBGeometryVectors)> process)
{
    forEachDatasetTile(geoCell,
                       CDBDataset::PowerlineNetwork,
                       [&](const std::filesystem::path &powerlineNetworkTilePath) {
                           std::optional<CDBGeometryVectors> powerlineNetwork
                               = CDBGeometryVectors::createFromFile(powerlineNetworkTilePath, m_path);
                           if (powerlineNetwork) {
                               process(std::move(*powerlineNetwork));
                           }
                       });
}

void CDB::forEachHydrographyNetworkTile(const CDBGeoCell &geoCell,
                                        std::function<void(CDBGeometryVectors)> process)
{
    forEachDatasetTile(geoCell,
                       CDBDataset::HydrographyNetwork,
                       [&](const std::filesystem::path &hydrographyNetworkTilePath) {
                           std::optional<CDBGeometryVectors> hydrographyNetwork
                               = CDBGeometryVectors::createFromFile(hydrographyNetworkTilePath, m_path);
                           if (hydrographyNetwork) {
                               process(std::move(*hydrographyNetwork));
                           }
                       });
}

void CDB::traverseModelsAttributes(const CDBTile *root,
                                   const CDBTile *oldElevationTile,
                                   GDALDataset *oldElevationDataset,
                                   std::function<void(CDBModelsAttributes)> process)
{
    if (root == nullptr) {
        return;
    }

    const auto &featureFile = root->getCustomContentURI();
    if (featureFile) {
        GDALDatasetUniquePtr attributesDataset = GDALDatasetUniquePtr(
            (GDALDataset *) GDALOpenEx(featureFile->c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));

        if (attributesDataset) {
            CDBModelsAttributes model(std::move(attributesDataset), *root, m_path);
            if (model.getInstancesAttributes().getInstancesCount() > 0) {
                CDBTile currentElevation = CDBTile(root->getGeoCell(),
                                                   CDBDataset::Elevation,
                                                   1,
                                                   1,
                                                   root->getLevel(),
                                                   root->getUREF(),
                                                   root->getRREF());

                auto currentElevationPath = m_path / (currentElevation.getRelativePath().string() + ".tif");
                if (!std::filesystem::exists(currentElevationPath)) {
                    // reuse the previous read parent elevation if there is any
                    if (oldElevationTile) {
                        for (auto &point : model.getCartographicPositions()) {
                            clampPointOnElevation(*oldElevationTile, *oldElevationDataset, point);
                        }

                        process(std::move(model));
                        for (auto child : root->getChildren()) {
                            traverseModelsAttributes(child, oldElevationTile, oldElevationDataset, process);
                        }
                    } else {
                        // find the parent elevation to clamp on if no current elevation is found
                        auto parentElevation = queryParentElevationTiles(currentElevation);
                        GDALDatasetUniquePtr elevationData;
                        if (parentElevation) {
                            auto elevationFile = m_path
                                                 / (parentElevation->getRelativePath().string() + ".tif");
                            elevationData = GDALDatasetUniquePtr(
                                (GDALDataset *) GDALOpen(elevationFile.c_str(), GDALAccess::GA_ReadOnly));

                            if (elevationData) {
                                for (auto &point : model.getCartographicPositions()) {
                                    clampPointOnElevation(*parentElevation, *elevationData, point);
                                }
                            }
                        }

                        process(std::move(model));
                        for (auto child : root->getChildren()) {
                            if (parentElevation) {
                                traverseModelsAttributes(child,
                                                         &*parentElevation,
                                                         elevationData.get(),
                                                         process);
                            } else {
                                traverseModelsAttributes(child, nullptr, nullptr, process);
                            }
                        }
                    }

                    return;
                }

                // find the highest possible elevation levels to clamp. Only do this for leaf since elevation
                // can have higher levels than GTFeature. As GTFeature stops refine, terrain
                // can continue refining due to higher LOD and completely cover low level GTFeature point, making the GTModel sink inside the terrain mesh.
                // We don't need to do this for non-leaf since it will be replaced by higher level anyway due to
                // replace refinement
                CDBTileset underlyingElevations(root->getLevel(), root->getUREF(), root->getRREF());
                if (root->getChildren().empty()) {
                    queryElevationTiles(currentElevation, underlyingElevations);
                } else {
                    underlyingElevations.insertTile(currentElevation);
                }

                clampPointsOnElevationTileset(model.getCartographicPositions(), underlyingElevations);
                process(std::move(model));
            }
        }
    }

    for (auto child : root->getChildren()) {
        traverseModelsAttributes(child, nullptr, nullptr, process);
    }
}

void CDB::queryElevationTiles(const CDBTile &elevationTile, CDBTileset &underlyingElevations)
{
    if (elevationTile.getLevel() < 0) {
        CDBTile child = CDBTile::createChildForNegativeLOD(elevationTile);
        if (isElevationExist(child)) {
            return queryElevationTiles(child, underlyingElevations);
        }

        underlyingElevations.insertTile(elevationTile);
        return;
    }

    bool hasChildren = false;

    CDBTile northWest = CDBTile::createNorthWestForPositiveLOD(elevationTile);
    if (isElevationExist(northWest)) {
        hasChildren = true;
        queryElevationTiles(northWest, underlyingElevations);
    }

    CDBTile northEast = CDBTile::createNorthEastForPositiveLOD(elevationTile);
    if (isElevationExist(northEast)) {
        hasChildren = true;
        queryElevationTiles(northEast, underlyingElevations);
    }

    CDBTile southWest = CDBTile::createSouthWestForPositiveLOD(elevationTile);
    if (isElevationExist(southWest)) {
        hasChildren = true;
        queryElevationTiles(southWest, underlyingElevations);
    }

    CDBTile southEast = CDBTile::createSouthEastForPositiveLOD(elevationTile);
    if (isElevationExist(southEast)) {
        hasChildren = true;
        queryElevationTiles(southEast, underlyingElevations);
    }

    if (!hasChildren) {
        underlyingElevations.insertTile(elevationTile);
    }
}

std::optional<CDBTile> CDB::queryParentElevationTiles(const CDBTile &elevationTile)
{
    if (elevationTile.getLevel() <= -10) {
        return std::nullopt;
    }

    int parentLevel = elevationTile.getLevel() - 1;
    int parentUREF, parentRREF;
    if (elevationTile.getLevel() <= 0) {
        parentUREF = elevationTile.getUREF();
        parentRREF = elevationTile.getRREF();
    } else {
        parentUREF = elevationTile.getUREF() / 2;
        parentRREF = elevationTile.getRREF() / 2;
    }

    CDBTile parentTile(elevationTile.getGeoCell(),
                       elevationTile.getDataset(),
                       elevationTile.getCS_1(),
                       elevationTile.getCS_2(),
                       parentLevel,
                       parentUREF,
                       parentRREF);
    if (isElevationExist(parentTile)) {
        return parentTile;
    }

    return queryParentElevationTiles(parentTile);
}

void CDB::clampPointsOnElevationTileset(std::vector<Core::Cartographic> &points,
                                        const CDBTileset &elevationTileset)
{
    if (elevationTileset.getRoot()) {
        std::unordered_map<CDBTile, std::vector<size_t>> elevationToClamp;
        for (size_t i = 0; i < points.size(); ++i) {
            auto elevationTile = elevationTileset.getFitTile(points[i]);
            if (elevationTile) {
                elevationToClamp[*elevationTile].emplace_back(i);
            }
        }

        for (const auto &elevation : elevationToClamp) {
            const auto &elevationTile = elevation.first;
            auto elevationFile = m_path / (elevationTile.getRelativePath().string() + ".tif");
            GDALDatasetUniquePtr rasterData = GDALDatasetUniquePtr(
                (GDALDataset *) GDALOpen(elevationFile.c_str(), GDALAccess::GA_ReadOnly));

            if (rasterData == nullptr) {
                continue;
            }

            auto heightBand = rasterData->GetRasterBand(1);
            auto rasterDataType = heightBand->GetRasterDataType();
            if (rasterDataType != GDT_Float32 && rasterDataType != GDT_Float64) {
                return;
            }

            for (auto i : elevation.second) {
                clampPointOnElevation(elevationTile, *rasterData, points[i]);
            }
        }
    }
}

void CDB::clampPointOnElevation(const CDBTile &elevationTile,
                                GDALDataset &elevationDataset,
                                Core::Cartographic &point)
{
    const Core::BoundingRegion &region = elevationTile.getBoundRegion();
    const Core::GlobeRectangle &rectangle = region.getRectangle();
    auto heightBand = elevationDataset.GetRasterBand(1);
    double rectangleWidth = rectangle.computeWidth();
    double rectangleHeight = rectangle.computeHeight();
    int rasterXSize = heightBand->GetXSize();
    int rasterYSize = heightBand->GetYSize();
    double gapX = rectangleWidth / rasterXSize;
    double gapY = rectangleHeight / rasterYSize;

    int x = static_cast<int>(glm::floor((point.longitude - rectangle.getWest()) / gapX));
    x = glm::min(x, rasterXSize - 1);
    int y = static_cast<int>(glm::floor((point.latitude - rectangle.getSouth()) / gapY));
    y = glm::min(rasterYSize - y - 1, rasterYSize - 1);
    double height = 0.0;
    if (GDALRasterIO(heightBand, GDALRWFlag::GF_Read, x, y, 1, 1, &height, 1, 1, GDALDataType::GDT_Float64, 0, 0)
        != CE_None) {
        height = 0.0;
    }

    point.height = height;
}

bool CDB::isElevationExist(const CDBTile &tile) const
{
    CDBTile elevationTile = CDBTile(tile.getGeoCell(),
                                    CDBDataset::Elevation,
                                    1,
                                    1,
                                    tile.getLevel(),
                                    tile.getUREF(),
                                    tile.getRREF());

    auto elevationPath = m_path / (elevationTile.getRelativePath().string() + ".tif");
    return std::filesystem::exists(elevationPath);
}

bool CDB::isImageryExist(const CDBTile &tile) const
{
    CDBTile imageryTile = CDBTile(tile.getGeoCell(),
                                  CDBDataset::Imagery,
                                  1,
                                  1,
                                  tile.getLevel(),
                                  tile.getUREF(),
                                  tile.getRREF());

    auto imagery = m_path / (imageryTile.getRelativePath().string() + ".jp2");
    return std::filesystem::exists(imagery);
}

std::optional<CDBImagery> CDB::getImagery(const CDBTile &tile) const
{
    CDBTile imageryTile = CDBTile(tile.getGeoCell(),
                                  CDBDataset::Imagery,
                                  1,
                                  1,
                                  tile.getLevel(),
                                  tile.getUREF(),
                                  tile.getRREF());

    auto imageryPath = m_path / (imageryTile.getRelativePath().string() + ".jp2");
    if (!std::filesystem::exists(imageryPath)) {
        return std::nullopt;
    }

    auto imageryDataset = GDALDatasetUniquePtr(
        (GDALDataset *) GDALOpen(imageryPath.c_str(), GDALAccess::GA_ReadOnly));

    if (!imageryDataset) {
        return std::nullopt;
    }

    return CDBImagery(std::move(imageryDataset), imageryTile);
}

void CDB::forEachDatasetTile(const CDBGeoCell &geoCell,
                             CDBDataset dataset,
                             std::function<void(const std::filesystem::path &)> process)
{
    auto datasetPath = m_path / geoCell.getRelativePath() / getCDBDatasetDirectoryName(dataset);
    if (!std::filesystem::exists(datasetPath) || !std::filesystem::is_directory(datasetPath)) {
        return;
    }

    for (std::filesystem::directory_entry levelDir : std::filesystem::directory_iterator(datasetPath)) {
        if (!std::filesystem::is_directory(levelDir)) {
            continue;
        }

        for (std::filesystem::directory_entry UREFDir : std::filesystem::directory_iterator(levelDir)) {
            if (!std::filesystem::is_directory(UREFDir)) {
                continue;
            }

            for (std::filesystem::directory_entry tilePath : std::filesystem::directory_iterator(UREFDir)) {
                process(tilePath);
            }
        }
    }
}
} // namespace CDBTo3DTiles

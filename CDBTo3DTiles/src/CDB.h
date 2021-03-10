#pragma once

#include "CDBElevation.h"
#include "CDBGeometryVectors.h"
#include "CDBImagery.h"
#include "CDBModels.h"
#include "CDBTileset.h"
#include <filesystem>
#include <functional>
#include <optional>
#include <stack>
#include <string>

namespace CDBTo3DTiles {

class CDB
{
public:
    explicit CDB(const std::filesystem::path &path);

    void forEachGeoCell(std::function<void(CDBGeoCell geoCell)> process);

    void forEachElevationTile(const CDBGeoCell &geoCell, std::function<void(CDBElevation)> process);

    void forEachGTModelTile(const CDBGeoCell &geoCell, std::function<void(CDBGTModels)> process);

    void forEachGSModelTile(const CDBGeoCell &geoCell, std::function<void(CDBGSModels)> process);

    void forEachRoadNetworkTile(const CDBGeoCell &geoCell, std::function<void(CDBGeometryVectors)> process);

    void forEachRailRoadNetworkTile(const CDBGeoCell &geoCell,
                                    std::function<void(CDBGeometryVectors)> process);

    void forEachPowerlineNetworkTile(const CDBGeoCell &geoCell,
                                     std::function<void(CDBGeometryVectors)> process);

    void forEachHydrographyNetworkTile(const CDBGeoCell &geoCell,
                                       std::function<void(CDBGeometryVectors)> process);

    bool isElevationExist(const CDBTile &elevationTile) const;

    bool isImageryExist(const CDBTile &tile) const;

    bool isGSModelExist(const CDBTile &tile) const;

    std::optional<CDBImagery> getImagery(const CDBTile &tile) const;

    static const std::filesystem::path TILES;
    static const std::filesystem::path METADATA;
    static const std::filesystem::path GTModel;

private:
    void traverseModelsAttributes(const CDBTile *root,
                                  const CDBTile *oldElevationTile,
                                  GDALDataset *oldElevationDataset,
                                  std::function<void(CDBModelsAttributes)> process);

    void queryElevationTiles(const CDBTile &elevationTile, CDBTileset &underlyingElevations);

    std::optional<CDBTile> queryParentElevationTiles(const CDBTile &elevationTile);

    void clampPointsOnElevationTileset(std::vector<Core::Cartographic> &points,
                                       const CDBTileset &elevationTileset);

    void clampPointOnElevation(const CDBTile &elevationTile,
                               GDALDataset &elevationDataset,
                               Core::Cartographic &point);

    void forEachDatasetTile(const CDBGeoCell &geoCell,
                            CDBDataset dataset,
                            std::function<void(const std::filesystem::path &)> process);

    std::optional<CDBGTModelCache> m_GTModelCache;
    std::filesystem::path m_path;
};
} // namespace CDBTo3DTiles


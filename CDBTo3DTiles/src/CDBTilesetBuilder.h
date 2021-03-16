#pragma once

#include <filesystem>
#include <vector>
#include "CDB.h"
#include "Gltf.h"

using namespace CDBTo3DTiles;
class CDBTilesetBuilder
{
  public:
    struct TilesetCollection
    {
      std::unordered_map<size_t, std::filesystem::path> CSToPaths;
      std::unordered_map<size_t, CDBTileset> CSToTilesets;
    };
    CDBTilesetBuilder(const std::filesystem::path &cdbInputPath, const std::filesystem::path &output)
        : elevationNormal{false}
        , elevationLOD{false}
        , use3dTilesNext{false}
        , subtreeLevels{7}
        , elevationDecimateError{0.01f}
        , elevationThresholdIndices{0.3f}
        , cdbPath{cdbInputPath}
        , outputPath{output}
    {
        if (std::filesystem::exists(output)) {
            std::filesystem::remove_all(output);
        }
    }

    void flushTilesetCollection(const CDBGeoCell &geoCell,
                                std::unordered_map<CDBGeoCell, TilesetCollection> &tilesetCollections,
                                bool replace = true);

    void addElevationAvailability(CDBElevation &elevation, const CDB &cdb,
      uint8_t* nodeAvailabilityBuffer, uint8_t* childSubtreeAvailabilityBuffer, 
      uint64_t* availableNodeCount, uint64_t* availableChildCount,
      int subtreeRootLevel,
      int subtreeRootX,
      int subtreeRootY);

    void addElevationToTilesetCollection(CDBElevation &elevation,
                                         const CDB &cdb,
                                         const std::filesystem::path &outputDirectory);

    void addElevationToTileset(CDBElevation &elevation,
                               const Texture *imagery,
                               const CDB &cdb,
                               const std::filesystem::path &outputDirectory,
                               CDBTileset &tileset);

    void fillMissingPositiveLODElevation(const CDBElevation &elevation,
                                         const Texture *currentImagery,
                                         const CDB &cdb,
                                         const std::filesystem::path &outputDirectory,
                                         CDBTileset &tileset);

    void fillMissingNegativeLODElevation(CDBElevation &elevation,
                                         const CDB &cdb,
                                         const std::filesystem::path &outputDirectory,
                                         CDBTileset &tileset);

    void addSubRegionElevationToTileset(CDBElevation &subRegion,
                                        const CDB &cdb,
                                        std::optional<CDBImagery> &subRegionImagery,
                                        const Texture *parentTexture,
                                        const std::filesystem::path &outputDirectory,
                                        CDBTileset &tileset);

    void generateElevationNormal(Mesh &simplifed);

    Texture createImageryTexture(CDBImagery &imagery, const std::filesystem::path &tilesetDirectory) const;
    Texture createFeatureIDTexture(CDBRMTexture &rmTexture,
                                                  const std::filesystem::path &tilesetOutputDirectory) const;

    void addVectorToTilesetCollection(const CDBGeometryVectors &vectors,
                                      const std::filesystem::path &collectionOutputDirectory,
                                      std::unordered_map<CDBGeoCell, TilesetCollection> &tilesetCollections);

    std::vector<Texture> writeModeTextures(const std::vector<Texture> &modelTextures,
                                           const std::vector<osg::ref_ptr<osg::Image>> &images,
                                           const std::filesystem::path &textureSubDir,
                                           const std::filesystem::path &gltfPath);

    void addGTModelToTilesetCollection(const CDBGTModels &model, const std::filesystem::path &outputDirectory);

    void addGSModelToTilesetCollection(const CDBGSModels &model, const std::filesystem::path &outputDirectory);

    void createB3DMForTileset(tinygltf::Model &model,
                              CDBTile cdbTile,
                              const CDBInstancesAttributes *instancesAttribs,
                              const std::filesystem::path &outputDirectory,
                              CDBTileset &tilesetCollections);

    size_t hashComponentSelectors(int CS_1, int CS_2);

    std::filesystem::path getTilesetDirectory(int CS_1,
                                              int CS_2,
                                              const std::filesystem::path &collectionOutputDirectory);

    void getTileset(const CDBTile &cdbTile,
                    const std::filesystem::path &outputDirectory,
                    std::unordered_map<CDBGeoCell, TilesetCollection> &tilesetCollections,
                    CDBTileset *&tileset,
                    std::filesystem::path &path);

    static const std::string ELEVATIONS_PATH;
    static const std::string ROAD_NETWORK_PATH;
    static const std::string RAILROAD_NETWORK_PATH;
    static const std::string POWERLINE_NETWORK_PATH;
    static const std::string HYDROGRAPHY_NETWORK_PATH;
    static const std::string GTMODEL_PATH;
    static const std::string GSMODEL_PATH;
    static const std::unordered_set<std::string> DATASET_PATHS;

    bool elevationNormal;
    bool elevationLOD;
    bool use3dTilesNext;
    int subtreeLevels;
    int maxLevel;
    float elevationDecimateError;
    float elevationThresholdIndices;
    std::filesystem::path cdbPath;
    std::filesystem::path outputPath;
    std::vector<std::filesystem::path> defaultDatasetToCombine;
    std::vector<std::vector<std::string>> requestedDatasetToCombine;
    std::unordered_set<std::string> processedModelTextures;
    std::unordered_map<CDBTile, Texture> processedParentImagery;
    std::unordered_map<std::string, std::filesystem::path> GTModelsToGltf;
    std::unordered_map<CDBGeoCell, TilesetCollection> elevationTilesets;
    std::unordered_map<CDBGeoCell, TilesetCollection> roadNetworkTilesets;
    std::unordered_map<CDBGeoCell, TilesetCollection> railRoadNetworkTilesets;
    std::unordered_map<CDBGeoCell, TilesetCollection> powerlineNetworkTilesets;
    std::unordered_map<CDBGeoCell, TilesetCollection> hydrographyNetworkTilesets;
    std::unordered_map<CDBGeoCell, TilesetCollection> GTModelTilesets;
    std::unordered_map<CDBGeoCell, TilesetCollection> GSModelTilesets;
};
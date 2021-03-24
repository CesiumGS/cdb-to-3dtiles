#include "CDB.h"
#include "CDBTilesetBuilder.h"
#include "Gltf.h"
#include "Math.h"
#include "TileFormatIO.h"
#include "gdal.h"
#include "osgDB/WriteFile"
#include <morton.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>
using json = nlohmann::json;
using namespace CDBTo3DTiles;

const std::string CDBTilesetBuilder::ELEVATIONS_PATH = "Elevation";
const std::string CDBTilesetBuilder::ROAD_NETWORK_PATH = "RoadNetwork";
const std::string CDBTilesetBuilder::RAILROAD_NETWORK_PATH = "RailRoadNetwork";
const std::string CDBTilesetBuilder::POWERLINE_NETWORK_PATH = "PowerlineNetwork";
const std::string CDBTilesetBuilder::HYDROGRAPHY_NETWORK_PATH = "HydrographyNetwork";
const std::string CDBTilesetBuilder::GTMODEL_PATH = "GTModels";
const std::string CDBTilesetBuilder::GSMODEL_PATH = "GSModels";
const int CDBTilesetBuilder::MAX_LEVEL = 23;

const std::unordered_set<std::string> CDBTilesetBuilder::DATASET_PATHS = {ELEVATIONS_PATH,
                                                                        ROAD_NETWORK_PATH,
                                                                        RAILROAD_NETWORK_PATH,
                                                                        POWERLINE_NETWORK_PATH,
                                                                        HYDROGRAPHY_NETWORK_PATH,
                                                                        GTMODEL_PATH,
                                                                        GSMODEL_PATH};
void CDBTilesetBuilder::flushTilesetCollection(
    const CDBGeoCell &geoCell,
    std::unordered_map<CDBGeoCell, TilesetCollection> &tilesetCollections,
    bool replace)
{
    auto geoCellCollectionIt = tilesetCollections.find(geoCell);
    if (geoCellCollectionIt != tilesetCollections.end()) {
        const auto &tilesetCollection = geoCellCollectionIt->second;
        const auto &CSToPaths = tilesetCollection.CSToPaths;
        for (const auto &CSTotileset : tilesetCollection.CSToTilesets) {
            const auto &tileset = CSTotileset.second;
            auto root = tileset.getRoot();
            if (!root) {
                continue;
            }

            auto tilesetDirectory = CSToPaths.at(CSTotileset.first);
            auto tilesetJsonPath = tilesetDirectory
                                   / (CDBTile::retrieveGeoCellDatasetFromTileName(*root) + ".json");

            // write to tileset.json file
            std::ofstream fs(tilesetJsonPath);

            writeToTilesetJson(tileset, replace, fs, use3dTilesNext, subtreeLevels);

            // add tileset json path to be combined later for multiple geocell
            // remove the output root path to become relative path
            tilesetJsonPath = std::filesystem::relative(tilesetJsonPath, outputPath);
            defaultDatasetToCombine.emplace_back(tilesetJsonPath);
        }

        tilesetCollections.erase(geoCell);
    }
}

std::vector<std::string> CDBTilesetBuilder::flushDatasetGroupTilesetCollections(const CDBGeoCell &geoCell,
    datasetGroup &group,
    std::string datasetGroupName)
{
    std::vector<CDBDataset> &datasets = group.datasets;
    std::vector<std::filesystem::path> &tilesetsToCombine = group.tilesetsToCombine;
    bool replace = group.replace;
    // key is level. Value is bounding region for the level
    std::map<int, Core::BoundingRegion> levelBoundingRegion;
    auto tilesetDirectory = outputPath / geoCell.getRelativePath();
    std::map<int, std::vector<std::string>> urisAtEachLevel;
    for(CDBDataset dataset : datasets)
    {
        std::unordered_map<CDBGeoCell, TilesetCollection> *tilesets = datasetTilesetCollections.at(dataset);
        if (tilesets->count(geoCell) == 0)
        {
            continue;
        }
        TilesetCollection &tilesetCollection = tilesets->at(geoCell);
        for (auto & CSToTilesets : tilesetCollection.CSToTilesets)
        {
            size_t key = CSToTilesets.first;
            CDBTileset *tileset = &CSToTilesets.second;
            const auto root = tileset->getRoot();
            if(!root)
                continue;
            for(int level = root->getLevel() ; level < MAX_LEVEL ; level++)
            {
                const CDBTile *tile = tileset->getFirstTileAtLevel(level);
                if(tile)
                {
                    if(levelBoundingRegion.count(level) == 0)
                    {
                        levelBoundingRegion.insert(std::pair<int, Core::BoundingRegion>(level, tile->getBoundRegion()));
                    }
                    else
                    {
                        levelBoundingRegion.at(level) = levelBoundingRegion.at(level).computeUnion(tile->getBoundRegion());
                    }
                    const std::filesystem::path *contentURI = tile->getCustomContentURI();
                    if(contentURI)
                    {
                        // All implicitly defined URIs (positive level) will be defined at level 0 tile
                        int implicitAdjustedLevel = level;
                        if (level > 0)
                            implicitAdjustedLevel = 0;
                        if(urisAtEachLevel.count(implicitAdjustedLevel) == 0)
                            urisAtEachLevel.insert(std::pair<int, std::vector<std::string>>(implicitAdjustedLevel, std::vector<std::string>()));
                        std::filesystem::path relativeContentPath = std::filesystem::relative(tilesetCollection.CSToPaths.at(key), tilesetDirectory);
                        urisAtEachLevel.at(implicitAdjustedLevel).emplace_back(relativeContentPath / (*contentURI));
                    }
                }
            }
        }
        tilesets->erase(geoCell);
    }
    if(urisAtEachLevel.empty()) // nothing in the tileset
        return {};

    CDBTile geoCellTile(geoCell, CDBDataset::MultipleContents, 1, 1, group.maxLevel, 0, 0);
    CDBTileset multiContentTileset;
    multiContentTileset.insertTile(geoCellTile);
    for(auto &[level, boundingRegion] : levelBoundingRegion)
    {
        multiContentTileset.getFirstTileAtLevel(level)->setBoundRegion(boundingRegion);
    }

    auto tilesetJsonPath = tilesetDirectory
                            / (geoCell.getLatitudeDirectoryName() + geoCell.getLongitudeDirectoryName() + "_" + datasetGroupName + ".json");

    // write to geocell json file
    std::ofstream fs(tilesetJsonPath);

    writeToTilesetJson(multiContentTileset, replace, fs, use3dTilesNext, subtreeLevels, group.maxLevel, urisAtEachLevel, datasetGroupName);

    // add tileset json path to be combined later for multiple geocell
    // remove the output root path to become relative path
    tilesetJsonPath = std::filesystem::relative(tilesetJsonPath, outputPath);
    tilesetsToCombine.emplace_back(tilesetJsonPath);

    if(urisAtEachLevel.count(0) != 0)
        return urisAtEachLevel.at(0);
    return {};
}


std::map<std::string, std::vector<std::string>> CDBTilesetBuilder::flushTilesetCollectionsMultiContent(const CDBGeoCell &geoCell)
// Write geocell json with implicit multicontent root for each dataset group
{
    // group name -> vector of URIs at level 0
    std::map<std::string, std::vector<std::string>> groupImplicitURIs;
    for(auto &[groupName, group] : datasetGroups)
    {
        for(CDBDataset dataset : group.datasets)
            if(datasetMaxLevels.count(dataset) != 0)
                group.maxLevel = std::max(group.maxLevel, datasetMaxLevels.at(dataset));
        std::vector<std::string> implicitURIs = flushDatasetGroupTilesetCollections(geoCell, group, groupName);
        groupImplicitURIs.insert(std::pair<std::string, std::vector<std::string>>(
            groupName, implicitURIs
        ));
    }
    return groupImplicitURIs;
}

std::string CDBTilesetBuilder::levelXYtoSubtreeKey(int level, int x, int y)
{
    return std::to_string(level) + "_" + std::to_string(x) + "_"
                                 + std::to_string(y);
}

std::string CDBTilesetBuilder::cs1cs2ToCSKey(int cs1, int cs2)
{
    return std::to_string(cs1) + "_" + std::to_string(cs2);
}

void CDBTilesetBuilder::addAvailability(
    const CDBTile &cdbTile)
{
    CDBDataset dataset = cdbTile.getDataset();
    if (datasetTilesetCollections.count(dataset) == 0) {
        throw std::invalid_argument("Not implemented yet for that dataset.");
    }
    if(datasetCSSubtrees.count(dataset) == 0)
        datasetCSSubtrees.insert(std::pair<CDBDataset, std::map<std::string, std::map<std::string, subtreeAvailability>>>(
            dataset,
            {}
        ));
    std::map<std::string, std::map<std::string, subtreeAvailability>> &csSubtrees = datasetCSSubtrees.at(dataset);

    std::string csKey = cs1cs2ToCSKey(cdbTile.getCS_1(), cdbTile.getCS_2());
    if (csSubtrees.count(csKey) == 0)
    {
        csSubtrees.insert(std::pair<std::string, std::map<std::string, subtreeAvailability>>(
            csKey, std::map<std::string, subtreeAvailability>{}));
    }

    std::map<std::string, subtreeAvailability> &subtreeMap = csSubtrees.at(csKey);

    int level = cdbTile.getLevel();

    if(datasetMaxLevels.count(dataset) == 0)
        datasetMaxLevels.insert(std::pair<CDBDataset, int>(dataset, 0));
    datasetMaxLevels.at(dataset) = std::max(datasetMaxLevels.at(dataset), level);

    int x = cdbTile.getRREF();
    int y = cdbTile.getUREF();

    subtreeAvailability *subtree;

    if (level >= 0) {
        // get the root of the subtree that this tile belongs to
        int subtreeRootLevel = (level / subtreeLevels) * subtreeLevels; // the level of the subtree root

        // from Volume 1: OGC CDB Core Standard: Model and Physical Data Store Structure page 120
        int levelWithinSubtree = level - subtreeRootLevel;
        int subtreeRootX = x / static_cast<int>(glm::pow(2, levelWithinSubtree));
        int subtreeRootY = y / static_cast<int>(glm::pow(2, levelWithinSubtree));

        std::string subtreeKey = levelXYtoSubtreeKey(subtreeRootLevel, subtreeRootX, subtreeRootY);
        if (subtreeMap.find(subtreeKey) == subtreeMap.end()) // the buffer isn't in the map
        {
            subtreeMap.insert(std::pair<std::string, subtreeAvailability>(subtreeKey, createSubtreeAvailability()));
        }

        subtree = &subtreeMap.at(subtreeKey);

        addDatasetAvailability(cdbTile,
                            subtree,
                            subtreeRootLevel,
                            subtreeRootX,
                            subtreeRootY);
                            // tileExists);
    }
}

void CDBTilesetBuilder::addDatasetAvailability(const CDBTile &cdbTile,
                                             subtreeAvailability *subtree,
                                             int subtreeRootLevel,
                                             int subtreeRootX,
                                             int subtreeRootY)
                                            //  bool (CDB::*tileExists)(const CDBTile &) const)
{
    if (subtree == NULL) {
        throw std::invalid_argument("Subtree availability pointer is null. Check if initialized.");
    }
    if (subtreeLevels < 1) {
        throw std::invalid_argument("Subtree level must be positive.");
    }
    int level = cdbTile.getLevel();
    int levelWithinSubtree = level - subtreeRootLevel;

    int localX = cdbTile.getRREF() - subtreeRootX * static_cast<int>(pow(2, levelWithinSubtree));
    int localY = cdbTile.getUREF() - subtreeRootY * static_cast<int>(pow(2, levelWithinSubtree));

    setBitAtXYLevelMorton(subtree->nodeBuffer, localX, localY, levelWithinSubtree);
    subtree->nodeCount += 1;

    std::string datasetGroupName = datasetToGroupName.at(cdbTile.getDataset());
    if(datasetGroupTileAndChildAvailabilities.count(datasetGroupName) == 0)
        datasetGroupTileAndChildAvailabilities.insert(
            std::pair<std::string, std::map<std::string, subtreeAvailability>>(
                datasetGroupName,
                std::map<std::string, subtreeAvailability>{}
            )
        );
    std::map<std::string, subtreeAvailability> &tileAndChildAvailabilities = 
        datasetGroupTileAndChildAvailabilities.at(datasetGroupName);
    std::string subtreeKey = levelXYtoSubtreeKey(subtreeRootLevel, subtreeRootX, subtreeRootY);
    createTileAndChildSubtreeAtKey(tileAndChildAvailabilities, subtreeKey);
    setBitAtXYLevelMorton(tileAndChildAvailabilities.at(subtreeKey).nodeBuffer,
        localX, localY, levelWithinSubtree);
    setParentBitsRecursively(tileAndChildAvailabilities, level, cdbTile.getRREF(), cdbTile.getUREF(),
        subtreeRootLevel, subtreeRootX, subtreeRootY);
}

bool CDBTilesetBuilder::setBitAtXYLevelMorton(std::vector<uint8_t> &buffer, int localX, int localY, int localLevel)
{
    const uint64_t mortonIndex = libmorton::morton2D_64_encode(localX, localY);
    // https://github.com/CesiumGS/3d-tiles/tree/3d-tiles-next/extensions/3DTILES_implicit_tiling/0.0.0#accessing-availability-bits
    const uint64_t nodeCountUpToThisLevel = (static_cast<uint64_t>(pow(4, localLevel)) - 1) / 3;

    const uint64_t index = nodeCountUpToThisLevel + mortonIndex;
    const uint64_t byte = index / 8;
    const uint64_t bit = index % 8;
    if(byte >= buffer.size())
        throw std::invalid_argument("x, y, level coordinates too large for given buffer.");
    int mask = (1 << bit);
    bool bitAlreadySet = (buffer[byte] & mask) >> bit == 1;
    const uint8_t availability = static_cast<uint8_t>(1 << bit);
    buffer[byte] |= availability;
    return bitAlreadySet;
}


void CDBTilesetBuilder::setParentBitsRecursively(std::map<std::string, subtreeAvailability> &tileAndChildAvailabilities,
        int level, int x, int y, int subtreeRootLevel, int subtreeRootX, int subtreeRootY)
{
    if(level == 0) // we reached the root tile
        return;
    if(level == subtreeRootLevel) // need to set childSubtree bit of parent subtree
    {
        subtreeRootLevel -= subtreeLevels;
        subtreeRootX /= static_cast<int>(glm::pow(2, subtreeLevels));
        subtreeRootY /= static_cast<int>(glm::pow(2, subtreeLevels));

        int localChildX = x - subtreeRootX * static_cast<int>(pow(2, subtreeLevels));
        int localChildY = y - subtreeRootY * static_cast<int>(pow(2, subtreeLevels));

        std::string subtreeKey = levelXYtoSubtreeKey(subtreeRootLevel, subtreeRootX, subtreeRootY);
        createTileAndChildSubtreeAtKey(tileAndChildAvailabilities, subtreeKey);
        setBitAtXYLevelMorton(tileAndChildAvailabilities[subtreeKey].childBuffer, localChildX, localChildY);
    }
    else
    {
        level -= 1;
        x /= 2;
        y /= 2;
        std::string subtreeKey = levelXYtoSubtreeKey(subtreeRootLevel, subtreeRootX, subtreeRootY);
        createTileAndChildSubtreeAtKey(tileAndChildAvailabilities, subtreeKey);

        int localLevel = level - subtreeRootLevel;
        int localX = x - subtreeRootX * static_cast<int>(pow(2, localLevel));
        int localY = y - subtreeRootY * static_cast<int>(pow(2, localLevel));

         bool bitAlreadySet = setBitAtXYLevelMorton(tileAndChildAvailabilities[subtreeKey].nodeBuffer, 
            localX, localY, localLevel);
        if(bitAlreadySet) // cut the recursion short
            return;
    }
    setParentBitsRecursively(tileAndChildAvailabilities, level, x, y, subtreeRootLevel, subtreeRootX, subtreeRootY);
}

void CDBTilesetBuilder::addElevationToTilesetCollection(CDBElevation &elevation,
                                                    //   const CDB &cdb,
                                                      const std::filesystem::path &collectionOutputDirectory)
{
    const auto &cdbTile = elevation.getTile();
    auto currentImagery = cdb->getImagery(cdbTile);

    std::filesystem::path tilesetDirectory;
    CDBTileset *tileset;
    getTileset(cdbTile, collectionOutputDirectory, elevationTilesets, tileset, tilesetDirectory);

    if (currentImagery) {
        Texture imageryTexture = createImageryTexture(*currentImagery, tilesetDirectory);
        addElevationToTileset(elevation, &imageryTexture, tilesetDirectory, *tileset);
    } else {
        // find parent imagery if the current one doesn't exist
        Texture *parentTexture = nullptr;
        auto current = CDBTile::createParentTile(cdbTile);
        while (current) {
            // if not in the cache, then write the image and save its name in the cache
            auto it = processedParentImagery.find(*current);
            if (it == processedParentImagery.end()) {
                auto parentImagery = cdb->getImagery(*current);
                if (parentImagery) {
                    auto newTexture = createImageryTexture(*parentImagery, tilesetDirectory);
                    auto cacheImageryTexture = processedParentImagery.insert(
                        {*current, std::move(newTexture)});

                    parentTexture = &(cacheImageryTexture.first->second);

                    break;
                }
            } else {
                // found it, we don't need to read the image again. Just use saved name of the saved image
                parentTexture = &it->second;
                break;
            }

            current = CDBTile::createParentTile(*current);
        }

        // we need to re-index UV of the mesh so that it is relative to the parent tile UVs for this case.
        // This step is not necessary for negative LOD since the tile and the parent covers the whole geo cell
        if (parentTexture && cdbTile.getLevel() > 0) {
            elevation.indexUVRelativeToParent(*current);
        }

        if (parentTexture) {
            addElevationToTileset(elevation, parentTexture, tilesetDirectory, *tileset);
        } else {
            addElevationToTileset(elevation, nullptr, tilesetDirectory, *tileset);
        }
    }
}

void CDBTilesetBuilder::addElevationToTileset(CDBElevation &elevation,
                                            const Texture *imagery,
                                            // const CDB &cdb,
                                            const std::filesystem::path &tilesetDirectory,
                                            CDBTileset &tileset)
{
    const auto &mesh = elevation.getUniformGridMesh();
    if (mesh.positionRTCs.empty()) {
        return;
    }

    size_t targetIndexCount = static_cast<size_t>(static_cast<float>(mesh.indices.size())
                                                  * elevationThresholdIndices);
    float targetError = elevationDecimateError;
    Mesh simplifed = elevation.createSimplifiedMesh(targetIndexCount, targetError);
    if (simplifed.positionRTCs.empty()) {
        simplifed = mesh;
    }

    if (elevationNormal) {
        generateElevationNormal(simplifed);
    }

    CDBTile tile = elevation.getTile();
    CDBTile tileWithBoundRegion = CDBTile(tile.getGeoCell(),
                                          tile.getDataset(),
                                          tile.getCS_1(),
                                          tile.getCS_2(),
                                          tile.getLevel(),
                                          tile.getUREF(),
                                          tile.getRREF());
    tileWithBoundRegion.setBoundRegion(
        Core::BoundingRegion(tileWithBoundRegion.getBoundRegion().getRectangle(),
                             elevation.getMinElevation(),
                             elevation.getMaxElevation()));
    elevation.setTile(tileWithBoundRegion);
    auto &cdbTile = elevation.getTile();

    // create material for mesh if there are imagery
    if (imagery) {
        Material material;
        material.doubleSided = true;
        material.unlit = !elevationNormal;
        material.texture = 0;
        simplifed.material = 0;

        tinygltf::Model gltf = createGltf(simplifed, &material, imagery);
        createB3DMForTileset(gltf, cdbTile, nullptr, tilesetDirectory, tileset);
    } else {
        tinygltf::Model gltf = createGltf(simplifed, nullptr, nullptr);
        createB3DMForTileset(gltf, cdbTile, nullptr, tilesetDirectory, tileset);
    }

    if (cdbTile.getLevel() < 0) {
        fillMissingNegativeLODElevation(elevation, tilesetDirectory, tileset);
    } else {
        fillMissingPositiveLODElevation(elevation, imagery, tilesetDirectory, tileset);
    }
}

void CDBTilesetBuilder::fillMissingPositiveLODElevation(const CDBElevation &elevation,
                                                      const Texture *currentImagery,
                                                    //   const CDB &cdb,
                                                      const std::filesystem::path &tilesetDirectory,
                                                      CDBTileset &tileset)
{
    const auto &cdbTile = elevation.getTile();
    auto nw = CDBTile::createNorthWestForPositiveLOD(cdbTile);
    auto ne = CDBTile::createNorthEastForPositiveLOD(cdbTile);
    auto sw = CDBTile::createSouthWestForPositiveLOD(cdbTile);
    auto se = CDBTile::createSouthEastForPositiveLOD(cdbTile);

    // check if elevation exist
    bool isNorthWestExist = cdb->isElevationExist(nw);
    bool isNorthEastExist = cdb->isElevationExist(ne);
    bool isSouthWestExist = cdb->isElevationExist(sw);
    bool isSouthEastExist = cdb->isElevationExist(se);
    bool shouldFillHole = isNorthEastExist || isNorthWestExist || isSouthWestExist || isSouthEastExist;

    // If we don't need to make elevation and imagery have the same LOD, then hasMoreImagery is false.
    // Otherwise, check if imagery exist even the elevation has no child
    bool hasMoreImagery;
    if (elevationLOD) {
        hasMoreImagery = false;
    } else {
        bool isNorthWestImageryExist = cdb->isImageryExist(nw);
        bool isNorthEastImageryExist = cdb->isImageryExist(ne);
        bool isSouthWestImageryExist = cdb->isImageryExist(sw);
        bool isSouthEastImageryExist = cdb->isImageryExist(se);
        hasMoreImagery = isNorthEastImageryExist || isNorthWestImageryExist || isSouthEastImageryExist
                         || isSouthWestImageryExist;
    }

    if (shouldFillHole || hasMoreImagery) {
        if (!isNorthWestExist) {
            auto subRegionImagery = cdb->getImagery(nw);
            bool reindexUV = subRegionImagery != std::nullopt;
            auto subRegion = elevation.createNorthWestSubRegion(reindexUV);
            if (subRegion) {
                addSubRegionElevationToTileset(*subRegion,
                                            //    cdb,
                                               subRegionImagery,
                                               currentImagery,
                                               tilesetDirectory,
                                               tileset);
            }
        }

        if (!isNorthEastExist) {
            auto subRegionImagery = cdb->getImagery(ne);
            bool reindexUV = subRegionImagery != std::nullopt;
            auto subRegion = elevation.createNorthEastSubRegion(reindexUV);
            if (subRegion) {
                addSubRegionElevationToTileset(*subRegion,
                                            //    cdb,
                                               subRegionImagery,
                                               currentImagery,
                                               tilesetDirectory,
                                               tileset);
            }
        }

        if (!isSouthEastExist) {
            auto subRegionImagery = cdb->getImagery(se);
            bool reindexUV = subRegionImagery != std::nullopt;
            auto subRegion = elevation.createSouthEastSubRegion(reindexUV);
            if (subRegion) {
                addSubRegionElevationToTileset(*subRegion,
                                            //    cdb,
                                               subRegionImagery,
                                               currentImagery,
                                               tilesetDirectory,
                                               tileset);
            }
        }

        if (!isSouthWestExist) {
            auto subRegionImagery = cdb->getImagery(sw);
            bool reindexUV = subRegionImagery != std::nullopt;
            auto subRegion = elevation.createSouthWestSubRegion(reindexUV);
            if (subRegion) {
                addSubRegionElevationToTileset(*subRegion,
                                            //    cdb,
                                               subRegionImagery,
                                               currentImagery,
                                               tilesetDirectory,
                                               tileset);
            }
        }
    }
}

void CDBTilesetBuilder::fillMissingNegativeLODElevation(CDBElevation &elevation,
                                                    //   const CDB &cdb,
                                                      const std::filesystem::path &outputDirectory,
                                                      CDBTileset &tileset)
{
    const auto &cdbTile = elevation.getTile();
    auto child = CDBTile::createChildForNegativeLOD(cdbTile);

    // if imagery exist, but we have no more terrain, then duplicate it. However,
    // when we only care about elevation LOD, don't duplicate it
    if (!cdb->isElevationExist(child)) {
        if (!elevationLOD) {
            auto childImagery = cdb->getImagery(child);
            if (childImagery) {
                Texture imageryTexture = createImageryTexture(*childImagery, outputDirectory);
                elevation.setTile(child);
                addElevationToTileset(elevation, &imageryTexture, outputDirectory, tileset);
            }
        }
    }
}

void CDBTilesetBuilder::generateElevationNormal(Mesh &simplifed)
{
    size_t totalVertices = simplifed.positions.size();

    // calculate normals
    const auto &ellipsoid = Core::Ellipsoid::WGS84;
    simplifed.normals.resize(totalVertices, glm::vec3(0.0));
    for (size_t i = 0; i < simplifed.indices.size(); i += 3) {
        uint32_t idx0 = simplifed.indices[i];
        uint32_t idx1 = simplifed.indices[i + 1];
        uint32_t idx2 = simplifed.indices[i + 2];

        glm::dvec3 p0 = simplifed.positionRTCs[idx0];
        glm::dvec3 p1 = simplifed.positionRTCs[idx1];
        glm::dvec3 p2 = simplifed.positionRTCs[idx2];

        glm::vec3 normal = glm::cross(p1 - p0, p2 - p0);
        simplifed.normals[idx0] += normal;
        simplifed.normals[idx1] += normal;
        simplifed.normals[idx2] += normal;
    }

    // normalize normal and calculate position rtc
    for (size_t i = 0; i < totalVertices; ++i) {
        auto &normal = simplifed.normals[i];
        if (glm::abs(glm::dot(normal, normal)) > Core::Math::EPSILON10) {
            normal = glm::normalize(normal);
        } else {
            auto cartographic = ellipsoid.cartesianToCartographic(simplifed.positions[i]);
            if (cartographic) {
                normal = ellipsoid.geodeticSurfaceNormal(*cartographic);
            }
        }
    }
}

void CDBTilesetBuilder::addSubRegionElevationToTileset(CDBElevation &subRegion,
                                                //    const CDB &cdb,
                                                   std::optional<CDBImagery> &subRegionImagery,
                                                   const Texture *parentTexture,
                                                   const std::filesystem::path &outputDirectory,
                                                   CDBTileset &tileset)
{
    // Use the sub region imagery. If sub region doesn't have imagery, reuse parent imagery if we don't have any higher LOD imagery
    if (subRegionImagery) {
        Texture subRegionTexture = createImageryTexture(*subRegionImagery, outputDirectory);
        addElevationToTileset(subRegion, &subRegionTexture, outputDirectory, tileset);
    } else if (parentTexture) {
        addElevationToTileset(subRegion, parentTexture, outputDirectory, tileset);
    } else {
        addElevationToTileset(subRegion, nullptr, outputDirectory, tileset);
    }
}

Texture CDBTilesetBuilder::createImageryTexture(CDBImagery &imagery,
                                              const std::filesystem::path &tilesetOutputDirectory) const
{
    static const std::filesystem::path MODEL_TEXTURE_SUB_DIR = "Textures";

    const auto &tile = imagery.getTile();
    auto textureRelativePath = MODEL_TEXTURE_SUB_DIR / (tile.getRelativePath().filename().string() + ".jpeg");
    auto textureAbsolutePath = tilesetOutputDirectory / textureRelativePath;
    auto textureDirectory = tilesetOutputDirectory / MODEL_TEXTURE_SUB_DIR;
    if (!std::filesystem::exists(textureDirectory)) {
        std::filesystem::create_directories(textureDirectory);
    }

    auto driver = (GDALDriver *) GDALGetDriverByName("jpeg");
    if (driver) {
        GDALDatasetUniquePtr jpegDataset = GDALDatasetUniquePtr(driver->CreateCopy(
            textureAbsolutePath.string().c_str(), &imagery.getData(), false, nullptr, nullptr, nullptr));
    }

    Texture texture;
    texture.uri = textureRelativePath;
    texture.magFilter = TextureFilter::LINEAR;
    texture.minFilter = TextureFilter::LINEAR_MIPMAP_NEAREST;

    return texture;
}

void CDBTilesetBuilder::addVectorToTilesetCollection(
    const CDBGeometryVectors &vectors,
    const std::filesystem::path &collectionOutputDirectory,
    std::unordered_map<CDBGeoCell, TilesetCollection> &tilesetCollections)
{
    const auto &cdbTile = vectors.getTile();
    const auto &mesh = vectors.getMesh();
    if (mesh.positionRTCs.empty()) {
        return;
    }

    std::filesystem::path tilesetDirectory;
    CDBTileset *tileset;
    getTileset(cdbTile, collectionOutputDirectory, tilesetCollections, tileset, tilesetDirectory);

    tinygltf::Model gltf = createGltf(mesh, nullptr, nullptr);
    createB3DMForTileset(gltf, cdbTile, &vectors.getInstancesAttributes(), tilesetDirectory, *tileset);
}

void CDBTilesetBuilder::addGTModelToTilesetCollection(const CDBGTModels &model,
                                                    const std::filesystem::path &collectionOutputDirectory)
{
    static const std::filesystem::path MODEL_GLTF_SUB_DIR = "Gltf";
    static const std::filesystem::path MODEL_TEXTURE_SUB_DIR = "Textures";

    auto cdbTile = model.getModelsAttributes().getTile();

    std::filesystem::path tilesetDirectory;
    CDBTileset *tileset;
    getTileset(cdbTile, collectionOutputDirectory, GTModelTilesets, tileset, tilesetDirectory);

    // create gltf file
    auto gltfOutputDIr = tilesetDirectory / MODEL_GLTF_SUB_DIR;
    std::filesystem::create_directories(gltfOutputDIr);

    std::map<std::string, std::vector<int>> instances;
    const auto &modelsAttribs = model.getModelsAttributes();
    const auto &instancesAttribs = modelsAttribs.getInstancesAttributes();
    for (size_t i = 0; i < instancesAttribs.getInstancesCount(); ++i) {
        std::string modelKey;
        auto model3D = model.locateModel3D(i, modelKey);
        if (model3D) {
            if (GTModelsToGltf.find(modelKey) == GTModelsToGltf.end()) {
                // write textures to files
                auto textures = writeModeTextures(model3D->getTextures(),
                                                  model3D->getImages(),
                                                  MODEL_TEXTURE_SUB_DIR,
                                                  gltfOutputDIr);

                // create gltf for the instance
                tinygltf::Model gltf = createGltf(model3D->getMeshes(), model3D->getMaterials(), textures);

                // write to glb
                tinygltf::TinyGLTF loader;
                std::filesystem::path modelGltfURI = MODEL_GLTF_SUB_DIR / (modelKey + ".glb");
                loader.WriteGltfSceneToFile(&gltf, tilesetDirectory / modelGltfURI, false, false, false, true);
                GTModelsToGltf.insert({modelKey, modelGltfURI});
            }

            auto &instance = instances[modelKey];
            instance.emplace_back(i);
        }
    }

    // write i3dm to cmpt
    std::string cdbTileFilename = cdbTile.getRelativePathWithNonZeroPaddedLevel().filename().string();
    std::filesystem::path cmpt = cdbTileFilename + std::string(".cmpt");
    std::filesystem::path cmptFullPath = tilesetDirectory / cmpt;
    std::ofstream fs(cmptFullPath, std::ios::binary);
    auto instance = instances.begin();
    writeToCMPT(static_cast<uint32_t>(instances.size()), fs, [&](std::ofstream &os, size_t) {
        const auto &GltfURI = GTModelsToGltf[instance->first];
        const auto &instanceIndices = instance->second;
        size_t totalWrite = writeToI3DM(GltfURI, modelsAttribs, instanceIndices, os);
        instance = std::next(instance);
        return totalWrite;
    });

    // add it to tileset
    cdbTile.setCustomContentURI(cmpt);
    if(use3dTilesNext && cdbTile.getLevel() >= 0)
        addAvailability(cdbTile);
    tileset->insertTile(cdbTile);
}

void CDBTilesetBuilder::addGSModelToTilesetCollection(const CDBGSModels &model,
                                                    const std::filesystem::path &collectionOutputDirectory)
{
    static const std::filesystem::path MODEL_TEXTURE_SUB_DIR = "Textures";

    const auto &cdbTile = model.getTile();
    const auto &model3D = model.getModel3D();

    std::filesystem::path tilesetDirectory;
    CDBTileset *tileset;
    getTileset(cdbTile, collectionOutputDirectory, GSModelTilesets, tileset, tilesetDirectory);

    auto textures = writeModeTextures(model3D.getTextures(),
                                      model3D.getImages(),
                                      MODEL_TEXTURE_SUB_DIR,
                                      tilesetDirectory);

    auto gltf = createGltf(model3D.getMeshes(), model3D.getMaterials(), textures);
    createB3DMForTileset(gltf, cdbTile, &model.getInstancesAttributes(), tilesetDirectory, *tileset);
}

std::vector<Texture> CDBTilesetBuilder::writeModeTextures(const std::vector<Texture> &modelTextures,
                                                        const std::vector<osg::ref_ptr<osg::Image>> &images,
                                                        const std::filesystem::path &textureSubDir,
                                                        const std::filesystem::path &gltfPath)
{
    auto textureDirectory = gltfPath / textureSubDir;
    if (!std::filesystem::exists(textureDirectory)) {
        std::filesystem::create_directories(textureDirectory);
    }

    auto textures = modelTextures;
    for (size_t i = 0; i < modelTextures.size(); ++i) {
        auto textureRelativePath = textureSubDir / modelTextures[i].uri;
        auto textureAbsolutePath = gltfPath / textureSubDir / modelTextures[i].uri;

        if (processedModelTextures.find(textureAbsolutePath) == processedModelTextures.end()) {
            osgDB::writeImageFile(*images[i], textureAbsolutePath.string(), nullptr);
        }

        textures[i].uri = textureRelativePath.string();
    }

    return textures;
}

void CDBTilesetBuilder::createB3DMForTileset(tinygltf::Model &gltf,
                                          CDBTile cdbTile,
                                          const CDBInstancesAttributes *instancesAttribs,
                                          const std::filesystem::path &outputDirectory,
                                          CDBTileset &tileset)
{
    // create b3dm file
    std::string cdbTileFilename = cdbTile.getRelativePathWithNonZeroPaddedLevel().filename().string();
    std::filesystem::path b3dm = cdbTileFilename + std::string(".b3dm");
    std::filesystem::path b3dmFullPath = outputDirectory / b3dm;

    // write to b3dm
    std::ofstream fs(b3dmFullPath, std::ios::binary);
    writeToB3DM(&gltf, instancesAttribs, fs);
    cdbTile.setCustomContentURI(b3dm);

    if(use3dTilesNext) 
    {
        if(cdbTile.getLevel() >= 0)
            addAvailability(cdbTile);
        if(cdbTile.getLevel() > 0) // don't add tiles above level 0, which are implicitly defined
            return;
    }
    tileset.insertTile(cdbTile);
}

size_t CDBTilesetBuilder::hashComponentSelectors(int CS_1, int CS_2)
{
    size_t CSHash = 0;
    hashCombine(CSHash, CS_1);
    hashCombine(CSHash, CS_2);
    return CSHash;
}

std::filesystem::path CDBTilesetBuilder::getTilesetDirectory(
    int CS_1, int CS_2, const std::filesystem::path &collectionOutputDirectory)
{
    return collectionOutputDirectory / (std::to_string(CS_1) + "_" + std::to_string(CS_2));
}

void CDBTilesetBuilder::getTileset(const CDBTile &cdbTile,
                                const std::filesystem::path &collectionOutputDirectory,
                                std::unordered_map<CDBGeoCell, TilesetCollection> &tilesetCollections,
                                CDBTileset *&tileset,
                                std::filesystem::path &path)
{
    const auto &geoCell = cdbTile.getGeoCell();
    auto &tilesetCollection = tilesetCollections[geoCell];

    // find output directory
    size_t CSHash = hashComponentSelectors(cdbTile.getCS_1(), cdbTile.getCS_2());

    auto &CSToPaths = tilesetCollection.CSToPaths;
    auto CSPathIt = CSToPaths.find(CSHash);
    if (CSPathIt == CSToPaths.end()) {
        path = getTilesetDirectory(cdbTile.getCS_1(), cdbTile.getCS_2(), collectionOutputDirectory);
        std::filesystem::create_directories(path);
        CSToPaths.insert({CSHash, path});
    } else {
        path = CSPathIt->second;
    }

    tileset = &tilesetCollection.CSToTilesets[CSHash];
}
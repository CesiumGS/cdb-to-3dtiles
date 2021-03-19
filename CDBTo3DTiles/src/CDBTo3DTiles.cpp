#include "CDBTo3DTiles.h"
#include "CDB.h"
#include "FileUtil.h"
#include "Gltf.h"
#include "Math.h"
#include "TileFormatIO.h"
#include "cpl_conv.h"
#include "gdal.h"
#include "osgDB/WriteFile"
#include <cmath>
#include <limits>
#include <morton.h>
#include <nlohmann/json.hpp>
#include <unordered_map>
#include <unordered_set>
using json = nlohmann::json;

namespace CDBTo3DTiles {

Converter::Converter(const std::filesystem::path &CDBPath, const std::filesystem::path &outputPath)
{
    m_impl = std::make_unique<CDBTilesetBuilder>(CDBPath, outputPath);
}

Converter::~Converter() noexcept {}

void Converter::combineDataset(const std::vector<std::string> &datasets)
{
    // Only combine when we have more than 1 tileset. Less than that, it means
    // the tileset doesn't exist (no action needed here) or
    // it is already combined from different geocell by default
    if (datasets.size() == 1) {
        return;
    }

    m_impl->requestedDatasetToCombine.emplace_back(datasets);
    for (const auto &dataset : datasets) {
        auto datasetNamePos = dataset.find("_");
        if (datasetNamePos == std::string::npos) {
            throw std::runtime_error("Wrong format. Required format should be: {DatasetName}_{Component "
                                     "Selector 1}_{Component Selector 2}");
        }

        auto datasetName = dataset.substr(0, datasetNamePos);
        if (m_impl->DATASET_PATHS.find(datasetName) == m_impl->DATASET_PATHS.end()) {
            std::string errorMessage = "Unrecognize dataset: " + datasetName + "\n";
            errorMessage += "Correct dataset names are: \n";
            for (const auto &requiredDataset : m_impl->DATASET_PATHS) {
                errorMessage += requiredDataset + "\n";
            }

            throw std::runtime_error(errorMessage);
        }

        auto CS_1Pos = dataset.find("_", datasetNamePos + 1);
        if (CS_1Pos == std::string::npos) {
            throw std::runtime_error("Wrong format. Required format should be: {DatasetName}_{Component "
                                     "Selector 1}_{Component Selector 2}");
        }

        auto CS_1 = dataset.substr(datasetNamePos + 1, CS_1Pos - datasetNamePos - 1);
        if (CS_1.empty() || !std::all_of(CS_1.begin(), CS_1.end(), ::isdigit)) {
            throw std::runtime_error("Component selector 1 has to be a number");
        }

        auto CS_2 = dataset.substr(CS_1Pos + 1);
        if (CS_2.empty() || !std::all_of(CS_2.begin(), CS_2.end(), ::isdigit)) {
            throw std::runtime_error("Component selector 2 has to be a number");
        }
    }
}

void Converter::setUse3dTilesNext(bool use3dTilesNext)
{
    m_impl->use3dTilesNext = use3dTilesNext;
}

void Converter::setGenerateElevationNormal(bool elevationNormal)
{
    m_impl->elevationNormal = elevationNormal;
}

void Converter::setElevationLODOnly(bool elevationLOD)
{
    m_impl->elevationLOD = elevationLOD;
}

void Converter::setSubtreeLevels(int subtreeLevels)
{
    m_impl->subtreeLevels = subtreeLevels;
}

void Converter::setElevationThresholdIndices(float elevationThresholdIndices)
{
    m_impl->elevationThresholdIndices = elevationThresholdIndices;
}

void Converter::setElevationDecimateError(float elevationDecimateError)
{
    m_impl->elevationDecimateError = elevationDecimateError;
}

void Converter::convert()
{
    CDB cdb(m_impl->cdbPath);
    m_impl->cdb = &cdb;
    std::map<std::string, std::vector<std::filesystem::path>> combinedTilesets;
    std::map<std::string, std::vector<Core::BoundingRegion>> combinedTilesetsRegions;
    std::map<std::string, Core::BoundingRegion> aggregateTilesetsRegion;

    if (m_impl->use3dTilesNext) {
        int subtreeLevels = m_impl->subtreeLevels;
        const uint64_t subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels) - 1) / 3);
        const uint64_t childSubtreeCount = static_cast<int>(pow(4, subtreeLevels)); // 4^N

        const uint64_t availabilityByteLength = static_cast<int>(
            ceil(static_cast<double>(subtreeNodeCount) / 8.0));
        const uint64_t nodeAvailabilityByteLengthWithPadding = alignTo8(availabilityByteLength);
        const uint64_t childSubtreeAvailabilityByteLength = static_cast<int>(
            ceil(static_cast<double>(childSubtreeCount) / 8.0));
        const uint64_t childSubtreeAvailabilityByteLengthWithPadding = alignTo8(
            childSubtreeAvailabilityByteLength);
        m_impl->nodeAvailabilityByteLengthWithPadding = nodeAvailabilityByteLengthWithPadding;
        m_impl->childSubtreeAvailabilityByteLengthWithPadding = childSubtreeAvailabilityByteLengthWithPadding;

        const uint64_t headerByteLength = 24;

        // Key is CDBDataset. Value is subtree map, which is key: level_x_y of subtree root, value: subtreeAvailability struct (buffers and avail count for both nodes and child subtrees)
        std::map<CDBDataset, std::map<std::string, subtreeAvailability>> &datasetSubtrees = m_impl->datasetSubtrees;

        std::vector<Core::BoundingRegion> boundingRegions;
        std::vector<std::filesystem::path> tilesetJsonPaths;
        cdb.forEachGeoCell([&](CDBGeoCell geoCell) {
            datasetSubtrees.clear();

            std::filesystem::path geoCellRelativePath = geoCell.getRelativePath();
            std::filesystem::path geoCellAbsolutePath = m_impl->outputPath / geoCellRelativePath;
            std::filesystem::path elevationDir = geoCellAbsolutePath / CDBTilesetBuilder::ELEVATIONS_PATH;
            std::filesystem::path GSModelDir = geoCellAbsolutePath / CDBTilesetBuilder::GSMODEL_PATH;
            std::filesystem::path GTModelDir = geoCellAbsolutePath / CDBTilesetBuilder::GTMODEL_PATH;
            std::filesystem::path roadNetworkDir = geoCellAbsolutePath / CDBTilesetBuilder::ROAD_NETWORK_PATH;
            std::map<CDBDataset, std::filesystem::path> datasetDirs;
            datasetDirs.insert(std::pair<CDBDataset, std::filesystem::path>(CDBDataset::Elevation, elevationDir));
            datasetDirs.insert(std::pair<CDBDataset, std::filesystem::path>(CDBDataset::GSFeature, GSModelDir));
            datasetDirs.insert(std::pair<CDBDataset, std::filesystem::path>(CDBDataset::GTFeature, GTModelDir));
            datasetDirs.insert(std::pair<CDBDataset, std::filesystem::path>(CDBDataset::RoadNetwork, roadNetworkDir));

            // TODO make max level depend on dataset group, component selector
            m_impl->maxLevel = INT_MIN;

            // TODO make addAvailability add when the b3dm/cmpt/glb is written to get true availability vs
            //   availability of input data
            // TODO check bounding region for elevation written to tileset
            // Elevation
            cdb.forEachElevationTile(geoCell, [&](CDBElevation elevation) {
                // m_impl->addAvailability(
                //                         // CDBDataset::Elevation,
                //                         datasetSubtrees,
                //                         elevation.getTile());
                m_impl->addElevationToTilesetCollection(elevation, elevationDir);
            });
            std::unordered_map<CDBTile, Texture>().swap(m_impl->processedParentImagery);

            // GSModels
            cdb.forEachGSModelTile(geoCell, [&](CDBGSModels GSModel) {
                m_impl->addAvailability(
                                        // CDBDataset::GSFeature,
                                        // datasetSubtrees,
                                        GSModel.getTile());
                m_impl->addGSModelToTilesetCollection(GSModel, GSModelDir);
            });

            // GTModels
            // TODO check what this is doing for various component selectors (1_1 vs 2_1)
            cdb.forEachGTModelTile(geoCell, [&](CDBGTModels GTModel) {
                m_impl->addAvailability(
                                        // CDBDataset::GTFeature,
                                        // datasetSubtrees,
                                        GTModel.getModelsAttributes().getTile());
                m_impl->addGTModelToTilesetCollection(GTModel, GTModelDir);
            });

            // cdb.forEachRoadNetworkTile(geoCell, [&](const CDBGeometryVectors &roadNetwork) {
            //     m_impl->addAvailability(cdb,
            //                             CDBDataset::RoadNetwork,
            //                             datasetSubtrees,
            //                             roadNetwork.getTile());
            //     m_impl->addVectorToTilesetCollection(roadNetwork, roadNetworkDir, m_impl->roadNetworkTilesets);
            // });

            if(m_impl->maxLevel == INT_MIN) // no content tiles
                return;
            m_impl->flushTilesetCollectionsMultiContent(geoCell);
            std::set<std::string> subtreeRoots;
            std::map<std::string, subtreeAvailability> &tileAndChildAvailabilities = m_impl->tileAndChildAvailabilities;

            // write all of the availability buffers and subtree files for each dataset group
            for(auto & [groupName, group] : m_impl->datasetGroups)
            {
                for (CDBDataset dataset : group.datasets) {
                    if (datasetSubtrees.count(dataset) == 0)
                    {
                        continue;
                    }
                    for (auto &[key, subtree] : datasetSubtrees.at(dataset)) {
                        subtreeRoots.insert(key);

                        subtreeAvailability *tileAndChildAvailability;
                        if(tileAndChildAvailabilities.count(key) == 0)
                        {
                            tileAndChildAvailabilities.insert(std::pair<std::string, subtreeAvailability>(key, m_impl->createSubtreeAvailability()));
                        }
                        tileAndChildAvailability = &tileAndChildAvailabilities.at(key);
                        for(uint64_t index = 0 ; index < availabilityByteLength ; index += 1)
                        {
                            tileAndChildAvailability->nodeBuffer.at(index) = static_cast<uint8_t>(tileAndChildAvailability->nodeBuffer.at(index) | subtree.nodeBuffer.at(index));
                        }
                        for(uint64_t index = 0 ; index < childSubtreeAvailabilityByteLength ; index += 1)
                        {
                            tileAndChildAvailability->childBuffer.at(index) = static_cast<uint8_t>(tileAndChildAvailability->childBuffer.at(index) | subtree.childBuffer.at(index));
                        }

                        bool constantNodeAvailability = (subtree.nodeCount == 0)
                                                        || (subtree.nodeCount == subtreeNodeCount);

                        if(constantNodeAvailability)
                        {
                            continue;
                        }

                        std::vector<uint8_t> outputBuffer(nodeAvailabilityByteLengthWithPadding);
                        uint8_t* outBuffer = &outputBuffer[0];
                        memset(&outBuffer[0], 0, nodeAvailabilityByteLengthWithPadding);
                        memcpy(&outBuffer[0], &subtree.nodeBuffer[0], nodeAvailabilityByteLengthWithPadding);
                        std::filesystem::path path = datasetDirs.at(dataset) / "availability" / (key + ".bin");
                        Utilities::writeBinaryFile(path, (const char *)&outBuffer[0], nodeAvailabilityByteLengthWithPadding);
                    }
                }

                // write .subtree files for every subtree that we had to make a buffer for
                for(std::string subtreeRoot: subtreeRoots)
                {
                    json subtreeJson;

                    nlohmann::json buffers = nlohmann::json::array();
                    int bufferIndex = 0;
                    nlohmann::json bufferViews = nlohmann::json::array();
                    subtreeAvailability tileAndChildAvailability = tileAndChildAvailabilities.at(subtreeRoot);
                    tileAndChildAvailability.nodeCount = countSetBitsInVectorOfInts(tileAndChildAvailability.nodeBuffer);
                    tileAndChildAvailability.childCount = countSetBitsInVectorOfInts(tileAndChildAvailability.childBuffer);
                    bool constantTileAvailability = (tileAndChildAvailability.nodeCount == 0) ||
                                                    (tileAndChildAvailability.nodeCount == subtreeNodeCount);
                    bool constantChildAvailability = (tileAndChildAvailability.childCount == 0) ||
                                                    (tileAndChildAvailability.childCount == childSubtreeCount);

                    uint64_t nodeBufferLengthToWrite = static_cast<int>(!constantTileAvailability)
                                                        * nodeAvailabilityByteLengthWithPadding;
                    uint64_t childBufferLengthToWrite = static_cast<int>(!constantChildAvailability)
                                                        * childSubtreeAvailabilityByteLengthWithPadding;
                    long unsigned int bufferByteLength = nodeBufferLengthToWrite + childBufferLengthToWrite;
                    if(bufferByteLength != 0)
                    {
                        nlohmann::json byteLength;
                        byteLength["byteLength"] = bufferByteLength;
                        buffers.emplace_back(byteLength);
                        bufferIndex += 1;
                    }
                    
                    std::vector<uint8_t> internalBuffer(bufferByteLength);
                    memset(&internalBuffer[0], 0, bufferByteLength);
                    uint8_t* outInternalBuffer = &internalBuffer[0];
                    nlohmann::json tileAvailabilityJson;
                    int bufferViewIndex = 0;
                    uint64_t internalBufferOffset = 0;
                    if (constantTileAvailability)
                        tileAvailabilityJson["constant"] = static_cast<int>(tileAndChildAvailability.nodeCount == subtreeNodeCount);
                    else
                    {
                        memcpy(&outInternalBuffer[0], &tileAndChildAvailability.nodeBuffer[0], nodeAvailabilityByteLengthWithPadding);
                        nlohmann::json bufferViewObj;
                        bufferViewObj["buffer"] = 0;
                        bufferViewObj["byteOffset"] = 0;
                        bufferViewObj["byteLength"] = availabilityByteLength;
                        bufferViews.emplace_back(bufferViewObj);
                        internalBufferOffset += nodeAvailabilityByteLengthWithPadding;
                        tileAvailabilityJson["bufferView"] = bufferViewIndex;
                        bufferViewIndex += 1;
                    }
                    subtreeJson["tileAvailability"] = tileAvailabilityJson;

                    nlohmann::json childAvailabilityJson;
                    if (constantChildAvailability)
                        childAvailabilityJson["constant"] = static_cast<int>(tileAndChildAvailability.childCount == childSubtreeCount);
                    else
                    {
                        memcpy(&outInternalBuffer[internalBufferOffset], &tileAndChildAvailability.childBuffer[0], childSubtreeAvailabilityByteLengthWithPadding);
                        nlohmann::json bufferViewObj;
                        bufferViewObj["buffer"] = 0;
                        bufferViewObj["byteOffset"] = internalBufferOffset;
                        bufferViewObj["byteLength"] = childSubtreeAvailabilityByteLength;
                        bufferViews.emplace_back(bufferViewObj);
                        childAvailabilityJson["bufferView"] = bufferViewIndex;
                        bufferViewIndex += 1;
                    }
                    subtreeJson["childSubtreeAvailability"] = childAvailabilityJson;
                    
                    nlohmann::json contentAvailability = nlohmann::json::array();
                    std::string availabilityFileName = subtreeRoot + ".bin";
                    for(CDBDataset dataset : group.datasets)
                    {
                        std::filesystem::path datasetDir = datasetDirs.at(dataset);
                        nlohmann::json contentObj;
                        if(std::filesystem::exists(datasetDir / "availability" / availabilityFileName))
                        {
                            nlohmann::json bufferObj;
                            auto datasetDirIt = datasetDir.end();
                            --datasetDirIt; // point to the dataset directory name
                            bufferObj["uri"] = "../.." /(*datasetDirIt) / "availability" / availabilityFileName;
                            bufferObj["byteLength"] = nodeAvailabilityByteLengthWithPadding;
                            buffers.emplace_back(bufferObj);
                            nlohmann::json bufferViewObj;
                            bufferViewObj["buffer"] = bufferIndex;
                            bufferViewObj["byteOffset"] = 0;
                            bufferViewObj["byteLength"] = availabilityByteLength;
                            bufferViews.emplace_back(bufferViewObj);
                            contentObj["bufferView"] = bufferViewIndex;
                            bufferViewIndex += 1;
                            bufferIndex += 1;
                        }
                        else if((datasetSubtrees.count(dataset) != 0) && datasetSubtrees.at(dataset).count(subtreeRoot) != 0)
                        {
                            subtreeAvailability subtree = datasetSubtrees.at(dataset).at(subtreeRoot);
                            contentObj["constant"] = static_cast<int>(subtree.nodeCount == subtreeNodeCount);
                        }
                        if(!contentObj.empty() && contentObj != NULL)
                            contentAvailability.emplace_back(contentObj);
                    }
                    nlohmann::json extensions;
                    nlohmann::json multiContent;
                    multiContent["contentAvailability"] = contentAvailability;
                    extensions["3DTILES_multiple_contents"] = multiContent;
                    subtreeJson["extensions"] = extensions;
                    if (!buffers.empty())
                        subtreeJson["buffers"] = buffers;
                    if (!bufferViews.empty())
                        subtreeJson["bufferViews"] = bufferViews;

                    // get json length
                    const std::string jsonString = subtreeJson.dump();
                    const uint64_t jsonStringByteLength = jsonString.size();
                    const uint64_t jsonStringByteLengthWithPadding = alignTo8(jsonStringByteLength);

                    // Write subtree binary
                    uint64_t outputBufferLength = jsonStringByteLengthWithPadding + bufferByteLength + headerByteLength;
                    std::vector<uint8_t> outputBuffer(outputBufferLength);
                    uint8_t* outBuffer = &outputBuffer[0];
                    *(uint32_t*)&outBuffer[0] = 0x74627573; // magic: "subt"
                    *(uint32_t*)&outBuffer[4] = 1; // version
                    *(uint64_t*)&outBuffer[8] = jsonStringByteLengthWithPadding; // JSON byte length with padding
                    *(uint64_t*)&outBuffer[16] = bufferByteLength; // BIN byte length with padding

                    memcpy(&outBuffer[headerByteLength], &jsonString[0], jsonStringByteLength);
                    memset(&outBuffer[headerByteLength + jsonStringByteLength], ' ', jsonStringByteLengthWithPadding - jsonStringByteLength);

                    if(bufferByteLength != 0)
                    {
                        memcpy(&outBuffer[headerByteLength + jsonStringByteLengthWithPadding], outInternalBuffer, bufferByteLength);
                    }
                    std::filesystem::path path = geoCellAbsolutePath / "subtrees" / groupName / (subtreeRoot + ".subtree");
                    Utilities::writeBinaryFile(path , (const char*)outBuffer, outputBufferLength);
                }
                tileAndChildAvailabilities.clear();
                subtreeRoots.clear();
            }

            // get the converted dataset in each geocell to be combine at the end
            Core::BoundingRegion geoCellRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
            for(auto &[groupName, group] : m_impl->datasetGroups)
            {
                for (auto tilesetJsonPath : group.tilesetsToCombine) {
                    auto combinedTilesetName = groupName;

                    combinedTilesets[combinedTilesetName].emplace_back(tilesetJsonPath);
                    combinedTilesetsRegions[combinedTilesetName].emplace_back(geoCellRegion);
                    auto tilesetAggregateRegion = aggregateTilesetsRegion.find(combinedTilesetName);
                    if (tilesetAggregateRegion == aggregateTilesetsRegion.end()) {
                        aggregateTilesetsRegion.insert({combinedTilesetName, geoCellRegion});
                    } else {
                        tilesetAggregateRegion->second = tilesetAggregateRegion->second.computeUnion(
                            geoCellRegion);
                    }
                }
                group.tilesetsToCombine.clear();
            }
        });

        for (auto const &[tilesetName, tileset] : combinedTilesets) {
            std::filesystem::path datasetGroupJsonPath = m_impl->outputPath / (tilesetName + ".json");
            std::ofstream fs(datasetGroupJsonPath);
            combineTilesetJson(tileset, combinedTilesetsRegions[tilesetName], fs);
            tilesetJsonPaths.emplace_back(tilesetName + ".json");
            boundingRegions.emplace_back(aggregateTilesetsRegion.at(tilesetName));
        }

        std::ofstream fs(m_impl->outputPath / "tileset.json");
        combineTilesetJson(tilesetJsonPaths, boundingRegions, fs);
    } 
    else {
        cdb.forEachGeoCell([&](CDBGeoCell geoCell) {
            // create directories for converted GeoCell
            std::filesystem::path geoCellRelativePath = geoCell.getRelativePath();
            std::filesystem::path geoCellAbsolutePath = m_impl->outputPath / geoCellRelativePath;
            std::filesystem::path elevationDir = geoCellAbsolutePath / CDBTilesetBuilder::ELEVATIONS_PATH;
            std::filesystem::path GTModelDir = geoCellAbsolutePath / CDBTilesetBuilder::GTMODEL_PATH;
            std::filesystem::path GSModelDir = geoCellAbsolutePath / CDBTilesetBuilder::GSMODEL_PATH;
            std::filesystem::path roadNetworkDir = geoCellAbsolutePath / CDBTilesetBuilder::ROAD_NETWORK_PATH;
            std::filesystem::path railRoadNetworkDir = geoCellAbsolutePath
                                                       / CDBTilesetBuilder::RAILROAD_NETWORK_PATH;
            std::filesystem::path powerlineNetworkDir = geoCellAbsolutePath
                                                        / CDBTilesetBuilder::POWERLINE_NETWORK_PATH;
            std::filesystem::path hydrographyNetworkDir = geoCellAbsolutePath
                                                          / CDBTilesetBuilder::HYDROGRAPHY_NETWORK_PATH;

            // process elevation
            cdb.forEachElevationTile(geoCell, [&](CDBElevation elevation) {
                m_impl->addElevationToTilesetCollection(elevation, elevationDir);
            });
            m_impl->flushTilesetCollection(geoCell, m_impl->elevationTilesets);
            std::unordered_map<CDBTile, Texture>().swap(m_impl->processedParentImagery);

            // process road network
            cdb.forEachRoadNetworkTile(geoCell, [&](const CDBGeometryVectors &roadNetwork) {
                m_impl->addVectorToTilesetCollection(roadNetwork, roadNetworkDir, m_impl->roadNetworkTilesets);
            });
            m_impl->flushTilesetCollection(geoCell, m_impl->roadNetworkTilesets);

            // process railroad network
            cdb.forEachRailRoadNetworkTile(geoCell, [&](const CDBGeometryVectors &railRoadNetwork) {
                m_impl->addVectorToTilesetCollection(railRoadNetwork,
                                                     railRoadNetworkDir,
                                                     m_impl->railRoadNetworkTilesets);
            });
            m_impl->flushTilesetCollection(geoCell, m_impl->railRoadNetworkTilesets);

            // process powerline network
            cdb.forEachPowerlineNetworkTile(geoCell, [&](const CDBGeometryVectors &powerlineNetwork) {
                m_impl->addVectorToTilesetCollection(powerlineNetwork,
                                                     powerlineNetworkDir,
                                                     m_impl->powerlineNetworkTilesets);
            });
            m_impl->flushTilesetCollection(geoCell, m_impl->powerlineNetworkTilesets);

            // process hydrography network
            cdb.forEachHydrographyNetworkTile(geoCell, [&](const CDBGeometryVectors &hydrographyNetwork) {
                m_impl->addVectorToTilesetCollection(hydrographyNetwork,
                                                     hydrographyNetworkDir,
                                                     m_impl->hydrographyNetworkTilesets);
            });
            m_impl->flushTilesetCollection(geoCell, m_impl->hydrographyNetworkTilesets);

            // process GTModel
            // TODO: Remove this workaround when EXT_mesh_gpu_instancing is implemented
            if (!m_impl->use3dTilesNext) {
                cdb.forEachGTModelTile(geoCell, [&](CDBGTModels GTModel) {
                    m_impl->addGTModelToTilesetCollection(GTModel, GTModelDir);
                });
                m_impl->flushTilesetCollection(geoCell, m_impl->GTModelTilesets);
            }

            // process GSModel
            cdb.forEachGSModelTile(geoCell, [&](CDBGSModels GSModel) {
                m_impl->addGSModelToTilesetCollection(GSModel, GSModelDir);
            });
            m_impl->flushTilesetCollection(geoCell, m_impl->GSModelTilesets, false);

            // get the converted dataset in each geocell to be combine at the end
            Core::BoundingRegion geoCellRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
            for (auto tilesetJsonPath : m_impl->defaultDatasetToCombine) {
                auto componentSelectors = tilesetJsonPath.parent_path().filename().string();
                auto dataset = tilesetJsonPath.parent_path().parent_path().filename().string();
                auto combinedTilesetName = dataset + "_" + componentSelectors;

                combinedTilesets[combinedTilesetName].emplace_back(tilesetJsonPath);
                combinedTilesetsRegions[combinedTilesetName].emplace_back(geoCellRegion);
                auto tilesetAggregateRegion = aggregateTilesetsRegion.find(combinedTilesetName);
                if (tilesetAggregateRegion == aggregateTilesetsRegion.end()) {
                    aggregateTilesetsRegion.insert({combinedTilesetName, geoCellRegion});
                } else {
                    tilesetAggregateRegion->second = tilesetAggregateRegion->second.computeUnion(
                        geoCellRegion);
                }
            }
            std::vector<std::filesystem::path>().swap(m_impl->defaultDatasetToCombine);
        });

        // combine all the default tileset in each geocell into a global one
        for (auto tileset : combinedTilesets) {
            std::ofstream fs(m_impl->outputPath / (tileset.first + ".json"));
            combineTilesetJson(tileset.second, combinedTilesetsRegions[tileset.first], fs);
        }

        // combine the requested tilesets
        for (const auto &tilesets : m_impl->requestedDatasetToCombine) {
            std::string combinedTilesetName;
            if (m_impl->requestedDatasetToCombine.size() > 1) {
                for (const auto &tileset : tilesets) {
                    combinedTilesetName += tileset;
                }
                combinedTilesetName += ".json";
            } else {
                combinedTilesetName = "tileset.json";
            }

            std::vector<std::filesystem::path> existTilesets;
            std::vector<Core::BoundingRegion> regions;
            regions.reserve(tilesets.size());
            for (const auto &tileset : tilesets) {
                auto tilesetRegion = aggregateTilesetsRegion.find(tileset);
                if (tilesetRegion != aggregateTilesetsRegion.end()) {
                    existTilesets.emplace_back(tilesetRegion->first + ".json");
                    regions.emplace_back(tilesetRegion->second);
                }
            }

            std::ofstream fs(m_impl->outputPath / combinedTilesetName);
            combineTilesetJson(existTilesets, regions, fs);
        }
    }
}

USE_OSGPLUGIN(png)
USE_OSGPLUGIN(jpeg)
USE_OSGPLUGIN(zip)
USE_OSGPLUGIN(rgb)
USE_OSGPLUGIN(OpenFlight)

GlobalInitializer::GlobalInitializer()
{
    GDALAllRegister();
    CPLSetConfigOption("GDAL_PAM_ENABLED", "NO");
}

GlobalInitializer::~GlobalInitializer() noexcept
{
    osgDB::Registry::instance(true);
}

} // namespace CDBTo3DTiles

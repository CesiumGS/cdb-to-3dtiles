#include "CDBTo3DTiles.h"
#include "CDB.h"
#include "Gltf.h"
#include "Math.h"
#include "TileFormatIO.h"
#include "cpl_conv.h"
#include "gdal.h"
#include "osgDB/WriteFile"
#include "FileUtil.h"
#include <unordered_map>
#include <unordered_set>
#include <morton.h>
#include <nlohmann/json.hpp>
// #include "ConverterImpl.h"
using json = nlohmann::json;

namespace CDBTo3DTiles {

inline uint64_t alignTo8(uint64_t v)
{
    return (v + 7) & ~7;
}
Converter::Converter(const std::filesystem::path &CDBPath, const std::filesystem::path &outputPath)
{
    m_impl = std::make_unique<ConverterImpl>(CDBPath, outputPath);
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

void Converter::setGenerateElevationNormal(bool elevationNormal)
{
    m_impl->elevationNormal = elevationNormal;
}

void Converter::setElevationLODOnly(bool elevationLOD)
{
    m_impl->elevationLOD = elevationLOD;
}

void Converter::setThreeDTilesNext(bool threeDTilesNext)
{
    m_impl->threeDTilesNext = threeDTilesNext;
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
    std::map<std::string, std::vector<std::filesystem::path>> combinedTilesets;
    std::map<std::string, std::vector<Core::BoundingRegion>> combinedTilesetsRegions;
    std::map<std::string, Core::BoundingRegion> aggregateTilesetsRegion;

    if(m_impl->threeDTilesNext) {
        const uint64_t subtreeLevels = 10; // TODO: adjust this with option
        const uint64_t subtreeNodeCount = ((1UL << (2 * subtreeLevels)) - 1) / 3;
        const uint64_t childSubtreeCount = 1 << (2 * subtreeLevels); // 4^N

        const uint64_t availabilityByteLength = 1 + (subtreeNodeCount - 1) / 8;
        const uint64_t nodeAvailabilityByteLengthWithPadding = alignTo8(availabilityByteLength);
        const uint64_t childSubtreeAvailabilityByteLength = 1 + (childSubtreeCount - 1) / 8;
        const uint64_t childSubtreeAvailabilityByteLengthWithPadding = alignTo8(childSubtreeAvailabilityByteLength);

        const uint64_t headerByteLength = 24;
        const uint64_t childSubtreeAvailabilityByteOffset = headerByteLength + availabilityByteLength;
        const uint64_t bufferByteLength = availabilityByteLength + headerByteLength + childSubtreeAvailabilityByteLength;
        std::vector<uint8_t> buffer(bufferByteLength * 2);
        uint8_t* tempBuffer = &buffer[0];
        uint8_t* outBuffer = &buffer[bufferByteLength];
        uint8_t* nodeAvailabilityBuffer = &tempBuffer[headerByteLength];
        uint8_t* childSubtreeAvailabilityBuffer = &tempBuffer[childSubtreeAvailabilityByteOffset];

        cdb.forEachGeoCell([&](CDBGeoCell geoCell) {
          memset(&nodeAvailabilityBuffer[0], 0, bufferByteLength);
          std::filesystem::path geoCellRelativePath = geoCell.getRelativePath();
          std::filesystem::path geoCellAbsolutePath = m_impl->outputPath / geoCellRelativePath;
          std::filesystem::path elevationDir = geoCellAbsolutePath / ConverterImpl::ELEVATIONS_PATH;
          uint64_t availableNodeCount = 0;
          uint64_t availableChildCount = 0;
          cdb.forEachElevationTile(geoCell, [&](CDBElevation elevation) {
            m_impl->addElevationAvailability(elevation, cdb, nodeAvailabilityBuffer, childSubtreeAvailabilityBuffer, subtreeLevels, availableNodeCount, availableChildCount);
            m_impl->addElevationToTilesetCollection(elevation, cdb, elevationDir);
          });
          memcpy(&outBuffer[headerByteLength], nodeAvailabilityBuffer, bufferByteLength);

          // create subtreeJson
          json subtreeJson;
          uint64_t bufferViewIndexAccum = 0;
          uint64_t bufferByteLengthAccum = 0;
          bool constantNodeAvailability = (availableNodeCount == 0) || (availableNodeCount == subtreeNodeCount);
          if(constantNodeAvailability)
          {
            int constant = static_cast<int>(availableNodeCount != 0);
            subtreeJson["tileAvailability"]["constant"] = constant;
            subtreeJson["contentAvailability"]["constant"] = constant;
          }
          else
          {
            subtreeJson["tileAvailability"]["bufferView"] = bufferViewIndexAccum;
            subtreeJson["contentAvailability"]["bufferView"] = bufferViewIndexAccum;
            subtreeJson["bufferViews"].push_back({
                            {"buffer", 0},
                            {"byteOffset", bufferByteLengthAccum},
                            {"byteLength", availabilityByteLength}
                        });
            bufferByteLengthAccum += nodeAvailabilityByteLengthWithPadding;
            bufferViewIndexAccum += 1;
          }

          bool constantChildAvailability = (availableChildCount==0) || (availableChildCount == childSubtreeCount);
          if(constantChildAvailability)
          {
            int constant = static_cast<int>(availableChildCount != 0);
            subtreeJson["childSubtreeAvailability"]["constant"] = constant;
          }
          else
          {
            subtreeJson["childSubtreeAvailability"]["bufferView"] = bufferViewIndexAccum;
            subtreeJson["bufferViews"].push_back({
                            {"buffer", 0},
                            {"byteOffset", bufferByteLengthAccum},
                            {"byteLength", childSubtreeAvailabilityByteLength}
                        });
            bufferByteLengthAccum += childSubtreeAvailabilityByteLengthWithPadding;
          }

          // get json length
          const std::string jsonString = subtreeJson.dump();
          const uint64_t jsonStringByteLength = jsonString.size();
          const uint64_t jsonStringByteLengthWithPadding = alignTo8(jsonStringByteLength);

          // Write subtree binary
          *(uint32_t*)&outBuffer[0] = 0x74627573; // magic: "subt"
          *(uint32_t*)&outBuffer[4] = 1; // version
          *(uint64_t*)&outBuffer[8] = jsonStringByteLengthWithPadding; // JSON byte length with padding
          *(uint64_t*)&outBuffer[16] = bufferByteLengthAccum; // BIN byte length with padding

          memcpy(&outBuffer[headerByteLength], &jsonString[0], jsonStringByteLength);
          memset(&outBuffer[headerByteLength + jsonStringByteLength], ' ', jsonStringByteLengthWithPadding - jsonStringByteLength);
          uint64_t outBufferByteOffset = headerByteLength + jsonStringByteLengthWithPadding;

          if(!constantNodeAvailability)
          {
            memcpy(&outBuffer[outBufferByteOffset], nodeAvailabilityBuffer, nodeAvailabilityByteLengthWithPadding); // BIN node availability (already padded with 0s)
            outBufferByteOffset += nodeAvailabilityByteLengthWithPadding;
          }
          if (!constantChildAvailability)
          {
            memcpy(&outBuffer[outBufferByteOffset], childSubtreeAvailabilityBuffer, childSubtreeAvailabilityByteLengthWithPadding); // BIN child subtree availability (already padded with 0s)
            outBufferByteOffset += childSubtreeAvailabilityByteLengthWithPadding;
          }

          // TODO hardcoded subtree file name
          std::filesystem::path path = geoCellAbsolutePath / "Elevation" / "subtrees" / "0_0_0.subtree";
          AGI::Utilities::writeBinaryFile(path , (const char*)outBuffer, outBufferByteOffset);


          std::ofstream fs(m_impl->outputPath / "tileset.json");
          createImplicitJson(geoCell, fs);
        });
        exit(0);
    }
    else {
      cdb.forEachGeoCell([&](CDBGeoCell geoCell) {
          // create directories for converted GeoCell
          std::filesystem::path geoCellRelativePath = geoCell.getRelativePath();
          std::filesystem::path geoCellAbsolutePath = m_impl->outputPath / geoCellRelativePath;
          std::filesystem::path elevationDir = geoCellAbsolutePath / ConverterImpl::ELEVATIONS_PATH;
          std::filesystem::path GTModelDir = geoCellAbsolutePath / ConverterImpl::GTMODEL_PATH;
          std::filesystem::path GSModelDir = geoCellAbsolutePath / ConverterImpl::GSMODEL_PATH;
          std::filesystem::path roadNetworkDir = geoCellAbsolutePath / ConverterImpl::ROAD_NETWORK_PATH;
          std::filesystem::path railRoadNetworkDir = geoCellAbsolutePath / ConverterImpl::RAILROAD_NETWORK_PATH;
          std::filesystem::path powerlineNetworkDir = geoCellAbsolutePath / ConverterImpl::POWERLINE_NETWORK_PATH;
          std::filesystem::path hydrographyNetworkDir = geoCellAbsolutePath / ConverterImpl::HYDROGRAPHY_NETWORK_PATH;

          // process elevation
          cdb.forEachElevationTile(geoCell, [&](CDBElevation elevation) {
              m_impl->addElevationToTilesetCollection(elevation, cdb, elevationDir)
              ;
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
          cdb.forEachGTModelTile(geoCell, [&](CDBGTModels GTModel) {
              m_impl->addGTModelToTilesetCollection(GTModel, GTModelDir);
          });
          m_impl->flushTilesetCollection(geoCell, m_impl->GTModelTilesets);

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
                  tilesetAggregateRegion->second = tilesetAggregateRegion->second.computeUnion(geoCellRegion);
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

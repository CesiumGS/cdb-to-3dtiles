#pragma once

#include "CDBAttributes.h"
#include "CDBTileset.h"
#include "tiny_gltf.h"
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>

namespace CDBTo3DTiles {

void combineTilesetJson(const std::vector<std::filesystem::path> &tilesetJsonPaths,
                        const std::vector<Core::BoundingRegion> &regions,
                        std::ofstream &fs);

void writeToTilesetJson(const CDBTileset &tileset, bool replace, std::ofstream &fs);

size_t writeToI3DM(std::string GltfURI,
                   const CDBInstancesAttributes &instancesAttributes,
                   const std::vector<Core::Cartographic> &cartographicPositions,
                   const std::vector<glm::vec3> &scales,
                   const std::vector<double> &orientations,
                   std::ofstream &fs);

void writeToB3DM(tinygltf::Model *gltf, const CDBInstancesAttributes *instancesAttribs, std::ofstream &fs);

void writeToCMPT(uint32_t numOfTiles,
                 std::ofstream &fs,
                 std::function<uint32_t(std::ofstream &fs, size_t tileIdx)> writeToTileFormat);

} // namespace CDBTo3DTiles

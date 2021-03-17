
#pragma once

#include "CDBTileset.h"
#include "gdal_priv.h"
#include "rapidxml.hpp"
#include "Gltf.h"

namespace CDBTo3DTiles {
class CDBRMDescriptor
{
private:
    std::filesystem::path _xmlPath;
    std::optional<CDBTile> _tile;
public:
    CDBRMDescriptor(std::filesystem::path xmlPath, const CDBTile &tile);
    void addFeatureTable(tinygltf::Model *gltf);
};
} // namespace CDBTo3DTiles

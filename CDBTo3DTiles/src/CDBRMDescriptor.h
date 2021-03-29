
#pragma once

#include "CDBTileset.h"
#include "gdal_priv.h"
#include "rapidxml.hpp"
#include "CDBMaterials.h"
#include "Gltf.h"

namespace CDBTo3DTiles {
class CDBRMDescriptor
{
private:
    std::filesystem::path _xmlPath;
    std::optional<CDBTile> _tile;
public:
    CDBRMDescriptor(std::filesystem::path xmlPath, const CDBTile &tile);
    void addFeatureTableToGltf(CDBMaterials *materials, tinygltf::Model *gltf, bool useExternalSchema);
};
} // namespace CDBTo3DTiles

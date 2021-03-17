
#pragma once

#include "CDBTileset.h"
#include "gdal_priv.h"
#include "json.hpp"
#include "rapidxml.hpp"
#include "tiny_gltf.h"

namespace CDBTo3DTiles {
class CDBRMDescriptor
{
public:
    CDBRMDescriptor(std::filesystem::path xmlPath, const CDBTile &tile);

    void addFeatureTable(tinygltf::Model *gltf);

private:
    rapidxml::xml_document<> _xml;
    std::optional<CDBTile> _tile;
};
} // namespace CDBTo3DTiles

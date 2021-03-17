
#pragma once

#include "CDBTileset.h"
#include "gdal_priv.h"
#include "rapidxml.hpp"

namespace CDBTo3DTiles {
class CDBRMDescriptor
{
public:
    CDBRMDescriptor(std::filesystem::path xmlPath, const CDBTile &tile);

private:
    rapidxml::xml_document<> _xml;
    std::optional<CDBTile> _tile;
};
} // namespace CDBTo3DTiles

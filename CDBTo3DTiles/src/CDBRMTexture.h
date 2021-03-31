#pragma once

#include "CDBTileset.h"
#include "gdal_priv.h"

namespace CDBTo3DTiles {
class CDBRMTexture
{
public:
    CDBRMTexture(GDALDatasetUniquePtr rmTextureDataset, const CDBTile &tile);

    inline GDALDataset &getData() noexcept { return *_data; }

    inline const CDBTile &getTile() const noexcept { return *_tile; }

private:
    GDALDatasetUniquePtr _data;
    std::optional<CDBTile> _tile;
};
} // namespace CDBTo3DTiles

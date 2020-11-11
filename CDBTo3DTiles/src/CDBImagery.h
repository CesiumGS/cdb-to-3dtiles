#pragma once

#include "CDBTileset.h"
#include "gdal_priv.h"

namespace CDBTo3DTiles {
class CDBImagery
{
public:
    CDBImagery(GDALDatasetUniquePtr imageryDataset, const CDBTile &tile);

    inline const GDALDataset &getData() const noexcept { return *_data; }

    inline GDALDataset &getData() noexcept { return *_data; }

    inline const CDBTile &getTile() const noexcept { return *_tile; }

private:
    GDALDatasetUniquePtr _data;
    std::optional<CDBTile> _tile;
};
} // namespace CDBTo3DTiles

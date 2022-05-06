#include "CDBRMTexture.h"

namespace CDBTo3DTiles {

CDBRMTexture::CDBRMTexture(GDALDatasetUniquePtr rmTextureDataset, const CDBTile &tile)
    : _data{std::move(rmTextureDataset)}
    , _tile{tile}
{}

} // namespace CDBTo3DTiles

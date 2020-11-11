#include "CDBImagery.h"

namespace CDBTo3DTiles {

CDBImagery::CDBImagery(GDALDatasetUniquePtr imageryDataset, const CDBTile &tile)
    : _data{std::move(imageryDataset)}
    , _tile{tile}
{}

} // namespace CDBTo3DTiles

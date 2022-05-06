#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include "../src/CDBTilesetBuilder.h"

namespace CDBTo3DTiles {
class GlobalInitializer
{
public:
    GlobalInitializer();

    ~GlobalInitializer() noexcept;
};

class Converter
{
public:
    Converter(const std::filesystem::path &CDBPath, const std::filesystem::path &outputPath);

    ~Converter() noexcept;

    void combineDataset(const std::vector<std::string> &datasets);

    void setUse3dTilesNext(bool use3dTilesNext);

    void setExternalSchema(bool externalSchema);

    void setGenerateElevationNormal(bool generateElevationNormal);

    void setSubtreeLevels(int subtreeLevels);

    void setElevationLODOnly(bool elevationLOD);

    void setElevationDecimateError(float elevationDecimateError);

    void setElevationThresholdIndices(float elevationThresholdIndices);

    void convert();

private:
    struct TilesetCollection;

    std::unique_ptr<CDBTilesetBuilder> m_impl;
};
} // namespace CDBTo3DTiles

#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include "../src/ConverterImpl.h"
// #include "ConverterImpl.h"

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

    void setGenerateElevationNormal(bool generateElevationNormal);

    void setThreeDTilesNext(bool threeDTilesNext);

    void setElevationLODOnly(bool elevationLOD);

    void setElevationDecimateError(float elevationDecimateError);

    void setElevationThresholdIndices(float elevationThresholdIndices);

    void convert();

private:
    // struct Impl;
    struct TilesetCollection;

    std::unique_ptr<ConverterImpl> m_impl;
};
} // namespace CDBTo3DTiles

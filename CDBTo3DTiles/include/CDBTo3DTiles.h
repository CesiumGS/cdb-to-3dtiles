#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

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

    void setElevationLODOnly(bool elevationLOD);

    void setElevationDecimateError(float elevationDecimateError);

    void setElevationThresholdIndices(float elevationThresholdIndices);

    void convert();

private:
    struct Impl;
    struct TilesetCollection;

    std::unique_ptr<Impl> m_impl;
};
} // namespace CDBTo3DTiles

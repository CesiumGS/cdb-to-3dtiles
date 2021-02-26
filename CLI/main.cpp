#define CXXOPTS_VECTOR_DELIMITER ';'
#include "CDBTo3DTiles.h"
#include "Utility.h"
#include "cxxopts.hpp"
#include <iostream>

int main(int argc, char **argv)
{
    cxxopts::Options options("CDBConverter", "Convert CDB to 3D Tiles");

    // clang-format off
    options.add_options()
        ("i, input",
            "CDB directory",
            cxxopts::value<std::string>())
        ("o, output",
            "3D Tiles output directory",
            cxxopts::value<std::string>())
        ("3d-tiles-next",
            "Use 3D Tiles Next extensions",
            cxxopts::value<bool>()->default_value("false"))
        ("combine",
            "Combine converted datasets into one tileset. Each dataset format is {DatasetName}_{ComponentSelector1}_{ComponentSelector2}. "
            "Repeat this option to group different dataset into different tilesets. "
            "E.g: --combine=Elevation_1_1,GSModels_1_1 --combine=GTModels_2_1,GTModels_1_1 will combine Elevation_1_1 and GSModels_1_1 into one tileset. GTModels_2_1 and GTModels_1_1 will be combined into a different tileset",
            cxxopts::value<std::vector<std::string>>()->default_value("Elevation_1_1,GSModels_1_1,GTModels_2_1,GTModels_1_1"))
        ("elevation-normal",
            "Generate elevation normal",
            cxxopts::value<bool>()->default_value("false"))
        ("elevation-lod",
            "Generate elevation and imagery based on elevation LOD only",
            cxxopts::value<bool>()->default_value("false"))
        ("elevation-decimate-error",
            "Set target error when decimating elevation mesh. Target error is normalized to 0..1 (0.01 means the simplifier maintains the error to be below 1% of the mesh extents)",
            cxxopts::value<float>()->default_value("0.01"))
        ("elevation-threshold-indices",
            "Set target percent of indices when decimating elevation mesh",
            cxxopts::value<float>()->default_value("0.3"))
        ("h, help", "Print usage");
    // clang-format on

    auto result = options.parse(argc, argv);
    if (result.count("help")) {
        std::cout << options.help() << "\n";
        return 0;
    }

    try {
        if (result.count("input") && result.count("output")) {
            std::filesystem::path CDBPath = result["input"].as<std::string>();
            std::filesystem::path outputPath = result["output"].as<std::string>();

            bool use3dTilesNext = result["3d-tiles-next"].as<bool>();
            bool generateElevationNormal = result["elevation-normal"].as<bool>();
            bool elevationLOD = result["elevation-lod"].as<bool>();
            float elevationDecimateError = result["elevation-decimate-error"].as<float>();
            float elevationThresholdIndices = result["elevation-threshold-indices"].as<float>();
            std::vector<std::string> combinedDatasets = result["combine"].as<std::vector<std::string>>();

            CDBTo3DTiles::GlobalInitializer initializer;
            CDBTo3DTiles::Converter converter(CDBPath, outputPath);
            converter.setUse3dTilesNext(use3dTilesNext);
            converter.setGenerateElevationNormal(generateElevationNormal);
            converter.setElevationLODOnly(elevationLOD);
            converter.setElevationDecimateError(elevationDecimateError);
            converter.setElevationThresholdIndices(elevationThresholdIndices);
            for (const auto &combined : combinedDatasets) {
                converter.combineDataset(CDBTo3DTiles::splitString(combined, ","));
            }

            converter.convert();
        } else {
            std::cout << options.help();
            return 0;
        }
    } catch (const std::exception &e) {
        std::cout << "An error has occured: " << e.what() << "\n";
    }

    return 0;
}

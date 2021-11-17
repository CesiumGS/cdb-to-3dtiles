#include "CDBModels.h"
#include "CDBTo3DTiles.h"
#include "Config.h"
#include <doctest/doctest.h>
#include "nlohmann/json.hpp"
#include "ogrsf_frmts.h"
#include <gdal_priv.h>
#include <filesystem>

using namespace CDBTo3DTiles;



TEST_SUITE_BEGIN("CDBGTModelCache");

TEST_CASE("Test locating GTModel with metadata in CDB database")
{
    std::filesystem::path CDBPath = dataPath / "GTModels";
    std::filesystem::path input = CDBPath / "Tiles" / "N32" / "W118" / "101_GTFeature" / "L00" / "U0"
                                  / "N32W118_D101_S001_T001_L00_U0_R0.dbf";

    // construct GTModel cache
    // CDBGTModelCache GTModelCache(CDBPath);

    // read in the GTFeature data
    GDALDatasetUniquePtr attributesDataset = GDALDatasetUniquePtr(
        (GDALDataset *) GDALOpenEx(input.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
    CHECK(attributesDataset != nullptr);

    auto GTFeatureTile = CDBTile::createFromFile(input.filename().string());
    CDBModelsAttributes modelsAttributes(std::move(attributesDataset), *GTFeatureTile, CDBPath);

    // CDBGTModels models(std::move(modelsAttributes), &GTModelCache);
    // const auto &modelsAttribs = models.getModelsAttributes();
    // const auto &instancesAttribs = modelsAttribs.getInstancesAttributes();

    // // check total instances
    // size_t instancesCount = instancesAttribs.getInstancesCount();
    // CHECK(instancesCount == 1);

    // // check if we can locate the model
    // std::string modelKey;
    // auto model3D = models.locateModel3D(0, modelKey);
    // CHECK(model3D != nullptr);
    // CHECK(modelKey == "D500_S001_T001_AL015_000_coronado_bridge");

    // // check model metadata
    // const auto &CNAMs = instancesAttribs.getCNAMs();
    // const auto &stringAttribs = instancesAttribs.getStringAttribs();
    // const auto &doubleAttribs = instancesAttribs.getDoubleAttribs();
    // const auto &integerAttribs = instancesAttribs.getIntegerAttribs();
    // CHECK(CNAMs.front() == "AL015000-54-44U44R54-0");

    // CHECK(stringAttribs.at("FACC").front() == "AL015");
    // CHECK(stringAttribs.at("MODL").front() == "coronado_bridge");

    // CHECK(integerAttribs.at("CMIX").front() == 2);
    // CHECK(integerAttribs.at("FSC").front() == 0);
    // CHECK(integerAttribs.at("SSC").front() == 0);
    // CHECK(integerAttribs.at("RTAI").front() == 100);
    // CHECK(integerAttribs.at("SRD").front() == 0);

    // CHECK(doubleAttribs.at("BBH").front() == doctest::Approx(79.248));
    // CHECK(doubleAttribs.at("BBL").front() == doctest::Approx(1730.210));
    // CHECK(doubleAttribs.at("BBW").front() == doctest::Approx(2211.903));
    // CHECK(doubleAttribs.at("BSR").front() == doctest::Approx(1404.673));
    // CHECK(doubleAttribs.at("HGT").front() == doctest::Approx(79.25));
    // CHECK(doubleAttribs.at("AO1").front() == doctest::Approx(0.0));
    // CHECK(doubleAttribs.at("SCALx").front() == doctest::Approx(1.0));
    // CHECK(doubleAttribs.at("SCALy").front() == doctest::Approx(1.0));
    // CHECK(doubleAttribs.at("SCALz").front() == doctest::Approx(1.0));

    // // check orientation, scale, and position are parsed correctly
    // auto cartographic = modelsAttribs.getCartographicPositions().front();
    // CHECK(cartographic.longitude == doctest::Approx(glm::radians(-117.152537805556)));
    // CHECK(cartographic.latitude == doctest::Approx(glm::radians(32.6943141111111)));
    // CHECK(cartographic.height == doctest::Approx(0.0));

    // CHECK(modelsAttribs.getOrientations().front() == doctest::Approx(0.0));

    // auto scale = modelsAttribs.getScales().front();
    // CHECK(scale.x == doctest::Approx(1.0));
    // CHECK(scale.y == doctest::Approx(1.0));
    // CHECK(scale.z == doctest::Approx(1.0));
}


TEST_SUITE_END();

// static void checkGTTilesetDirectoryStructure(const std::filesystem::path &tilesetOutput,
//                                              const std::filesystem::path &CDBPath,
//                                              const std::filesystem::path &verifiedTileset,
//                                              const std::filesystem::path &tilesetJsonName,
//                                              size_t expectedGTFeatureCount)
// {
//     CHECK(std::filesystem::exists(tilesetOutput / tilesetJsonName));
//     CHECK(std::filesystem::exists(tilesetOutput / "Gltf"));
//     CHECK(std::filesystem::exists(tilesetOutput / "Gltf" / "Textures"));
//     size_t GTFeatureCount = 0;
//     for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(tilesetOutput)) {
//         auto tile = CDBTile::createFromFile(entry.path().filename());
//         if (tile) {
//             CHECK(std::filesystem::exists(CDBPath / (tile->getRelativePath().string() + ".dbf")));
//             ++GTFeatureCount;
//         }
//     }
//     CHECK(GTFeatureCount == expectedGTFeatureCount);

//     std::filesystem::path tilesetPath = tilesetOutput / tilesetJsonName;
//     std::ifstream verifiedJS(CDBPath / verifiedTileset);
//     std::ifstream testJS(tilesetPath);
//     nlohmann::json verifiedJson = nlohmann::json::parse(verifiedJS);
//     nlohmann::json testJson = nlohmann::json::parse(testJS);
//     CHECK(testJson == verifiedJson);
// }



// TEST_SUITE_BEGIN("CDBGTModelCache");

// TEST_CASE("Test locating GTModel in CDB database")
// {
//     SUBCASE("Successfully find model")
//     {
//         std::filesystem::path input = dataPath / "GTModels";

//         CDBGTModelCache GTModelCache(input);
//         std::string modelKey;
//         auto model3DResult = GTModelCache.locateModel3D("AL015", "coronado_bridge", 0, modelKey);
//         CHECK(model3DResult != nullptr);
//         CHECK(model3DResult->getMaterials().size() == 3);
//         CHECK(model3DResult->getMeshes().size() == 3);
//         CHECK(model3DResult->getTextures().size() == 3);
//         CHECK(modelKey == "D500_S001_T001_AL015_000_coronado_bridge");
//     }

//     SUBCASE("Model not exist")
//     {
//         std::filesystem::path input = dataPath / "GTModels";   

//         CDBGTModelCache GTModelCache(input);
//         std::string modelKey;
//         auto model3DResult = GTModelCache.locateModel3D("122", "coronado_bridge", 0, modelKey);
//         CHECK(model3DResult == nullptr);
//     }
// }

// TEST_CASE("Test locating GTModel with metadata in CDB database")
// {
//     std::filesystem::path CDBPath = dataPath / "GTModels";
//     std::filesystem::path input = CDBPath / "Tiles" / "N32" / "W118" / "101_GTFeature" / "L00" / "U0"
//                                   / "N32W118_D101_S001_T001_L00_U0_R0.dbf";

//     // construct GTModel cache
//     CDBGTModelCache GTModelCache(CDBPath);

//     // read in the GTFeature data
//     GDALDatasetUniquePtr attributesDataset = GDALDatasetUniquePtr(
//         (GDALDataset *) GDALOpenEx(input.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
//     CHECK(attributesDataset != nullptr);

//     auto GTFeatureTile = CDBTile::createFromFile(input.filename().string());
//     CDBModelsAttributes modelsAttributes(std::move(attributesDataset), *GTFeatureTile, CDBPath);

//     CDBGTModels models(std::move(modelsAttributes), &GTModelCache);
//     const auto &modelsAttribs = models.getModelsAttributes();
//     const auto &instancesAttribs = modelsAttribs.getInstancesAttributes();

//     // check total instances
//     size_t instancesCount = instancesAttribs.getInstancesCount();
//     CHECK(instancesCount == 1);

//     // check if we can locate the model
//     std::string modelKey;
//     auto model3D = models.locateModel3D(0, modelKey);
//     CHECK(model3D != nullptr);
//     CHECK(modelKey == "D500_S001_T001_AL015_000_coronado_bridge");

//     // check model metadata
//     const auto &CNAMs = instancesAttribs.getCNAMs();
//     const auto &stringAttribs = instancesAttribs.getStringAttribs();
//     const auto &doubleAttribs = instancesAttribs.getDoubleAttribs();
//     const auto &integerAttribs = instancesAttribs.getIntegerAttribs();
//     CHECK(CNAMs.front() == "AL015000-54-44U44R54-0");

//     CHECK(stringAttribs.at("FACC").front() == "AL015");
//     CHECK(stringAttribs.at("MODL").front() == "coronado_bridge");

//     CHECK(integerAttribs.at("CMIX").front() == 2);
//     CHECK(integerAttribs.at("FSC").front() == 0);
//     CHECK(integerAttribs.at("SSC").front() == 0);
//     CHECK(integerAttribs.at("RTAI").front() == 100);
//     CHECK(integerAttribs.at("SRD").front() == 0);

//     CHECK(doubleAttribs.at("BBH").front() == doctest::Approx(79.248));
//     CHECK(doubleAttribs.at("BBL").front() == doctest::Approx(1730.210));
//     CHECK(doubleAttribs.at("BBW").front() == doctest::Approx(2211.903));
//     CHECK(doubleAttribs.at("BSR").front() == doctest::Approx(1404.673));
//     CHECK(doubleAttribs.at("HGT").front() == doctest::Approx(79.25));
//     CHECK(doubleAttribs.at("AO1").front() == doctest::Approx(0.0));
//     CHECK(doubleAttribs.at("SCALx").front() == doctest::Approx(1.0));
//     CHECK(doubleAttribs.at("SCALy").front() == doctest::Approx(1.0));
//     CHECK(doubleAttribs.at("SCALz").front() == doctest::Approx(1.0));

//     // check orientation, scale, and position are parsed correctly
//     auto cartographic = modelsAttribs.getCartographicPositions().front();
//     CHECK(cartographic.longitude == doctest::Approx(glm::radians(-117.152537805556)));
//     CHECK(cartographic.latitude == doctest::Approx(glm::radians(32.6943141111111)));
//     CHECK(cartographic.height == doctest::Approx(0.0));

//     CHECK(modelsAttribs.getOrientations().front() == doctest::Approx(0.0));

//     auto scale = modelsAttribs.getScales().front();
//     CHECK(scale.x == doctest::Approx(1.0));
//     CHECK(scale.y == doctest::Approx(1.0));
//     CHECK(scale.z == doctest::Approx(1.0));
// }

// TEST_CASE("Test CDBGTModels conversion to tileset.json")
// {
//     std::filesystem::path CDBPath = dataPath / "GTModels";
//     std::filesystem::path output = "GTModels";
//     Converter converter(CDBPath, output);
//     converter.convert();

//     // check the structure of bridge tileset
//     std::filesystem::path bridgeOutputPath = output / "Tiles" / "N32" / "W118" / "GTModels" / "1_1";
//     checkGTTilesetDirectoryStructure(bridgeOutputPath,
//                                      CDBPath,
//                                      "VerifiedBridgeTileset.json",
//                                      "N32W118_D101_S001_T001.json",
//                                      1); // only L0 is where point features are present

//     // check the structure of tree tileset
//     std::filesystem::path treeOutputPath = output / "Tiles" / "N32" / "W118" / "GTModels" / "2_1";
//     checkGTTilesetDirectoryStructure(treeOutputPath,
//                                      CDBPath,
//                                      "VerifiedTreeTileset.json",
//                                      "N32W118_D101_S002_T001.json",
//                                      1); // only L0 is where point features are present

//     std::filesystem::remove_all(output);
// }

TEST_SUITE_END();
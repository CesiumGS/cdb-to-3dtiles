#include "CDBModels.h"
#include "CDBTo3DTiles.h"
#include "Config.h"
#include "catch2/catch.hpp"
#include "nlohmann/json.hpp"
#include "ogrsf_frmts.h"
#include <filesystem>

using namespace CDBTo3DTiles;

static void checkGTTilesetDirectoryStructure(const std::filesystem::path &tilesetOutput,
                                             const std::filesystem::path &CDBPath,
                                             const std::filesystem::path &verifiedTileset,
                                             size_t expectedGTFeatureCount)
{
    REQUIRE(std::filesystem::exists(tilesetOutput / "tileset.json"));
    REQUIRE(std::filesystem::exists(tilesetOutput / "Gltf"));
    REQUIRE(std::filesystem::exists(tilesetOutput / "Gltf" / "Textures"));
    size_t GTFeatureCount = 0;
    for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(tilesetOutput)) {
        auto tile = CDBTile::createFromFile(entry.path().filename().string());
        if (tile) {
            REQUIRE(std::filesystem::exists(CDBPath / (tile->getRelativePath().string() + ".dbf")));
            ++GTFeatureCount;
        }
    }
    REQUIRE(GTFeatureCount == expectedGTFeatureCount);

    std::filesystem::path tilesetPath = tilesetOutput / "tileset.json";
    std::ifstream verifiedJS(CDBPath / verifiedTileset);
    std::ifstream testJS(tilesetPath);
    nlohmann::json verifiedJson = nlohmann::json::parse(verifiedJS);
    nlohmann::json testJson = nlohmann::json::parse(testJS);
    REQUIRE(testJson == verifiedJson);
}

TEST_CASE("Test locating GTModel in CDB database", "[CDBGTModelCache]")
{
    SECTION("Successfully find model")
    {
        std::filesystem::path input = dataPath / "GTModels";

        CDBGTModelCache GTModelCache(input);
        std::string modelKey;
        auto model3DResult = GTModelCache.locateModel3D("AL015", "coronado_bridge", 0, modelKey);
        REQUIRE(model3DResult != nullptr);
        REQUIRE(model3DResult->getMaterials().size() == 3);
        REQUIRE(model3DResult->getMeshes().size() == 3);
        REQUIRE(model3DResult->getTextures().size() == 3);
        REQUIRE(modelKey == "D500_S001_T001_AL015_000_coronado_bridge");
    }

    SECTION("Model not exist")
    {
        std::filesystem::path input = dataPath / "GTModels";

        CDBGTModelCache GTModelCache(input);
        std::string modelKey;
        auto model3DResult = GTModelCache.locateModel3D("122", "coronado_bridge", 0, modelKey);
        REQUIRE(model3DResult == nullptr);
    }
}

TEST_CASE("Test locating GTModel with metadata in CDB database", "[CDBGTModels]")
{
    std::filesystem::path CDBPath = dataPath / "GTModels";
    std::filesystem::path input = CDBPath / "Tiles" / "N32" / "W118" / "101_GTFeature" / "L00" / "U0"
                                  / "N32W118_D101_S001_T001_L00_U0_R0.dbf";

    // construct GTModel cache
    CDBGTModelCache GTModelCache(CDBPath);

    // read in the GTFeature data
    GDALDatasetUniquePtr attributesDataset = GDALDatasetUniquePtr(
        (GDALDataset *) GDALOpenEx(input.string().c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
    REQUIRE(attributesDataset != nullptr);

    auto GTFeatureTile = CDBTile::createFromFile(input.filename().string());
    CDBModelsAttributes modelsAttributes(std::move(attributesDataset), *GTFeatureTile, CDBPath);

    CDBGTModels models(std::move(modelsAttributes), &GTModelCache);
    const auto &modelsAttribs = models.getModelsAttributes();
    const auto &instancesAttribs = modelsAttribs.getInstancesAttributes();

    // check total instances
    size_t instancesCount = instancesAttribs.getInstancesCount();
    REQUIRE(instancesCount == 1);

    // check if we can locate the model
    std::string modelKey;
    auto model3D = models.locateModel3D(0, modelKey);
    REQUIRE(model3D != nullptr);
    REQUIRE(modelKey == "D500_S001_T001_AL015_000_coronado_bridge");

    // check model metadata
    const auto &CNAMs = instancesAttribs.getCNAMs();
    const auto &stringAttribs = instancesAttribs.getStringAttribs();
    const auto &doubleAttribs = instancesAttribs.getDoubleAttribs();
    const auto &integerAttribs = instancesAttribs.getIntegerAttribs();
    REQUIRE(CNAMs.front() == "AL015000-54-44U44R54-0");

    REQUIRE(stringAttribs.at("FACC").front() == "AL015");
    REQUIRE(stringAttribs.at("MODL").front() == "coronado_bridge");

    REQUIRE(integerAttribs.at("CMIX").front() == 2);
    REQUIRE(integerAttribs.at("FSC").front() == 0);
    REQUIRE(integerAttribs.at("SSC").front() == 0);
    REQUIRE(integerAttribs.at("RTAI").front() == 100);
    REQUIRE(integerAttribs.at("SRD").front() == 0);

    REQUIRE(doubleAttribs.at("BBH").front() == Approx(79.248));
    REQUIRE(doubleAttribs.at("BBL").front() == Approx(1730.210));
    REQUIRE(doubleAttribs.at("BBW").front() == Approx(2211.903));
    REQUIRE(doubleAttribs.at("BSR").front() == Approx(1404.673));
    REQUIRE(doubleAttribs.at("HGT").front() == Approx(79.25));
    REQUIRE(doubleAttribs.at("AO1").front() == Approx(0.0));
    REQUIRE(doubleAttribs.at("SCALx").front() == Approx(1.0));
    REQUIRE(doubleAttribs.at("SCALy").front() == Approx(1.0));
    REQUIRE(doubleAttribs.at("SCALz").front() == Approx(1.0));

    // check orientation, scale, and position are parsed correctly
    auto cartographic = modelsAttribs.getCartographicPositions().front();
    REQUIRE(cartographic.longitude == Approx(glm::radians(-117.152537805556)));
    REQUIRE(cartographic.latitude == Approx(glm::radians(32.6943141111111)));
    REQUIRE(cartographic.height == Approx(0.0));

    REQUIRE(modelsAttribs.getOrientations().front() == Approx(0.0));

    auto scale = modelsAttribs.getScales().front();
    REQUIRE(scale.x == Approx(1.0));
    REQUIRE(scale.y == Approx(1.0));
    REQUIRE(scale.z == Approx(1.0));
}

TEST_CASE("Test CDBGTModels conversion to tileset.json", "[CDBGTModels]")
{
    std::filesystem::path CDBPath = dataPath / "GTModels";
    std::filesystem::path output = "GTModels";
    Converter converter(CDBPath, output);
    converter.convert();

    // check the structure of bridge tileset
    std::filesystem::path bridgeOutputPath = output / "Tiles" / "N32" / "W118" / "GTModels" / "1_1";
    checkGTTilesetDirectoryStructure(bridgeOutputPath,
                                     CDBPath,
                                     "VerifiedBridgeTileset.json",
                                     1); // only L0 is where point features are present

    // check the structure of tree tileset
    std::filesystem::path treeOutputPath = output / "Tiles" / "N32" / "W118" / "GTModels" / "2_1";
    checkGTTilesetDirectoryStructure(treeOutputPath,
                                     CDBPath,
                                     "VerifiedTreeTileset.json",
                                     1); // only L0 is where point features are present

    std::filesystem::remove_all(output);
}

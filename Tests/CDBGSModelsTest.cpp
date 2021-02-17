#include "CDBModels.h"
#include "CDBTo3DTiles.h"
#include "Config.h"
#include "catch2/catch.hpp"
#include "nlohmann/json.hpp"

using namespace CDBTo3DTiles;

TEST_CASE("Test creating GSModel from model attributes", "[CDBGSModels]")
{
    SECTION("Create GSModel with texture in GTModel directory")
    {
        std::filesystem::path CDBPath = dataPath / "GSModelsWithGTModelTexture";
        std::filesystem::path input = CDBPath / "Tiles" / "N32" / "W118" / "100_GSFeature" / "L00" / "U0"
                                      / "N32W118_D100_S001_T001_L00_U0_R0.dbf";

        // read in the GSFeature data
        GDALDatasetUniquePtr attributesDataset = GDALDatasetUniquePtr(
            (GDALDataset *) GDALOpenEx(input.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
        REQUIRE(attributesDataset != nullptr);

        auto GSFeatureTile = CDBTile::createFromFile(input.filename().string());
        CDBModelsAttributes modelsAttributes(std::move(attributesDataset), *GSFeatureTile, CDBPath);

        // create GSModel (only 8 models in the zip file)
        auto models = CDBGSModels::createFromModelsAttributes(std::move(modelsAttributes), CDBPath);
        REQUIRE(models != std::nullopt);

        auto model3D = models->getModel3D();
        const auto &materials = model3D.getMaterials();
        const auto &meshes = model3D.getMeshes();

        REQUIRE(meshes.size() > 0);
        REQUIRE(meshes.size() == materials.size());

        for (const auto &mesh : meshes) {
            REQUIRE(mesh.positionRTCs.size() > 0);
            REQUIRE(mesh.positions.size() > 0);
            if (mesh.material >= 0 && materials[mesh.material].texture >= 0) {
                REQUIRE(mesh.UVs.size() > 0);
            } else {
                REQUIRE(mesh.UVs.size() == 0);
            }
        }

        // check tile property
        const auto &tile = models->getTile();
        REQUIRE(tile.getGeoCell() == CDBGeoCell(32, -118));
        REQUIRE(tile.getDataset() == CDBDataset::GSModelGeometry);
        REQUIRE(tile.getCS_1() == 1);
        REQUIRE(tile.getCS_2() == 1);
        REQUIRE(tile.getLevel() == 0);
        REQUIRE(tile.getUREF() == 0);
        REQUIRE(tile.getRREF() == 0);
    }

    SECTION("Create GSModel with texture in GSModelTexture directory")
    {
        std::filesystem::path CDBPath = dataPath / "GSModelsWithGSModelTexture";
        std::filesystem::path input = CDBPath / "Tiles" / "N32" / "W118" / "100_GSFeature" / "L00" / "U0"
                                      / "N32W118_D100_S001_T001_L00_U0_R0.dbf";

        // read in the GSFeature data
        GDALDatasetUniquePtr attributesDataset = GDALDatasetUniquePtr(
            (GDALDataset *) GDALOpenEx(input.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
        REQUIRE(attributesDataset != nullptr);

        auto GSFeatureTile = CDBTile::createFromFile(input.filename().string());
        CDBModelsAttributes modelsAttributes(std::move(attributesDataset), *GSFeatureTile, CDBPath);

        // create GSModel (only 1 model in the zip file)
        auto models = CDBGSModels::createFromModelsAttributes(std::move(modelsAttributes), CDBPath);
        REQUIRE(models != std::nullopt);

        auto model3D = models->getModel3D();
        const auto &materials = model3D.getMaterials();
        const auto &meshes = model3D.getMeshes();
        const auto &textures = model3D.getTextures();
        const auto &images = model3D.getImages();

        // check mesh loaded
        REQUIRE(meshes.size() > 0);
        REQUIRE(meshes.size() == materials.size());

        for (const auto &mesh : meshes) {
            REQUIRE(mesh.positionRTCs.size() > 0);
            REQUIRE(mesh.positions.size() > 0);
            if (mesh.material >= 0 && materials[mesh.material].texture >= 0) {
                REQUIRE(mesh.UVs.size() > 0);
            } else {
                REQUIRE(mesh.UVs.size() == 0);
            }
        }

        // the model have 2 textures
        REQUIRE(textures.size() == 2);
        REQUIRE(images.size() == 2);

        // make sure textures are loaded
        for (auto image : model3D.getImages()) {
            REQUIRE(image != nullptr);
        }

        // check tile property
        const auto &tile = models->getTile();
        REQUIRE(tile.getGeoCell() == CDBGeoCell(32, -118));
        REQUIRE(tile.getDataset() == CDBDataset::GSModelGeometry);
        REQUIRE(tile.getCS_1() == 1);
        REQUIRE(tile.getCS_2() == 1);
        REQUIRE(tile.getLevel() == 0);
        REQUIRE(tile.getUREF() == 0);
        REQUIRE(tile.getRREF() == 0);
    }
}

TEST_CASE("Test GSModel will close zip archive when destruct", "[CDBGSModels]")
{
    std::filesystem::path CDBPath = dataPath / "GSModelsWithGTModelTexture";
    std::filesystem::path input = CDBPath / "Tiles" / "N32" / "W118" / "100_GSFeature" / "L00" / "U0"
                                  / "N32W118_D100_S001_T001_L00_U0_R0.dbf";

    // read in the GSFeature data
    GDALDatasetUniquePtr attributesDataset = GDALDatasetUniquePtr(
        (GDALDataset *) GDALOpenEx(input.c_str(), GDAL_OF_VECTOR, nullptr, nullptr, nullptr));
    REQUIRE(attributesDataset != nullptr);

    auto GSFeatureTile = CDBTile::createFromFile(input.filename().string());
    CDBModelsAttributes modelsAttributes(std::move(attributesDataset), *GSFeatureTile, CDBPath);

    // find GSModel archive
    auto attributeTile = modelsAttributes.getTile();
    CDBTile modelTile(attributeTile.getGeoCell(),
                      CDBDataset::GSModelGeometry,
                      1,
                      1,
                      attributeTile.getLevel(),
                      attributeTile.getUREF(),
                      attributeTile.getRREF());

    std::filesystem::path GSModelZip = CDBPath / (modelTile.getRelativePath().string() + ".zip");
    REQUIRE(std::filesystem::exists(GSModelZip));

    osgDB::ReaderWriter *rw = osgDB::Registry::instance()->getReaderWriterForExtension("zip");
    REQUIRE(rw != nullptr);

    // set relative path for GSModel
    osg::ref_ptr<osgDB::Options> options = new osgDB::Options();
    options->setObjectCacheHint(osgDB::Options::CACHE_NONE);
    options->getDatabasePathList().push_front(GSModelZip.parent_path());

    // read GSModel geometry
    osgDB::ReaderWriter::ReadResult GSModelRead = rw->openArchive(GSModelZip, osgDB::Archive::READ);
    REQUIRE(GSModelRead.validArchive());

    osg::ref_ptr<osgDB::Archive> archive = GSModelRead.takeArchive();

    // The function getFileNames will return true if archive is opened
    std::vector<std::string> fileNames;
    REQUIRE(archive->getFileNames(fileNames) == true);

    {
        auto models = CDBGSModels(std::move(modelsAttributes), modelTile, archive, options);
    }

    // This function will return false if archive is closed;
    REQUIRE(archive->getFileNames(fileNames) == false);
}

TEST_CASE("Test converting GSModel to tileset.json", "[CDBGSModels]")
{
    std::filesystem::path CDBPath = dataPath / "GSModelsWithGTModelTexture";
    std::filesystem::path output = "GSModelsWithGTModelTexture";
    Converter converter(CDBPath, output);
    converter.convert();

    // make sure every zip file in GSModelGeometry will have a corresponding b3dm in the output
    size_t geometryModelCount = 0;
    std::filesystem::path GSModelGeometryInput = CDBPath / "Tiles" / "N32" / "W118" / "300_GSModelGeometry";
    std::filesystem::path tilesetPath = output / "Tiles" / "N32" / "W118" / "GSModels" / "1_1";
    for (std::filesystem::directory_entry levelDir :
         std::filesystem::directory_iterator(GSModelGeometryInput)) {
        for (std::filesystem::directory_entry UREFDir : std::filesystem::directory_iterator(levelDir)) {
            for (std::filesystem::directory_entry tilePath : std::filesystem::directory_iterator(UREFDir)) {
                auto GSModelGeometryTile = CDBTile::createFromFile(tilePath.path().stem());
                REQUIRE(std::filesystem::exists(
                    tilesetPath / (GSModelGeometryTile->getRelativePath().stem().string() + ".b3dm")));
                ++geometryModelCount;
            }
        }
    }

    REQUIRE(geometryModelCount == 3);

    // check the tileset
    std::ifstream verifiedJS(CDBPath / "VerifiedTileset.json");
    std::ifstream testJS(tilesetPath / "N32W118_D300_S001_T001.json");
    nlohmann::json verifiedJson = nlohmann::json::parse(verifiedJS);
    nlohmann::json testJson = nlohmann::json::parse(testJS);
    REQUIRE(testJson == verifiedJson);

    // remove the test output
    std::filesystem::remove_all(output);
}

TEST_CASE("Test converting GSModel using 3D Tiles Next", "[CDBGSModels]")
{
    std::filesystem::path CDBPath = dataPath / "GSModelsWithGTModelTexture";
    std::filesystem::path output = "GSModelsWithGTModelTexture";
    Converter converter(CDBPath, output);
    converter.setUse3dTilesNext(true);
    converter.convert();

    // make sure every zip file in GSModelGeometry will have a corresponding b3dm in the output
    size_t geometryModelCount = 0;
    std::filesystem::path GSModelGeometryInput = CDBPath / "Tiles" / "N32" / "W118" / "300_GSModelGeometry";
    std::filesystem::path tilesetPath = output / "Tiles" / "N32" / "W118" / "GSModels" / "1_1";
    for (std::filesystem::directory_entry levelDir :
         std::filesystem::directory_iterator(GSModelGeometryInput)) {
        for (std::filesystem::directory_entry UREFDir : std::filesystem::directory_iterator(levelDir)) {
            for (std::filesystem::directory_entry tilePath : std::filesystem::directory_iterator(UREFDir)) {
                auto GSModelGeometryTile = CDBTile::createFromFile(tilePath.path().stem());
                REQUIRE(std::filesystem::exists(
                    tilesetPath / (GSModelGeometryTile->getRelativePath().stem().string() + ".glb")));
                ++geometryModelCount;
            }
        }
    }

    REQUIRE(geometryModelCount == 3);

    // remove the test output
    std::filesystem::remove_all(output);
}
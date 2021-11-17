#include "CDBElevation.h"
#include "CDBTo3DTiles.h"
#include "Config.h"
#include "TileFormatIO.h"
#include <doctest/doctest.h>
#include "nlohmann/json.hpp"
#include "tiny_gltf.h"
#include <fstream>

using namespace CDBTo3DTiles;

static void checkAllConvertedImagery(const std::filesystem::path &imageryPath,
                                     const std::filesystem::path &imageryOutputPath,
                                     size_t expectedImageryCount)
{
    size_t textureCount = 0;
    for (std::filesystem::directory_entry levelDir : std::filesystem::directory_iterator(imageryPath)) {
        for (std::filesystem::directory_entry UREFDir : std::filesystem::directory_iterator(levelDir)) {
            for (std::filesystem::directory_entry tilePath : std::filesystem::directory_iterator(UREFDir)) {
                auto tileFilename = tilePath.path();
                if (tileFilename.extension() != ".jp2") {
                    continue;
                }

                auto texture = imageryOutputPath / (tileFilename.stem().string() + ".jpeg");
                CHECK(std::filesystem::exists(texture));
                ++textureCount;
            }
        }
    }

    CHECK(textureCount == expectedImageryCount);
}

static void checkElevationDuplicated(const std::filesystem::path &imageryPath,
                                     const std::filesystem::path &elevationPath,
                                     size_t expectedElevationCount)
{
    size_t elevationCount = 0;
    for (std::filesystem::directory_entry levelDir : std::filesystem::directory_iterator(imageryPath)) {
        for (std::filesystem::directory_entry UREFDir : std::filesystem::directory_iterator(levelDir)) {
            for (std::filesystem::directory_entry tilePath : std::filesystem::directory_iterator(UREFDir)) {
                if (tilePath.path().extension() != ".jp2") {
                    continue;
                }

                auto imageryTile = CDBTile::createFromFile(tilePath.path().stem());
                auto elevationTile = CDBTile(imageryTile->getGeoCell(),
                                             CDBDataset::Elevation,
                                             1,
                                             1,
                                             imageryTile->getLevel(),
                                             imageryTile->getUREF(),
                                             imageryTile->getRREF());
                CHECK(std::filesystem::exists(
                    elevationPath / (elevationTile.getRelativePathWithNonZeroPaddedLevel().stem().string() + ".b3dm")));

                ++elevationCount;
            }
        }
    }

    CHECK(elevationCount == expectedElevationCount);
}

static void checkUVTheSameAsOldElevation(const Mesh &subregionMesh,
                                         const Mesh &oldElevation,
                                         size_t gridWidth,
                                         glm::uvec2 gridFrom,
                                         glm::uvec2 gridTo)
{
    int UVidx = 0;
    for (uint32_t y = gridFrom.y; y < gridTo.y + 1; ++y) {
        for (uint32_t x = gridFrom.x; x < gridTo.x + 1; ++x) {
            size_t oldIdx = y * (gridWidth + 1) + x;
            CHECK(subregionMesh.UVs[UVidx].x == doctest::Approx(oldElevation.UVs[oldIdx].x));
            CHECK(subregionMesh.UVs[UVidx].y == doctest::Approx(oldElevation.UVs[oldIdx].y));
            ++UVidx;
        }
    }
}

static void checkUVReindexForSubRegion(const Mesh &subregionMesh, size_t gridWidth, size_t gridHeight)
{
    // check that UV is properly re-index from 0,0
    int UVidx = 0;
    for (size_t i = 0; i < gridHeight + 1; ++i) {
        for (size_t j = 0; j < gridWidth + 1; ++j) {
            CHECK(subregionMesh.UVs[UVidx].x
                    == doctest::Approx(static_cast<float>(j) / static_cast<float>(gridWidth)));
            CHECK(subregionMesh.UVs[UVidx].y
                    == doctest::Approx(static_cast<float>(i) / static_cast<float>(gridHeight)));
            ++UVidx;
        }
    }
}

TEST_SUITE_BEGIN("CDBElevation");

TEST_SUITE("Test create elevation from file")
{
    TEST_CASE("Create from valid GeoTiff file")
    {
        auto elevation = CDBElevation::createFromFile(dataPath / "Elevation"
                                                      / "N34W119_D001_S001_T001_LC06_U0_R0.tif");
        CHECK(elevation != std::nullopt);
        CHECK(elevation->getGridWidth() == 16);
        CHECK(elevation->getGridHeight() == 16);

        // Level -6 has 16x16 raster but we extends to the edge to cover crack,
        // so total of vertices are 17x17 vertices
        const auto &mesh = elevation->getUniformGridMesh();
        CHECK(mesh.indices.size() == 16 * 16 * 6);
        CHECK(mesh.positions.size() == 289);
        CHECK(mesh.positionRTCs.size() == 289);
        CHECK(mesh.UVs.size() == 289);
        CHECK(mesh.normals.size() == 0);

        // Check tile
        const auto &cdbTile = elevation->getTile();
        CHECK(cdbTile.getGeoCell() == CDBGeoCell(34, -119));
        CHECK(cdbTile.getDataset() == CDBDataset::Elevation);
        CHECK(cdbTile.getCS_1() == 1);
        CHECK(cdbTile.getCS_2() == 1);
        CHECK(cdbTile.getLevel() == -6);
        CHECK(cdbTile.getUREF() == 0);
        CHECK(cdbTile.getRREF() == 0);
    }

    TEST_CASE("Create from invalid name file")
    {
        auto elevation = CDBElevation::createFromFile(dataPath / "Elevation" / "InvalidName.tif");
        CHECK(elevation == std::nullopt);
    }

    TEST_CASE("Create from valid name file but wrong extension")
    {
        auto elevation = CDBElevation::createFromFile(dataPath / "Elevation"
                                                      / "N34W119_D001_S001_T001_LC06_U0_R0.png");
        CHECK(elevation == std::nullopt);
    }

    TEST_CASE("Create from valid name file but wrong component selectors")
    {
        auto elevation = CDBElevation::createFromFile(dataPath / "Elevation"
                                                      / "N34W119_D001_S001_T002_LC06_U0_R0.tif");
        CHECK(elevation == std::nullopt);
    }

    TEST_CASE("Create from valid name file but invalid content")
    {
        auto elevation = CDBElevation::createFromFile(dataPath / "Elevation"
                                                      / "N34W119_D001_S001_T001_L06_U0_R0.tif");
        CHECK(elevation == std::nullopt);
    }
}

TEST_SUITE("Test create sub region of an elevation")
{
    // 16x16 mesh
    auto elevation = CDBElevation::createFromFile(dataPath / "Elevation"
                                                  / "N34W119_D001_S001_T001_LC06_U0_R0.tif");
    
    TEST_CASE("Create North West")
    {
        {
            // quarter of the grid is 8x8, but we extend to the edge to cover crack, so
            // actual vertices are 9x9 vertices
            auto NW = elevation->createNorthWestSubRegion(false);
            CHECK(NW != std::nullopt);
            CHECK(NW->getGridWidth() == 8);
            CHECK(NW->getGridHeight() == 8);

            const auto &mesh = NW->getUniformGridMesh();
            CHECK(mesh.indices.size() == 8 * 8 * 6);
            CHECK(mesh.positions.size() == 81);
            CHECK(mesh.positionRTCs.size() == 81);
            CHECK(mesh.UVs.size() == 81);

            // check that UV is still the same as the old elevation
            glm::uvec2 gridFrom(0, 0);
            glm::uvec2 gridTo(elevation->getGridWidth() / 2, elevation->getGridHeight() / 2);
            checkUVTheSameAsOldElevation(mesh,
                                         elevation->getUniformGridMesh(),
                                         elevation->getGridWidth(),
                                         gridFrom,
                                         gridTo);
        }

        {
            // quarter of the grid is 8x8, but we extend to the edge to cover crack, so
            // actual vertices are 9x9 vertices
            auto NW = elevation->createNorthWestSubRegion(true);
            CHECK(NW != std::nullopt);
            CHECK(NW->getGridWidth() == 8);
            CHECK(NW->getGridHeight() == 8);

            const auto &mesh = NW->getUniformGridMesh();
            CHECK(mesh.indices.size() == 8 * 8 * 6);
            CHECK(mesh.positions.size() == 81);
            CHECK(mesh.positionRTCs.size() == 81);
            CHECK(mesh.UVs.size() == 81);

            // check that UV is properly re-index from 0,0
            checkUVReindexForSubRegion(mesh, NW->getGridWidth(), NW->getGridHeight());
        }
    }

    TEST_CASE("Create North East")
    {
        {
            // quarter of the grid is 8x8, but we extend to the edge to cover crack, so
            // actual vertices are 9x9 vertices
            auto NE = elevation->createNorthEastSubRegion(false);
            CHECK(NE != std::nullopt);
            CHECK(NE->getGridWidth() == 8);
            CHECK(NE->getGridHeight() == 8);

            const auto &mesh = NE->getUniformGridMesh();
            CHECK(mesh.indices.size() == 8 * 8 * 6);
            CHECK(mesh.positions.size() == 81);
            CHECK(mesh.positionRTCs.size() == 81);
            CHECK(mesh.UVs.size() == 81);

            // check that UV is still the same as the old elevation
            glm::uvec2 gridFrom(elevation->getGridWidth() / 2, 0);
            glm::uvec2 gridTo = gridFrom
                                + glm::uvec2(elevation->getGridWidth() / 2, elevation->getGridHeight() / 2);
            checkUVTheSameAsOldElevation(mesh,
                                         elevation->getUniformGridMesh(),
                                         elevation->getGridWidth(),
                                         gridFrom,
                                         gridTo);
        }

        {
            // quarter of the grid is 8x8, but we extend to the edge to cover crack, so
            // actual vertices are 9x9 vertices
            auto NE = elevation->createNorthEastSubRegion(true);
            CHECK(NE != std::nullopt);
            CHECK(NE->getGridWidth() == 8);
            CHECK(NE->getGridHeight() == 8);

            const auto &mesh = NE->getUniformGridMesh();
            CHECK(mesh.indices.size() == 8 * 8 * 6);
            CHECK(mesh.positions.size() == 81);
            CHECK(mesh.positionRTCs.size() == 81);
            CHECK(mesh.UVs.size() == 81);

            // check that UV is properly re-index from 0,0
            checkUVReindexForSubRegion(mesh, NE->getGridWidth(), NE->getGridHeight());
        }
    }

    TEST_CASE("Create South West")
    {
        {
            // quarter of the grid is 8x8, but we extend to the edge to cover crack, so
            // actual vertices are 9x9 vertices
            auto SW = elevation->createSouthWestSubRegion(false);
            CHECK(SW != std::nullopt);
            CHECK(SW->getGridWidth() == 8);
            CHECK(SW->getGridHeight() == 8);

            const auto &mesh = SW->getUniformGridMesh();
            CHECK(mesh.indices.size() == 8 * 8 * 6);
            CHECK(mesh.positions.size() == 81);
            CHECK(mesh.positionRTCs.size() == 81);
            CHECK(mesh.UVs.size() == 81);

            // check that UV is still the same as the old elevation
            glm::uvec2 gridFrom(0, elevation->getGridHeight() / 2);
            glm::uvec2 gridTo = gridFrom
                                + glm::uvec2(elevation->getGridWidth() / 2, elevation->getGridHeight() / 2);
            checkUVTheSameAsOldElevation(mesh,
                                         elevation->getUniformGridMesh(),
                                         elevation->getGridWidth(),
                                         gridFrom,
                                         gridTo);
        }

        {
            // quarter of the grid is 8x8, but we extend to the edge to cover crack, so
            // actual vertices are 9x9 vertices
            auto SW = elevation->createSouthWestSubRegion(true);
            CHECK(SW != std::nullopt);
            CHECK(SW->getGridWidth() == 8);
            CHECK(SW->getGridHeight() == 8);

            const auto &mesh = SW->getUniformGridMesh();
            CHECK(mesh.indices.size() == 8 * 8 * 6);
            CHECK(mesh.positions.size() == 81);
            CHECK(mesh.positionRTCs.size() == 81);
            CHECK(mesh.UVs.size() == 81);

            // check that UV is properly re-index from 0,0
            checkUVReindexForSubRegion(mesh, SW->getGridWidth(), SW->getGridHeight());
        }
    }

    TEST_CASE("Create South East")
    {
        {
            // quarter of the grid is 8x8, but we extend to the edge to cover crack, so
            // actual vertices are 9x9 vertices
            auto SE = elevation->createSouthEastSubRegion(false);
            CHECK(SE != std::nullopt);
            CHECK(SE->getGridWidth() == 8);
            CHECK(SE->getGridHeight() == 8);

            const auto &mesh = SE->getUniformGridMesh();
            CHECK(mesh.indices.size() == 8 * 8 * 6);
            CHECK(mesh.positions.size() == 81);
            CHECK(mesh.positionRTCs.size() == 81);
            CHECK(mesh.UVs.size() == 81);

            // check that UV is still the same as the old elevation
            glm::uvec2 gridFrom(elevation->getGridWidth() / 2, elevation->getGridHeight() / 2);
            glm::uvec2 gridTo = glm::uvec2(elevation->getGridWidth(), elevation->getGridHeight());
            checkUVTheSameAsOldElevation(mesh,
                                         elevation->getUniformGridMesh(),
                                         elevation->getGridWidth(),
                                         gridFrom,
                                         gridTo);
        }

        {
            // quarter of the grid is 8x8, but we extend to the edge to cover crack, so
            // actual vertices are 9x9 vertices
            auto SE = elevation->createSouthEastSubRegion(true);
            CHECK(SE != std::nullopt);
            CHECK(SE->getGridWidth() == 8);
            CHECK(SE->getGridHeight() == 8);

            const auto &mesh = SE->getUniformGridMesh();
            CHECK(mesh.indices.size() == 8 * 8 * 6);
            CHECK(mesh.positions.size() == 81);
            CHECK(mesh.positionRTCs.size() == 81);
            CHECK(mesh.UVs.size() == 81);

            // check that UV is properly re-index from 0,0
            checkUVReindexForSubRegion(mesh, SE->getGridWidth(), SE->getGridHeight());
        }
    }
}

TEST_SUITE_END();
TEST_SUITE_BEGIN("CDBElevationConversion");

TEST_SUITE("Test conversion when elevation has more LOD than imagery")
{
    TEST_CASE("Imagery has only negative LOD")
    {
        std::filesystem::path input = dataPath / "ElevationMoreLODNegativeImagery";
        std::filesystem::path output = "ElevationMoreLODNegativeImagery";
        std::filesystem::path elevationOutputDir = output / "Tiles" / "N32" / "W118" / "Elevation" / "1_1";

        Converter converter(input, output);
        converter.convert();

        // check that all the imagery created as textures
        std::filesystem::path textureOutputDir = elevationOutputDir / "Textures";
        CHECK(std::filesystem::exists(textureOutputDir));
        checkAllConvertedImagery(input / "Tiles" / "N32" / "W118" / "004_Imagery", textureOutputDir, 10);

        // verified tileset.json
        std::filesystem::path tilesetPath = elevationOutputDir / "N32W118_D001_S001_T001.json";
        CHECK(std::filesystem::exists(tilesetPath));

        std::ifstream verifiedJS(input / "VerifiedTileset.json");
        nlohmann::json verifiedJson = nlohmann::json::parse(verifiedJS);

        std::ifstream testJS(tilesetPath);
        nlohmann::json testJson = nlohmann::json::parse(testJS);

        CHECK(testJson == verifiedJson);

        // remove the test output
        std::filesystem::remove_all(output);
    }

    TEST_CASE("Imagery has positive LOD")
    {
        std::filesystem::path input = dataPath / "ElevationMoreLODPositiveImagery";
        std::filesystem::path output = "ElevationMoreLODPositiveImagery";
        std::filesystem::path elevationOutputDir = output / "Tiles" / "N32" / "W118" / "Elevation" / "1_1";

        Converter converter(input, output);
        converter.convert();

        // check all the imagery are created
        std::filesystem::path textureOutputDir = elevationOutputDir / "Textures";
        CHECK(std::filesystem::exists(textureOutputDir));
        checkAllConvertedImagery(input / "Tiles" / "N32" / "W118" / "004_Imagery", textureOutputDir, 13);

        // verified tileset.json
        std::filesystem::path tilesetPath = elevationOutputDir / "N32W118_D001_S001_T001.json";
        CHECK(std::filesystem::exists(tilesetPath));

        std::ifstream verifiedJS(input / "VerifiedTileset.json");
        nlohmann::json verifiedJson = nlohmann::json::parse(verifiedJS);

        std::ifstream testJS(tilesetPath);
        nlohmann::json testJson = nlohmann::json::parse(testJS);

        CHECK(testJson == verifiedJson);

        // remove the test output
        std::filesystem::remove_all(output);
    }
}

TEST_SUITE("Test conversion when imagery has more LOD than elevation")
{
    TEST_CASE("Test conversion when elevation has negative LOD only")
    {
        std::filesystem::path input = dataPath / "ImageryMoreLODNegativeElevation";
        std::filesystem::path output = "ImageryMoreLODNegativeElevation";
        std::filesystem::path elevationOutputDir = output / "Tiles" / "N32" / "W118" / "Elevation" / "1_1";

        Converter converter(input, output);
        converter.convert();

        // check all the imagery are created.
        // check that elevation is duplicated to levels where imagery exists
        std::filesystem::path imageryInput = input / "Tiles" / "N32" / "W118" / "004_Imagery";
        std::filesystem::path textureOutputDir = elevationOutputDir / "Textures";
        CHECK(std::filesystem::exists(textureOutputDir));
        checkAllConvertedImagery(imageryInput, textureOutputDir, 13);
        checkElevationDuplicated(imageryInput, elevationOutputDir, 13);

        // verified tileset.json
        std::filesystem::path tilesetPath = elevationOutputDir / "N32W118_D001_S001_T001.json";
        CHECK(std::filesystem::exists(tilesetPath));

        std::ifstream verifiedJS(input / "VerifiedTileset.json");
        nlohmann::json verifiedJson = nlohmann::json::parse(verifiedJS);

        std::ifstream testJS(tilesetPath);
        nlohmann::json testJson = nlohmann::json::parse(testJS);

        CHECK(testJson == verifiedJson);

        // remove the test output
        std::filesystem::remove_all(output);
    }

    TEST_CASE("Test conversion when elevation has positive LOD")
    {
        std::filesystem::path input = dataPath / "ImageryMoreLODPositiveElevation";
        std::filesystem::path output = "ImageryMoreLODPositiveElevation";
        std::filesystem::path elevationOutputDir = output / "Tiles" / "N32" / "W118" / "Elevation" / "1_1";

        Converter converter(input, output);
        converter.convert();

        // check all the imagery are created.
        // check that elevation is duplicated to levels where imagery exists
        std::filesystem::path imageryInput = input / "Tiles" / "N32" / "W118" / "004_Imagery";
        std::filesystem::path textureOutputDir = elevationOutputDir / "Textures";
        CHECK(std::filesystem::exists(textureOutputDir));
        checkAllConvertedImagery(imageryInput, textureOutputDir, 18);
        checkElevationDuplicated(imageryInput, elevationOutputDir, 18);

        // verified tileset.json
        std::filesystem::path tilesetPath = elevationOutputDir / "N32W118_D001_S001_T001.json";
        CHECK(std::filesystem::exists(tilesetPath));

        std::ifstream verifiedJS(input / "VerifiedTileset.json");
        nlohmann::json verifiedJson = nlohmann::json::parse(verifiedJS);

        std::ifstream testJS(tilesetPath);
        nlohmann::json testJson = nlohmann::json::parse(testJS);

        CHECK(testJson == verifiedJson);

        // remove the test output
        std::filesystem::remove_all(output);
    }
}

TEST_CASE("Test conversion using elevation LOD only")
{
    std::filesystem::path input = dataPath / "ImageryMoreLODPositiveElevation";
    std::filesystem::path output = "ImageryMoreLODPositiveElevation";
    std::filesystem::path elevationOutputDir = output / "Tiles" / "N32" / "W118" / "Elevation" / "1_1";

    Converter converter(input, output);
    converter.setElevationLODOnly(true);
    converter.convert();

    // elevation max level is 1, so we check that
    int maxLevel = -10;
    for (std::filesystem::directory_entry entry : std::filesystem::directory_iterator(elevationOutputDir)) {
        auto tile = CDBTile::createFromFile(entry.path().filename());
        if (tile) {
            maxLevel = glm::max(tile->getLevel(), maxLevel);
        }
    }

    CHECK(maxLevel == 1);

    // remove the test output
    std::filesystem::remove_all(output);
}

TEST_CASE("Test that elevation conversion uses uniform grid mesh instead of simplified mesh if simplified "
          "mesh is empty")
{
    float decimateError = 0.5f;
    float thresholdIndices = 0.02f;

    std::filesystem::path input = dataPath / "ImageryMoreLODPositiveElevation";
    std::filesystem::path output = "ImageryMoreLODPositiveElevation";
    std::filesystem::path elevationOutputDir = output / "Tiles" / "N32" / "W118" / "Elevation" / "1_1";

    // confirm that the elevation at LC09 is empty after decimation
    std::filesystem::path LC09Path = input / "Tiles" / "N32" / "W118" / "001_Elevation" / "LC" / "U0"
                                     / "N32W118_D001_S001_T001_LC09_U0_R0.tif";
    auto elevation = CDBElevation::createFromFile(LC09Path);
    CHECK(elevation != std::nullopt);
    size_t targetIndices = static_cast<size_t>(
        thresholdIndices * static_cast<float>(elevation->getUniformGridMesh().indices.size()));
    auto simplied = elevation->createSimplifiedMesh(targetIndices, decimateError);
    CHECK(simplied.indices.size() == 0);
    CHECK(simplied.positionRTCs.size() == 0);
    CHECK(simplied.normals.size() == 0);
    CHECK(simplied.UVs.size() == 0);

    // convert the CDB
    Converter converter(input, output);
    converter.setElevationDecimateError(decimateError);
    converter.setElevationThresholdIndices(thresholdIndices);
    converter.setElevationLODOnly(true);
    converter.convert();

    // check that LC09 is using uniform grid
    std::ifstream fs(elevationOutputDir / "N32W118_D001_S001_T001_LC9_U0_R0.b3dm", std::ios::binary);
    B3dmHeader b3dm;
    fs.read(reinterpret_cast<char *>(&b3dm), sizeof(b3dm));

    size_t glbBegin = sizeof(b3dm) + b3dm.featureTableJsonByteLength + b3dm.featureTableBinByteLength
                      + b3dm.batchTableJsonByteLength + b3dm.batchTableBinByteLength;
    std::vector<unsigned char> glb(b3dm.byteLength - glbBegin);

    fs.seekg(glbBegin);
    fs.read(reinterpret_cast<char *>(glb.data()), glb.size());

    tinygltf::TinyGLTF loader;
    tinygltf::Model model;
    std::string err;
    std::string warn;
    loader.LoadBinaryFromMemory(&model,
                                &err,
                                &warn,
                                glb.data(),
                                static_cast<unsigned int>(glb.size()),
                                elevationOutputDir);

    CHECK(model.meshes.size() == 1);

    // check mesh primitive
    const auto &gltfMesh = model.meshes.front();
    CHECK(gltfMesh.primitives.size() == 1);

    const auto &gltfPrimitive = gltfMesh.primitives.front();
    CHECK(gltfPrimitive.mode == TINYGLTF_MODE_TRIANGLES);
    CHECK(gltfPrimitive.indices == 0);
    CHECK(gltfPrimitive.attributes.at("POSITION") == 1);
    CHECK(gltfPrimitive.attributes.at("TEXCOORD_0") == 2);

    // check accessors
    const auto &uniformElevation = elevation->getUniformGridMesh();
    const auto &indicesAccessor = model.accessors[gltfPrimitive.indices];
    CHECK(indicesAccessor.count == uniformElevation.indices.size());
    CHECK(indicesAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT);

    const auto &positionAccessor = model.accessors[gltfPrimitive.attributes.at("POSITION")];
    CHECK(positionAccessor.count == uniformElevation.positionRTCs.size());
    CHECK(positionAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
    CHECK(positionAccessor.type == TINYGLTF_TYPE_VEC3);

    const auto &texCoordAccessor = model.accessors[gltfPrimitive.attributes.at("TEXCOORD_0")];
    CHECK(texCoordAccessor.count == uniformElevation.UVs.size());
    CHECK(texCoordAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
    CHECK(texCoordAccessor.type == TINYGLTF_TYPE_VEC2);

    // check buffer view
    const auto &gltfBufferData = model.buffers.front().data;

    const auto &indicesBufferView = model.bufferViews[indicesAccessor.bufferView];
    size_t index = 0;
    for (size_t i = indicesBufferView.byteOffset; i < indicesBufferView.byteLength;
         i += sizeof(unsigned int)) {
        unsigned gltfIndex = 0;
        std::memcpy(&gltfIndex, &gltfBufferData[i], sizeof(unsigned int));
        CHECK(gltfIndex == uniformElevation.indices[index]);
        ++index;
    }

    const auto &positionBufferView = model.bufferViews[positionAccessor.bufferView];
    index = 0;
    for (size_t i = positionBufferView.byteOffset; i < positionBufferView.byteLength; i += sizeof(glm::vec3)) {
        glm::vec3 gltfPosition;
        std::memcpy(&gltfPosition, &gltfBufferData[i], sizeof(glm::vec3));
        CHECK(gltfPosition.x == doctest::Approx(uniformElevation.positionRTCs[index].x));
        CHECK(gltfPosition.y == doctest::Approx(uniformElevation.positionRTCs[index].y));
        CHECK(gltfPosition.z == doctest::Approx(uniformElevation.positionRTCs[index].z));
        ++index;
    }

    const auto &texCoordBufferView = model.bufferViews[texCoordAccessor.bufferView];
    index = 0;
    for (size_t i = texCoordBufferView.byteOffset; i < texCoordBufferView.byteLength; i += sizeof(glm::vec2)) {
        glm::vec2 gltfTexCoord;
        std::memcpy(&gltfTexCoord, &gltfBufferData[i], sizeof(glm::vec2));
        CHECK(gltfTexCoord.x == doctest::Approx(uniformElevation.UVs[index].x));
        CHECK(gltfTexCoord.y == doctest::Approx(uniformElevation.UVs[index].y));
        ++index;
    }

    // remove the test output
    std::filesystem::remove_all(output);
}

TEST_SUITE_END();
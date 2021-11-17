#include "CDB.h"
#include "CDBElevation.h"
#include "CDBGeoCell.h"
#include "CDBTile.h"
#include "CDBTo3DTiles.h"
#include "Config.h"
#include <doctest/doctest.h>
#include "glm/glm.hpp"
#include "libmorton/morton.h"
#include "nlohmann/json.hpp"
#include <fstream>

using namespace CDBTo3DTiles;

TEST_SUITE_BEGIN("CombineTilesets");

TEST_CASE("Test invalid combined dataset")
{
    std::filesystem::path input = dataPath / "CombineTilesets";
    std::filesystem::path output = "CombineTilesets";

    Converter converter(input, output);
    CHECK_THROWS_WITH(converter.combineDataset({"Elevation_1_1", "SASS_2_2", "HydrographyNetwork_2_2"}),
                        "Unrecognize dataset: SASS\n"
                        "Correct dataset names are: \n"
                        "GTModels\n"
                        "HydrographyNetwork\n"
                        "GSModels\n"
                        "PowerlineNetwork\n"
                        "RailRoadNetwork\n"
                        "RoadNetwork\n"
                        "Elevation\n");

    CHECK_THROWS_WITH(converter.combineDataset({"Elevation_as_1", "GTModels_1_1"}),
                        "Component selector 1 has to be a number");
    CHECK_THROWS_WITH(converter.combineDataset({"Elevation_1_as", "GTModels_1_1"}),
                        "Component selector 2 has to be a number");
}

TEST_CASE("Test converter combines all the tilesets available in the GeoCells")
{
    std::filesystem::path input = dataPath / "CombineTilesets";
    std::filesystem::path output = "CombineTilesets";

    Converter converter(input, output);
    converter.convert();

    {
        // check the combined elevation tileset in geocell N32 W119
        std::filesystem::path elevationOutput = output / "Elevation_1_1.json";
        CHECK(std::filesystem::exists(elevationOutput));
        std::ifstream elevationFs(elevationOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(elevationFs);

        auto geoCell = CDBGeoCell(32, -119);
        auto boundRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
        const auto &boundRectangle = boundRegion.getRectangle();
        double boundNorth = boundRectangle.getNorth();
        double boundWest = boundRectangle.getWest();
        double boundSouth = boundRectangle.getSouth();
        double boundEast = boundRectangle.getEast();
        CHECK(tilesetJson["geometricError"] == doctest::Approx(300000.0f));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        CHECK(child["content"]["uri"]
                == (geoCell.getRelativePath() / "Elevation" / "1_1" / "N32W119_D001_S001_T001.json").string());
        CHECK(child["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(child["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(child["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(child["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));
    }

    {
        // check GTModels
        std::filesystem::path tilesetOutput = output / "GTModels_2_1.json";
        CHECK(std::filesystem::exists(tilesetOutput));
        std::ifstream fs(tilesetOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(fs);

        auto geoCell = CDBGeoCell(32, -118);
        auto boundRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
        const auto &boundRectangle = boundRegion.getRectangle();
        double boundNorth = boundRectangle.getNorth();
        double boundWest = boundRectangle.getWest();
        double boundSouth = boundRectangle.getSouth();
        double boundEast = boundRectangle.getEast();
        CHECK(tilesetJson["geometricError"] == doctest::Approx(300000.0f));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        CHECK(child["content"]["uri"]
                == (geoCell.getRelativePath() / "GTModels" / "2_1" / "N32W118_D101_S002_T001.json").string());
        CHECK(child["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(child["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(child["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(child["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));
    }

    {
        // check GTModels
        std::filesystem::path tilesetOutput = output / "GTModels_1_1.json";
        CHECK(std::filesystem::exists(tilesetOutput));
        std::ifstream fs(tilesetOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(fs);

        auto geoCell = CDBGeoCell(32, -118);
        auto boundRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
        const auto &boundRectangle = boundRegion.getRectangle();
        double boundNorth = boundRectangle.getNorth();
        double boundWest = boundRectangle.getWest();
        double boundSouth = boundRectangle.getSouth();
        double boundEast = boundRectangle.getEast();
        CHECK(tilesetJson["geometricError"] == doctest::Approx(300000.0f));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        CHECK(child["content"]["uri"]
                == (geoCell.getRelativePath() / "GTModels" / "1_1" / "N32W118_D101_S001_T001.json").string());
        CHECK(child["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(child["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(child["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(child["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));
    }

    {
        // check RoadNetwork
        std::filesystem::path tilesetOutput = output / "RoadNetwork_2_3.json";
        CHECK(std::filesystem::exists(tilesetOutput));
        std::ifstream fs(tilesetOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(fs);

        auto geoCell = CDBGeoCell(32, -118);
        auto boundRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
        const auto &boundRectangle = boundRegion.getRectangle();
        double boundNorth = boundRectangle.getNorth();
        double boundWest = boundRectangle.getWest();
        double boundSouth = boundRectangle.getSouth();
        double boundEast = boundRectangle.getEast();
        CHECK(tilesetJson["geometricError"] == doctest::Approx(300000.0f));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        CHECK(
            child["content"]["uri"]
            == (geoCell.getRelativePath() / "RoadNetwork" / "2_3" / "N32W118_D201_S002_T003.json").string());
        CHECK(child["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(child["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(child["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(child["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));
    }

    std::filesystem::remove_all(output);
}

TEST_CASE("Test converer combine one set of requested datasets")
{
    std::filesystem::path input = dataPath / "CombineTilesets";
    std::filesystem::path output = "CombineTilesets";

    Converter converter(input, output);
    converter.combineDataset({"Elevation_1_1"});
    converter.combineDataset({"Elevation_1_1", "RoadNetwork_2_3", "GTModels_1_1"});
    converter.convert();

    SUBCASE("Test the converter doesn't combine only one requested dataset since it is already processed by "
            "default")
    {
        // check the combined elevation tileset in geocell N32 W119
        std::filesystem::path elevationOutput = output / "Elevation_1_1.json";
        CHECK(std::filesystem::exists(elevationOutput));
        std::ifstream elevationFs(elevationOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(elevationFs);

        auto geoCell = CDBGeoCell(32, -119);
        auto boundRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
        const auto &boundRectangle = boundRegion.getRectangle();
        double boundNorth = boundRectangle.getNorth();
        double boundWest = boundRectangle.getWest();
        double boundSouth = boundRectangle.getSouth();
        double boundEast = boundRectangle.getEast();
        CHECK(tilesetJson["geometricError"] == doctest::Approx(300000.0f));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        CHECK(child["content"]["uri"]
                == (geoCell.getRelativePath() / "Elevation" / "1_1" / "N32W119_D001_S001_T001.json").string());
        CHECK(child["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(child["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(child["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(child["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));
    }

    SUBCASE("Test multiple dataset combine together")
    {
        std::filesystem::path tilesetOutput = output / "tileset.json";
        CHECK(std::filesystem::exists(tilesetOutput));
        std::ifstream fs(tilesetOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(fs);

        auto N32W119 = CDBGeoCell(32, -119);
        auto N32W119BoundRegion = CDBTile::calcBoundRegion(N32W119, -10, 0, 0);

        auto N32W118 = CDBGeoCell(32, -118);
        auto N32W118BoundRegion = CDBTile::calcBoundRegion(N32W118, -10, 0, 0);

        auto unionBoundRegion = N32W118BoundRegion.computeUnion(N32W119BoundRegion);
        auto unionRectangle = unionBoundRegion.getRectangle();
        double boundNorth = unionRectangle.getNorth();
        double boundWest = unionRectangle.getWest();
        double boundSouth = unionRectangle.getSouth();
        double boundEast = unionRectangle.getEast();
        CHECK(tilesetJson["geometricError"] == doctest::Approx(300000.0f));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));
        CHECK(tilesetJson["root"]["children"].size() == 3);

        {
            // elevation
            auto childRectangle = N32W119BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][0];
            CHECK(child["content"]["uri"] == "Elevation_1_1.json");
            CHECK(child["boundingVolume"]["region"][0] == doctest::Approx(childBoundWest));
            CHECK(child["boundingVolume"]["region"][1] == doctest::Approx(childBoundSouth));
            CHECK(child["boundingVolume"]["region"][2] == doctest::Approx(childBoundEast));
            CHECK(child["boundingVolume"]["region"][3] == doctest::Approx(childBoundNorth));
        }

        {
            // RoadNetwork
            auto childRectangle = N32W118BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][1];
            CHECK(child["content"]["uri"] == "RoadNetwork_2_3.json");
            CHECK(child["boundingVolume"]["region"][0] == doctest::Approx(childBoundWest));
            CHECK(child["boundingVolume"]["region"][1] == doctest::Approx(childBoundSouth));
            CHECK(child["boundingVolume"]["region"][2] == doctest::Approx(childBoundEast));
            CHECK(child["boundingVolume"]["region"][3] == doctest::Approx(childBoundNorth));
        }

        {
            // GTModels
            auto childRectangle = N32W118BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][2];
            CHECK(child["content"]["uri"] == "GTModels_1_1.json");
            CHECK(child["boundingVolume"]["region"][0] == doctest::Approx(childBoundWest));
            CHECK(child["boundingVolume"]["region"][1] == doctest::Approx(childBoundSouth));
            CHECK(child["boundingVolume"]["region"][2] == doctest::Approx(childBoundEast));
            CHECK(child["boundingVolume"]["region"][3] == doctest::Approx(childBoundNorth));
        }
    }

    std::filesystem::remove_all(output);
}

TEST_CASE("Test combine multiple sets of tilesets")
{
    std::filesystem::path input = dataPath / "CombineTilesets";
    std::filesystem::path output = "CombineTilesets";

    Converter converter(input, output);
    converter.combineDataset({"Elevation_1_1", "RoadNetwork_2_3"});
    converter.combineDataset({"Elevation_1_1", "GTModels_1_1"});
    converter.convert();

    {
        std::filesystem::path tilesetOutput = output / "Elevation_1_1GTModels_1_1.json";
        CHECK(std::filesystem::exists(tilesetOutput));
        std::ifstream fs(tilesetOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(fs);

        auto N32W119 = CDBGeoCell(32, -119);
        auto N32W119BoundRegion = CDBTile::calcBoundRegion(N32W119, -10, 0, 0);

        auto N32W118 = CDBGeoCell(32, -118);
        auto N32W118BoundRegion = CDBTile::calcBoundRegion(N32W118, -10, 0, 0);

        auto unionBoundRegion = N32W118BoundRegion.computeUnion(N32W119BoundRegion);
        auto unionRectangle = unionBoundRegion.getRectangle();
        double boundNorth = unionRectangle.getNorth();
        double boundWest = unionRectangle.getWest();
        double boundSouth = unionRectangle.getSouth();
        double boundEast = unionRectangle.getEast();
        CHECK(tilesetJson["geometricError"] == doctest::Approx(300000.0f));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));
        CHECK(tilesetJson["root"]["children"].size() == 2);

        {
            // elevation
            auto childRectangle = N32W119BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][0];
            CHECK(child["content"]["uri"] == "Elevation_1_1.json");
            CHECK(child["boundingVolume"]["region"][0] == doctest::Approx(childBoundWest));
            CHECK(child["boundingVolume"]["region"][1] == doctest::Approx(childBoundSouth));
            CHECK(child["boundingVolume"]["region"][2] == doctest::Approx(childBoundEast));
            CHECK(child["boundingVolume"]["region"][3] == doctest::Approx(childBoundNorth));
        }

        {
            // GTModels
            auto childRectangle = N32W118BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][1];
            CHECK(child["content"]["uri"] == "GTModels_1_1.json");
            CHECK(child["boundingVolume"]["region"][0] == doctest::Approx(childBoundWest));
            CHECK(child["boundingVolume"]["region"][1] == doctest::Approx(childBoundSouth));
            CHECK(child["boundingVolume"]["region"][2] == doctest::Approx(childBoundEast));
            CHECK(child["boundingVolume"]["region"][3] == doctest::Approx(childBoundNorth));
        }
    }

    {
        std::filesystem::path tilesetOutput = output / "Elevation_1_1RoadNetwork_2_3.json";
        CHECK(std::filesystem::exists(tilesetOutput));
        std::ifstream fs(tilesetOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(fs);

        auto N32W119 = CDBGeoCell(32, -119);
        auto N32W119BoundRegion = CDBTile::calcBoundRegion(N32W119, -10, 0, 0);

        auto N32W118 = CDBGeoCell(32, -118);
        auto N32W118BoundRegion = CDBTile::calcBoundRegion(N32W118, -10, 0, 0);

        auto unionBoundRegion = N32W118BoundRegion.computeUnion(N32W119BoundRegion);
        auto unionRectangle = unionBoundRegion.getRectangle();
        double boundNorth = unionRectangle.getNorth();
        double boundWest = unionRectangle.getWest();
        double boundSouth = unionRectangle.getSouth();
        double boundEast = unionRectangle.getEast();
        CHECK(tilesetJson["geometricError"] == doctest::Approx(300000.0f));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][0] == doctest::Approx(boundWest));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][1] == doctest::Approx(boundSouth));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][2] == doctest::Approx(boundEast));
        CHECK(tilesetJson["root"]["boundingVolume"]["region"][3] == doctest::Approx(boundNorth));
        CHECK(tilesetJson["root"]["children"].size() == 2);

        {
            // elevation
            auto childRectangle = N32W119BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][0];
            CHECK(child["content"]["uri"] == "Elevation_1_1.json");
            CHECK(child["boundingVolume"]["region"][0] == doctest::Approx(childBoundWest));
            CHECK(child["boundingVolume"]["region"][1] == doctest::Approx(childBoundSouth));
            CHECK(child["boundingVolume"]["region"][2] == doctest::Approx(childBoundEast));
            CHECK(child["boundingVolume"]["region"][3] == doctest::Approx(childBoundNorth));
        }

        {
            // RoadNetwork
            auto childRectangle = N32W118BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][1];
            CHECK(child["content"]["uri"] == "RoadNetwork_2_3.json");
            CHECK(child["boundingVolume"]["region"][0] == doctest::Approx(childBoundWest));
            CHECK(child["boundingVolume"]["region"][1] == doctest::Approx(childBoundSouth));
            CHECK(child["boundingVolume"]["region"][2] == doctest::Approx(childBoundEast));
            CHECK(child["boundingVolume"]["region"][3] == doctest::Approx(childBoundNorth));
        }
    }

    std::filesystem::remove_all(output);
}

TEST_CASE("Test converter for implicit elevation")
{
    const uint64_t headerByteLength = 24;
    std::filesystem::path input = dataPath / "CombineTilesets";
    CDB cdb(input);
    std::filesystem::path output = "CombineTilesets";
    std::filesystem::path elevationTilePath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L02"
                                              / "U2" / "N32W119_D001_S001_T001_L02_U2_R3.tif";
    std::unique_ptr<CDBTilesetBuilder> m_impl = std::make_unique<CDBTilesetBuilder>(input, output);
    std::optional<CDBElevation> elevation = CDBElevation::createFromFile(elevationTilePath);
    SUBCASE("Test converter errors out of 3D Tiles Next conversion with uninitialized availabilty buffer.")
    {
        SubtreeAvailability *nullPointer = NULL;
        CHECK_THROWS_AS(m_impl->addAvailability((*elevation).getTile(), nullPointer, 0, 0, 0),
                          std::invalid_argument);
    }

    // TODO write function for creating buffer given subtree level
    int subtreeLevels = 3;
    m_impl->subtreeLevels = subtreeLevels;
    uint64_t subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels) - 1) / 3);
    uint64_t childSubtreeCount = static_cast<int>(pow(4, subtreeLevels)); // 4^N
    uint64_t availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
    uint64_t childSubtreeAvailabilityByteLength = static_cast<int>(
        ceil(static_cast<double>(childSubtreeCount) / 8.0));
    m_impl->nodeAvailabilityByteLengthWithPadding = availabilityByteLength;
    m_impl->childSubtreeAvailabilityByteLengthWithPadding = childSubtreeAvailabilityByteLength;

    SubtreeAvailability subtree = m_impl->createSubtreeAvailability();

    m_impl->addAvailability((*elevation).getTile(), &subtree, 0, 0, 0);
    SUBCASE("Test availability bit is set with correct morton index.")
    {
        const auto &cdbTile = elevation->getTile();
        uint64_t mortonIndex = libmorton::morton2D_64_encode(cdbTile.getRREF(), cdbTile.getUREF());
        int levelWithinSubtree = cdbTile.getLevel();
        const uint64_t nodeCountUpToThisLevel = ((1 << (2 * levelWithinSubtree)) - 1) / 3;

        const uint64_t index = nodeCountUpToThisLevel + mortonIndex;
        uint64_t byte = index / 8;
        uint64_t bit = index % 8;
        const uint8_t availability = static_cast<uint8_t>(1 << bit);
        CHECK(subtree.nodeBuffer[byte] == availability);
    }

    SUBCASE("Test available node count is being incremented.") { CHECK(subtree.nodeCount == 1); }

    subtreeLevels = 2;
    m_impl->subtreeLevels = subtreeLevels;
    m_impl->datasetCSTileAndChildAvailabilities.clear();
    subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels) - 1) / 3);
    childSubtreeCount = static_cast<int>(pow(4, subtreeLevels)); // 4^N
    availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
    childSubtreeAvailabilityByteLength = static_cast<int>(ceil(static_cast<double>(childSubtreeCount) / 8.0));
    m_impl->nodeAvailabilityByteLengthWithPadding = availabilityByteLength;
    m_impl->childSubtreeAvailabilityByteLengthWithPadding = childSubtreeAvailabilityByteLength;
    subtree = m_impl->createSubtreeAvailability();
    std::vector<uint8_t> childSubtreeAvailabilityBufferVerified(childSubtreeAvailabilityByteLength);
    elevationTilePath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L01" / "U1"
                        / "N32W119_D001_S001_T001_L01_U1_R1.tif";
    elevation = CDBElevation::createFromFile(elevationTilePath);
    m_impl->addAvailability((*elevation).getTile(), &subtree, 0, 0, 0);

    std::filesystem::path elevationChildPath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L02"
                                               / "U2" / "N32W119_D001_S001_T001_L02_U2_R2.tif";
    std::optional<CDBElevation> elevationChild = CDBElevation::createFromFile(elevationChildPath);
    m_impl->addAvailability((*elevationChild).getTile(), &subtree, 2, 2, 2);
    elevationChildPath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L02" / "U2"
                         / "N32W119_D001_S001_T001_L02_U2_R3.tif";
    elevationChild = CDBElevation::createFromFile(elevationChildPath);
    m_impl->addAvailability((*elevationChild).getTile(), &subtree, 2, 3, 2);
    elevationChildPath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L02" / "U3"
                         / "N32W119_D001_S001_T001_L02_U3_R2.tif";
    elevationChild = CDBElevation::createFromFile(elevationChildPath);
    m_impl->addAvailability((*elevationChild).getTile(), &subtree, 2, 2, 3);
    elevationChildPath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L02" / "U3"
                         / "N32W119_D001_S001_T001_L02_U3_R3.tif";
    elevationChild = CDBElevation::createFromFile(elevationChildPath);
    m_impl->addAvailability((*elevationChild).getTile(), &subtree, 2, 3, 3);
    SUBCASE("Test child subtree availability bit is set with correct morton index.")
    {
        const auto &cdbTile = elevation->getTile();
        auto nw = CDBTile::createNorthWestForPositiveLOD(cdbTile);
        auto ne = CDBTile::createNorthEastForPositiveLOD(cdbTile);
        auto sw = CDBTile::createSouthWestForPositiveLOD(cdbTile);
        auto se = CDBTile::createSouthEastForPositiveLOD(cdbTile);
        for (auto childTile : {nw, ne, sw, se}) {
            uint64_t childMortonIndex = libmorton::morton2D_64_encode(childTile.getRREF(),
                                                                      childTile.getUREF());
            const uint64_t childByte = childMortonIndex / 8;
            const uint64_t childBit = childMortonIndex % 8;
            uint8_t availability = static_cast<uint8_t>(1 << childBit);
            (&childSubtreeAvailabilityBufferVerified.at(0))[childByte] |= availability;
        }
        CHECK(childSubtreeAvailabilityBufferVerified
                == m_impl->datasetCSTileAndChildAvailabilities.at(CDBDataset::Elevation)
                       .at("1_1")
                       .at("0_0_0")
                       .childBuffer);
    }

    SUBCASE("Test availability buffer correct length for subtree level and verify subtree json.")
    {
        subtreeLevels = 4;
        Converter converter(input, output);
        converter.setSubtreeLevels(subtreeLevels);
        converter.setUse3dTilesNext(true);
        converter.convert();

        std::filesystem::path subtreeBinary = output / "Tiles" / "N32" / "W119" / "Elevation" / "1_1"
                                              / "subtrees" / "0_0_0.subtree";
        CHECK(std::filesystem::exists(subtreeBinary));

        subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels) - 1) / 3);
        availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
        const uint64_t nodeAvailabilityByteLengthWithPadding = alignTo8(availabilityByteLength);

        // buffer length is header + json + node availability buffer + child subtree availability (constant in this case, so no buffer)
        std::filesystem::path binaryBufferPath = output / "Tiles" / "N32" / "W119" / "Elevation" / "1_1"
                                                 / "availability" / "0_0_0.bin";
        CHECK(std::filesystem::exists(binaryBufferPath));
        std::ifstream availabilityInputStream(binaryBufferPath, std::ios_base::binary);
        std::vector<unsigned char> availabilityBuffer(std::istreambuf_iterator<char>(availabilityInputStream),
                                                      {});

        std::ifstream subtreeInputStream(subtreeBinary, std::ios_base::binary);
        std::vector<unsigned char> subtreeBuffer(std::istreambuf_iterator<char>(subtreeInputStream), {});

        uint64_t jsonStringByteLength = *(uint64_t *) &subtreeBuffer[8]; // 64-bit int from 8 8-bit ints
        uint32_t binaryBufferByteLength = static_cast<uint32_t>(availabilityBuffer.size());
        CHECK(binaryBufferByteLength == nodeAvailabilityByteLengthWithPadding);

        std::vector<unsigned char>::iterator jsonBeginning = subtreeBuffer.begin() + headerByteLength;
        std::string jsonString(jsonBeginning, jsonBeginning + jsonStringByteLength);
        nlohmann::json subtreeJson = nlohmann::json::parse(jsonString);
        std::ifstream fs(input / "VerifiedSubtree.json");
        nlohmann::json verifiedJson = nlohmann::json::parse(fs);
        CHECK(subtreeJson == verifiedJson);
    }

    SUBCASE("Test that subtree JSON has no buffer object when availabilities are both constant.")
    {
        input = dataPath / "CombineTilesetsSmallElevation";
        output = "CombineTilesetsSmallElevation";
        subtreeLevels = 2;
        Converter converter(input, output);
        converter.setSubtreeLevels(subtreeLevels);
        converter.setUse3dTilesNext(true);
        converter.convert();

        std::filesystem::path subtreeBinary = output / "Tiles" / "N32" / "W119" / "Elevation" / "1_1"
                                              / "subtrees" / "0_0_0.subtree";
        CHECK(std::filesystem::exists(subtreeBinary));

        std::ifstream inputStream(subtreeBinary, std::ios_base::binary);
        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputStream), {});
        uint64_t jsonStringByteLength = *(uint64_t *) &buffer[8];

        std::vector<unsigned char>::iterator jsonBeginning = buffer.begin() + headerByteLength;
        std::string jsonString(jsonBeginning, jsonBeginning + jsonStringByteLength);
        nlohmann::json subtreeJson = nlohmann::json::parse(jsonString);

        CHECK(subtreeJson.find("buffers") == subtreeJson.end());
        CHECK(subtreeJson.find("bufferViews") == subtreeJson.end());
    }

    SUBCASE("Verify geocell tileset json.")
    {
        subtreeLevels = 4;
        Converter converter(input, output);
        converter.setSubtreeLevels(subtreeLevels);
        converter.setUse3dTilesNext(true);
        converter.convert();

        std::filesystem::path geoCellJson = output / "Tiles" / "N32" / "W119" / "Elevation" / "1_1"
                                            / "N32W119_D001_S001_T001.json";
        CHECK(std::filesystem::exists(geoCellJson));
        std::ifstream fs(geoCellJson);
        nlohmann::json tilesetJson = nlohmann::json::parse(fs);
        nlohmann::json child = tilesetJson["root"];

        // Get down to the last explicitly defined tile
        while (child.find("children") != child.end()) {
            child = child["children"][0];
        }

        nlohmann::json implicitTiling = child["extensions"]["3DTILES_implicit_tiling"];
        CHECK(implicitTiling["maximumLevel"] == 2);
        CHECK(implicitTiling["subdivisionScheme"] == "QUADTREE");
        CHECK(implicitTiling["subtreeLevels"] == subtreeLevels);
        CHECK(implicitTiling["subtrees"]["uri"] == "subtrees/{level}_{x}_{y}.subtree");

        // Make sure extensions are in extensionsUsed and extensionsCHECKd
        nlohmann::json extensionsUsed = tilesetJson["extensionsUsed"];
        CHECK(std::find(extensionsUsed.begin(), extensionsUsed.end(), "3DTILES_implicit_tiling")
                != extensionsUsed.end());

        nlohmann::json extensionsCHECKd = tilesetJson["extensionsCHECKd"];
        CHECK(std::find(extensionsCHECKd.begin(), extensionsCHECKd.end(), "3DTILES_implicit_tiling")
                != extensionsCHECKd.end());
    }

    std::filesystem::remove_all(output);
}

TEST_SUITE_END();

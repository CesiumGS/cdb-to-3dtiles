#include "CDBGeoCell.h"
#include "CDBTile.h"
#include "CDBTo3DTiles.h"
#include "Config.h"
#include "catch2/catch.hpp"
#include "glm/glm.hpp"
#include "nlohmann/json.hpp"
#include "morton.h"
#include <fstream>
#include "CDBElevation.h"
#include "CDB.h"

using namespace CDBTo3DTiles;


TEST_CASE("Test invalid combined dataset", "[CombineTilesets]")
{
    std::filesystem::path input = dataPath / "CombineTilesets";
    std::filesystem::path output = "CombineTilesets";

    Converter converter(input, output);
    REQUIRE_THROWS_WITH(converter.combineDataset({"Elevation_1_1", "SASS_2_2", "HydrographyNetwork_2_2"}),
                        "Unrecognize dataset: SASS\n"
                        "Correct dataset names are: \n"
                        "GTModels\n"
                        "HydrographyNetwork\n"
                        "GSModels\n"
                        "PowerlineNetwork\n"
                        "RailRoadNetwork\n"
                        "RoadNetwork\n"
                        "Elevation\n");

    REQUIRE_THROWS_WITH(converter.combineDataset({"Elevation_as_1", "GTModels_1_1"}),
                        "Component selector 1 has to be a number");
    REQUIRE_THROWS_WITH(converter.combineDataset({"Elevation_1_as", "GTModels_1_1"}),
                        "Component selector 2 has to be a number");
}

TEST_CASE("Test converter combines all the tilesets available in the GeoCells", "[CombineTilesets]")
{
    std::filesystem::path input = dataPath / "CombineTilesets";
    std::filesystem::path output = "CombineTilesets";

    Converter converter(input, output);
    converter.convert();

    {
        // check the combined elevation tileset in geocell N32 W119
        std::filesystem::path elevationOutput = output / "Elevation_1_1.json";
        REQUIRE(std::filesystem::exists(elevationOutput));
        std::ifstream elevationFs(elevationOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(elevationFs);

        auto geoCell = CDBGeoCell(32, -119);
        auto boundRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
        const auto &boundRectangle = boundRegion.getRectangle();
        double boundNorth = boundRectangle.getNorth();
        double boundWest = boundRectangle.getWest();
        double boundSouth = boundRectangle.getSouth();
        double boundEast = boundRectangle.getEast();
        REQUIRE(tilesetJson["geometricError"] == Approx(300000.0f));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][3] == Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        REQUIRE(child["content"]["uri"]
                == (geoCell.getRelativePath() / "Elevation" / "1_1" / "N32W119_D001_S001_T001.json").string());
        REQUIRE(child["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(child["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(child["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(child["boundingVolume"]["region"][3] == Approx(boundNorth));
    }

    {
        // check GTModels
        std::filesystem::path tilesetOutput = output / "GTModels_2_1.json";
        REQUIRE(std::filesystem::exists(tilesetOutput));
        std::ifstream fs(tilesetOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(fs);

        auto geoCell = CDBGeoCell(32, -118);
        auto boundRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
        const auto &boundRectangle = boundRegion.getRectangle();
        double boundNorth = boundRectangle.getNorth();
        double boundWest = boundRectangle.getWest();
        double boundSouth = boundRectangle.getSouth();
        double boundEast = boundRectangle.getEast();
        REQUIRE(tilesetJson["geometricError"] == Approx(300000.0f));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][3] == Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        REQUIRE(child["content"]["uri"]
                == (geoCell.getRelativePath() / "GTModels" / "2_1" / "N32W118_D101_S002_T001.json").string());
        REQUIRE(child["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(child["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(child["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(child["boundingVolume"]["region"][3] == Approx(boundNorth));
    }

    {
        // check GTModels
        std::filesystem::path tilesetOutput = output / "GTModels_1_1.json";
        REQUIRE(std::filesystem::exists(tilesetOutput));
        std::ifstream fs(tilesetOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(fs);

        auto geoCell = CDBGeoCell(32, -118);
        auto boundRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
        const auto &boundRectangle = boundRegion.getRectangle();
        double boundNorth = boundRectangle.getNorth();
        double boundWest = boundRectangle.getWest();
        double boundSouth = boundRectangle.getSouth();
        double boundEast = boundRectangle.getEast();
        REQUIRE(tilesetJson["geometricError"] == Approx(300000.0f));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][3] == Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        REQUIRE(child["content"]["uri"]
                == (geoCell.getRelativePath() / "GTModels" / "1_1" / "N32W118_D101_S001_T001.json").string());
        REQUIRE(child["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(child["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(child["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(child["boundingVolume"]["region"][3] == Approx(boundNorth));
    }

    {
        // check RoadNetwork
        std::filesystem::path tilesetOutput = output / "RoadNetwork_2_3.json";
        REQUIRE(std::filesystem::exists(tilesetOutput));
        std::ifstream fs(tilesetOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(fs);

        auto geoCell = CDBGeoCell(32, -118);
        auto boundRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
        const auto &boundRectangle = boundRegion.getRectangle();
        double boundNorth = boundRectangle.getNorth();
        double boundWest = boundRectangle.getWest();
        double boundSouth = boundRectangle.getSouth();
        double boundEast = boundRectangle.getEast();
        REQUIRE(tilesetJson["geometricError"] == Approx(300000.0f));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][3] == Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        REQUIRE(
            child["content"]["uri"]
            == (geoCell.getRelativePath() / "RoadNetwork" / "2_3" / "N32W118_D201_S002_T003.json").string());
        REQUIRE(child["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(child["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(child["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(child["boundingVolume"]["region"][3] == Approx(boundNorth));
    }

    std::filesystem::remove_all(output);
}

TEST_CASE("Test converer combine one set of requested datasets", "[CombineTilesets]")
{
    std::filesystem::path input = dataPath / "CombineTilesets";
    std::filesystem::path output = "CombineTilesets";

    Converter converter(input, output);
    converter.combineDataset({"Elevation_1_1"});
    converter.combineDataset({"Elevation_1_1", "RoadNetwork_2_3", "GTModels_1_1"});
    converter.convert();

    SECTION("Test the converter doesn't combine only one requested dataset since it is already processed by "
            "default")
    {
        // check the combined elevation tileset in geocell N32 W119
        std::filesystem::path elevationOutput = output / "Elevation_1_1.json";
        REQUIRE(std::filesystem::exists(elevationOutput));
        std::ifstream elevationFs(elevationOutput);
        nlohmann::json tilesetJson = nlohmann::json::parse(elevationFs);

        auto geoCell = CDBGeoCell(32, -119);
        auto boundRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
        const auto &boundRectangle = boundRegion.getRectangle();
        double boundNorth = boundRectangle.getNorth();
        double boundWest = boundRectangle.getWest();
        double boundSouth = boundRectangle.getSouth();
        double boundEast = boundRectangle.getEast();
        REQUIRE(tilesetJson["geometricError"] == Approx(300000.0f));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][3] == Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        REQUIRE(child["content"]["uri"]
                == (geoCell.getRelativePath() / "Elevation" / "1_1" / "N32W119_D001_S001_T001.json").string());
        REQUIRE(child["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(child["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(child["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(child["boundingVolume"]["region"][3] == Approx(boundNorth));
    }

    SECTION("Test multiple dataset combine together")
    {
        std::filesystem::path tilesetOutput = output / "tileset.json";
        REQUIRE(std::filesystem::exists(tilesetOutput));
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
        REQUIRE(tilesetJson["geometricError"] == Approx(300000.0f));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][3] == Approx(boundNorth));
        REQUIRE(tilesetJson["root"]["children"].size() == 3);

        {
            // elevation
            auto childRectangle = N32W119BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][0];
            REQUIRE(child["content"]["uri"] == "Elevation_1_1.json");
            REQUIRE(child["boundingVolume"]["region"][0] == Approx(childBoundWest));
            REQUIRE(child["boundingVolume"]["region"][1] == Approx(childBoundSouth));
            REQUIRE(child["boundingVolume"]["region"][2] == Approx(childBoundEast));
            REQUIRE(child["boundingVolume"]["region"][3] == Approx(childBoundNorth));
        }

        {
            // RoadNetwork
            auto childRectangle = N32W118BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][1];
            REQUIRE(child["content"]["uri"] == "RoadNetwork_2_3.json");
            REQUIRE(child["boundingVolume"]["region"][0] == Approx(childBoundWest));
            REQUIRE(child["boundingVolume"]["region"][1] == Approx(childBoundSouth));
            REQUIRE(child["boundingVolume"]["region"][2] == Approx(childBoundEast));
            REQUIRE(child["boundingVolume"]["region"][3] == Approx(childBoundNorth));
        }

        {
            // GTModels
            auto childRectangle = N32W118BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][2];
            REQUIRE(child["content"]["uri"] == "GTModels_1_1.json");
            REQUIRE(child["boundingVolume"]["region"][0] == Approx(childBoundWest));
            REQUIRE(child["boundingVolume"]["region"][1] == Approx(childBoundSouth));
            REQUIRE(child["boundingVolume"]["region"][2] == Approx(childBoundEast));
            REQUIRE(child["boundingVolume"]["region"][3] == Approx(childBoundNorth));
        }
    }

    std::filesystem::remove_all(output);
}

TEST_CASE("Test combine multiple sets of tilesets", "[CombineTilesets]")
{
    std::filesystem::path input = dataPath / "CombineTilesets";
    std::filesystem::path output = "CombineTilesets";

    Converter converter(input, output);
    converter.combineDataset({"Elevation_1_1", "RoadNetwork_2_3"});
    converter.combineDataset({"Elevation_1_1", "GTModels_1_1"});
    converter.convert();

    {
        std::filesystem::path tilesetOutput = output / "Elevation_1_1GTModels_1_1.json";
        REQUIRE(std::filesystem::exists(tilesetOutput));
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
        REQUIRE(tilesetJson["geometricError"] == Approx(300000.0f));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][3] == Approx(boundNorth));
        REQUIRE(tilesetJson["root"]["children"].size() == 2);

        {
            // elevation
            auto childRectangle = N32W119BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][0];
            REQUIRE(child["content"]["uri"] == "Elevation_1_1.json");
            REQUIRE(child["boundingVolume"]["region"][0] == Approx(childBoundWest));
            REQUIRE(child["boundingVolume"]["region"][1] == Approx(childBoundSouth));
            REQUIRE(child["boundingVolume"]["region"][2] == Approx(childBoundEast));
            REQUIRE(child["boundingVolume"]["region"][3] == Approx(childBoundNorth));
        }

        {
            // GTModels
            auto childRectangle = N32W118BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][1];
            REQUIRE(child["content"]["uri"] == "GTModels_1_1.json");
            REQUIRE(child["boundingVolume"]["region"][0] == Approx(childBoundWest));
            REQUIRE(child["boundingVolume"]["region"][1] == Approx(childBoundSouth));
            REQUIRE(child["boundingVolume"]["region"][2] == Approx(childBoundEast));
            REQUIRE(child["boundingVolume"]["region"][3] == Approx(childBoundNorth));
        }
    }

    {
        std::filesystem::path tilesetOutput = output / "Elevation_1_1RoadNetwork_2_3.json";
        REQUIRE(std::filesystem::exists(tilesetOutput));
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
        REQUIRE(tilesetJson["geometricError"] == Approx(300000.0f));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][3] == Approx(boundNorth));
        REQUIRE(tilesetJson["root"]["children"].size() == 2);

        {
            // elevation
            auto childRectangle = N32W119BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][0];
            REQUIRE(child["content"]["uri"] == "Elevation_1_1.json");
            REQUIRE(child["boundingVolume"]["region"][0] == Approx(childBoundWest));
            REQUIRE(child["boundingVolume"]["region"][1] == Approx(childBoundSouth));
            REQUIRE(child["boundingVolume"]["region"][2] == Approx(childBoundEast));
            REQUIRE(child["boundingVolume"]["region"][3] == Approx(childBoundNorth));
        }

        {
            // RoadNetwork
            auto childRectangle = N32W118BoundRegion.getRectangle();
            double childBoundNorth = childRectangle.getNorth();
            double childBoundWest = childRectangle.getWest();
            double childBoundSouth = childRectangle.getSouth();
            double childBoundEast = childRectangle.getEast();

            auto child = tilesetJson["root"]["children"][1];
            REQUIRE(child["content"]["uri"] == "RoadNetwork_2_3.json");
            REQUIRE(child["boundingVolume"]["region"][0] == Approx(childBoundWest));
            REQUIRE(child["boundingVolume"]["region"][1] == Approx(childBoundSouth));
            REQUIRE(child["boundingVolume"]["region"][2] == Approx(childBoundEast));
            REQUIRE(child["boundingVolume"]["region"][3] == Approx(childBoundNorth));
        }
    }

    std::filesystem::remove_all(output);
}

TEST_CASE("Test converter for implicit elevation", "[CombineTilesets]")
{
  const uint64_t headerByteLength = 24;
  std::filesystem::path input = dataPath / "CombineTilesets";
  CDB cdb(input);
  std::filesystem::path output = "CombineTilesets";
  std::filesystem::path elevationTilePath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L02" / "U2" / "N32W119_D001_S001_T001_L02_U2_R3.tif";
  std::unique_ptr<CDBTilesetBuilder> m_impl = std::make_unique<CDBTilesetBuilder>(input, output);
  std::optional<CDBElevation> elevation = CDBElevation::createFromFile(elevationTilePath);
  SECTION("Test converter errors out of 3D Tiles Next conversion with uninitialized availabilty buffer.")
  {
    SubtreeAvailability *nullPointer = NULL;
    REQUIRE_THROWS_AS(m_impl->addDatasetAvailability((*elevation).getTile(), nullPointer, 0, 0, 0), std::invalid_argument);
  }

  
  // TODO write function for creating buffer given subtree level
  int subtreeLevels = 3;
  m_impl->subtreeLevels = subtreeLevels;
  uint64_t subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels)-1) / 3);
  uint64_t childSubtreeCount = static_cast<int>(pow(4, subtreeLevels)); // 4^N
  uint64_t availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
  uint64_t childSubtreeAvailabilityByteLength = static_cast<int>(ceil(static_cast<double>(childSubtreeCount) / 8.0));
  m_impl->nodeAvailabilityByteLengthWithPadding = availabilityByteLength;
  m_impl->childSubtreeAvailabilityByteLengthWithPadding = childSubtreeAvailabilityByteLength;

  SubtreeAvailability subtree = m_impl->createSubtreeAvailability();

  m_impl->addDatasetAvailability((*elevation).getTile(), &subtree, 0, 0, 0);
  SECTION("Test availability bit is set with correct morton index.")
  {
    const auto &cdbTile = elevation->getTile();
    uint64_t mortonIndex = libmorton::morton2D_64_encode(cdbTile.getRREF(), cdbTile.getUREF());
    int levelWithinSubtree = cdbTile.getLevel();
    const uint64_t nodeCountUpToThisLevel = ((1 << (2 * levelWithinSubtree)) - 1) / 3;

    const uint64_t index = nodeCountUpToThisLevel + mortonIndex;
    uint64_t byte = index / 8;
    uint64_t bit = index % 8;
    const uint8_t availability = static_cast<uint8_t>(1 << bit);
    REQUIRE(subtree.nodeBuffer[byte] == availability);
  }

  SECTION("Test available node count is being incremented.")
  {
    REQUIRE(subtree.nodeCount == 1);
  }

  subtreeLevels = 2;
  m_impl->subtreeLevels = subtreeLevels;
  m_impl->datasetCSTileAndChildAvailabilities.clear();
  subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels)-1) / 3);
  childSubtreeCount = static_cast<int>(pow(4, subtreeLevels)); // 4^N
  availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
  childSubtreeAvailabilityByteLength = static_cast<int>(ceil(static_cast<double>(childSubtreeCount) / 8.0));
  m_impl->nodeAvailabilityByteLengthWithPadding = availabilityByteLength;
  m_impl->childSubtreeAvailabilityByteLengthWithPadding = childSubtreeAvailabilityByteLength;
  subtree = m_impl->createSubtreeAvailability();
  std::vector<uint8_t> childSubtreeAvailabilityBufferVerified(childSubtreeAvailabilityByteLength);
  elevationTilePath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L01" / "U1" / "N32W119_D001_S001_T001_L01_U1_R1.tif";
  elevation = CDBElevation::createFromFile(elevationTilePath);
  m_impl->addDatasetAvailability((*elevation).getTile(), &subtree, 0, 0, 0);

  std::filesystem::path elevationChildPath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L02" / "U2" / "N32W119_D001_S001_T001_L02_U2_R2.tif";
  std::optional<CDBElevation> elevationChild = CDBElevation::createFromFile(elevationChildPath);
  m_impl->addDatasetAvailability((*elevationChild).getTile(), &subtree, 2, 2, 2);
  elevationChildPath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L02" / "U2" / "N32W119_D001_S001_T001_L02_U2_R3.tif";
  elevationChild = CDBElevation::createFromFile(elevationChildPath);
  m_impl->addDatasetAvailability((*elevationChild).getTile(), &subtree, 2, 3, 2);
  elevationChildPath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L02" / "U3" / "N32W119_D001_S001_T001_L02_U3_R2.tif";
  elevationChild = CDBElevation::createFromFile(elevationChildPath);
  m_impl->addDatasetAvailability((*elevationChild).getTile(), &subtree, 2, 2, 3);
  elevationChildPath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L02" / "U3" / "N32W119_D001_S001_T001_L02_U3_R3.tif";
  elevationChild = CDBElevation::createFromFile(elevationChildPath);
  m_impl->addDatasetAvailability((*elevationChild).getTile(), &subtree, 2, 3, 3);
  SECTION("Test child subtree availability bit is set with correct morton index.")
  {
    const auto &cdbTile = elevation->getTile();
    auto nw = CDBTile::createNorthWestForPositiveLOD(cdbTile);
    auto ne = CDBTile::createNorthEastForPositiveLOD(cdbTile);
    auto sw = CDBTile::createSouthWestForPositiveLOD(cdbTile);
    auto se = CDBTile::createSouthEastForPositiveLOD(cdbTile);
    for(auto childTile : {nw, ne, sw, se})
    {
      uint64_t childMortonIndex = libmorton::morton2D_64_encode(childTile.getRREF(), childTile.getUREF());
      const uint64_t childByte = childMortonIndex / 8;
      const uint64_t childBit = childMortonIndex % 8;
      uint8_t availability = static_cast<uint8_t>(1 << childBit);
      (&childSubtreeAvailabilityBufferVerified.at(0))[childByte] |= availability;
    }
    REQUIRE(childSubtreeAvailabilityBufferVerified == m_impl->datasetCSTileAndChildAvailabilities.at(CDBDataset::Elevation).at("1_1").at("0_0_0").childBuffer);
  }

  SECTION("Test availability buffer correct length for subtree level and verify subtree json.")
  {
    subtreeLevels = 4;
    Converter converter(input, output);
    converter.setSubtreeLevels(subtreeLevels);
    converter.setUse3dTilesNext(true);
    converter.convert();

    std::filesystem::path subtreeBinary = output / "Tiles" / "N32" / "W119" / "subtrees" / "Elevation" / "0_0_0.subtree";
    REQUIRE(std::filesystem::exists(subtreeBinary));

    subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels)-1) / 3);
    availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
    const uint64_t nodeAvailabilityByteLengthWithPadding = alignTo8(availabilityByteLength);


    // buffer length is header + json + node availability buffer + child subtree availability (constant in this case, so no buffer)
    std::filesystem::path binaryBufferPath = output / "Tiles" / "N32" / "W119" / "Elevation" / "1_1" / "availability" / "0_0_0.bin";
    REQUIRE(std::filesystem::exists(binaryBufferPath));
    std::ifstream availabilityInputStream(binaryBufferPath, std::ios_base::binary);
    std::vector<unsigned char> availabilityBuffer(std::istreambuf_iterator<char>(availabilityInputStream), {});

    std::ifstream subtreeInputStream(subtreeBinary, std::ios_base::binary);
    std::vector<unsigned char> subtreeBuffer(std::istreambuf_iterator<char>(subtreeInputStream), {});

    uint64_t jsonStringByteLength = *(uint64_t*)&subtreeBuffer[8]; // 64-bit int from 8 8-bit ints
    uint32_t binaryBufferByteLength = static_cast<uint32_t>(availabilityBuffer.size());
    REQUIRE(binaryBufferByteLength == nodeAvailabilityByteLengthWithPadding);

    std::vector<unsigned char>::iterator jsonBeginning = subtreeBuffer.begin() + headerByteLength;
    std::string jsonString(jsonBeginning, jsonBeginning + jsonStringByteLength);
    nlohmann::json subtreeJson = nlohmann::json::parse(jsonString);
    std::ifstream fs(input / "VerifiedSubtree.json");
    nlohmann::json verifiedJson = nlohmann::json::parse(fs);
    REQUIRE(subtreeJson == verifiedJson);
  }

  SECTION("Test that subtree JSON has no buffer object when availabilities are both constant.")
  {
    input = dataPath / "CombineTilesetsSmallElevation";
    output = "CombineTilesetsSmallElevation";
    subtreeLevels = 2;
    Converter converter(input, output);
    converter.setSubtreeLevels(subtreeLevels);
    converter.setUse3dTilesNext(true);
    converter.convert();

    std::filesystem::path subtreeBinary = output / "Tiles" / "N32" / "W119" / "subtrees" / "Elevation" / "0_0_0.subtree";
    REQUIRE(std::filesystem::exists(subtreeBinary));

    std::ifstream inputStream(subtreeBinary, std::ios_base::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputStream), {});
    uint64_t jsonStringByteLength = *(uint64_t*)&buffer[8];

    std::vector<unsigned char>::iterator jsonBeginning = buffer.begin() + headerByteLength;
    std::string jsonString(jsonBeginning, jsonBeginning + jsonStringByteLength);
    nlohmann::json subtreeJson = nlohmann::json::parse(jsonString);

    REQUIRE(subtreeJson.find("buffers") == subtreeJson.end());
    REQUIRE(subtreeJson.find("bufferViews") == subtreeJson.end());
  }

  SECTION("Verify geocell tileset json.")
  {
    subtreeLevels = 4;
    Converter converter(input, output);
    converter.setSubtreeLevels(subtreeLevels);
    converter.setUse3dTilesNext(true);
    converter.convert();

    std::filesystem::path geoCellJson = output / "Tiles" / "N32" / "W119" / "N32W119_Elevation.json";
    REQUIRE(std::filesystem::exists(geoCellJson));
    std::ifstream fs(geoCellJson);
    nlohmann::json tilesetJson = nlohmann::json::parse(fs);
    nlohmann::json child = tilesetJson["root"];

    // Get down to the last explicitly defined tile
    while(child.find("children") != child.end())
    {
      child = child["children"][0];
    }

    nlohmann::json implicitTiling = child["extensions"]["3DTILES_implicit_tiling"];
    REQUIRE(implicitTiling["maximumLevel"] == 2);
    REQUIRE(implicitTiling["subdivisionScheme"] == "QUADTREE");
    REQUIRE(implicitTiling["subtreeLevels"] == 4);
    REQUIRE(implicitTiling["subtrees"]["uri"] == "subtrees/Elevation/{level}_{x}_{y}.subtree");

    nlohmann::json multipleContents = child["extensions"]["3DTILES_multiple_contents"];
    REQUIRE(multipleContents["content"].size() == 1); // only elevation for now
    REQUIRE(multipleContents["content"][0]["uri"] == "Elevation/1_1/N32W119_D001_S001_T001_L{level}_U{y}_R{x}.b3dm");

    // Make sure extensions are in extensionsUsed and extensionsRequired
    nlohmann::json extensionsUsed = tilesetJson["extensionsUsed"];
    REQUIRE(std::find(extensionsUsed.begin(), extensionsUsed.end(), "3DTILES_implicit_tiling") != extensionsUsed.end());
    REQUIRE(std::find(extensionsUsed.begin(), extensionsUsed.end(), "3DTILES_multiple_contents") != extensionsUsed.end());

    nlohmann::json extensionsRequired = tilesetJson["extensionsRequired"];
    REQUIRE(std::find(extensionsRequired.begin(), extensionsRequired.end(), "3DTILES_implicit_tiling") != extensionsRequired.end());
    REQUIRE(std::find(extensionsRequired.begin(), extensionsRequired.end(), "3DTILES_multiple_contents") != extensionsRequired.end());
  }

  std::filesystem::remove_all(output);
}

TEST_CASE("Test converter for multiple contents.", "[CombineTilesets]")
{
    std::filesystem::path input = dataPath / "CombineTilesets";
    CDB cdb(input);
    std::filesystem::path output = "CombineTilesets";
    std::filesystem::path elevationTilePath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L02" / "U2" / "N32W119_D001_S001_T001_L02_U2_R3.tif";
    std::unique_ptr<CDBTilesetBuilder> m_impl = std::make_unique<CDBTilesetBuilder>(input, output);
    std::optional<CDBElevation> elevation = CDBElevation::createFromFile(elevationTilePath);
    SECTION("Test converter addAvailability errors out when given unsupported dataset.")
    {
        const CDBTile tile = (*elevation).getTile();
        CDBTile invalidDatasetTile(tile.getGeoCell(), CDBDataset::ClientSpecific, 1, 1, 1, 1, 1);
        REQUIRE_THROWS_AS(m_impl->addAvailability(invalidDatasetTile), std::invalid_argument);
    }

    int subtreeLevels = 3;
    m_impl->subtreeLevels = subtreeLevels;
    uint64_t subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels)-1) / 3);
    uint64_t childSubtreeCount = static_cast<int>(pow(4, subtreeLevels)); // 4^N
    uint64_t availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
    uint64_t childSubtreeAvailabilityByteLength = static_cast<int>(ceil(static_cast<double>(childSubtreeCount) / 8.0));
    m_impl->nodeAvailabilityByteLengthWithPadding = availabilityByteLength;
    m_impl->childSubtreeAvailabilityByteLengthWithPadding = childSubtreeAvailabilityByteLength;

    SubtreeAvailability subtree = m_impl->createSubtreeAvailability();
    SECTION("Test availability bit is set with correct morton index for GTModels.")
    {
        CDBTile gtModelTile = CDBTile(CDBGeoCell(32, -118), CDBDataset::GTFeature, 2, 1, 1, 1, 1);
        m_impl->addDatasetAvailability(gtModelTile, &subtree, 0, 0, 0);
        uint64_t mortonIndex = libmorton::morton2D_64_encode(gtModelTile.getRREF(), gtModelTile.getUREF());
        int levelWithinSubtree = gtModelTile.getLevel();
        const uint64_t nodeCountUpToThisLevel = ((1 << (2 * levelWithinSubtree)) - 1) / 3;

        const uint64_t index = nodeCountUpToThisLevel + mortonIndex;
        uint64_t byte = index / 8;
        uint64_t bit = index % 8;
        const uint8_t availability = static_cast<uint8_t>(1 << bit);
        REQUIRE(subtree.nodeBuffer[byte] == availability);
    }

    Converter converter(input, output);
    subtreeLevels = 2;
    m_impl->subtreeLevels = subtreeLevels;
    subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels)-1) / 3);
    childSubtreeCount = static_cast<int>(pow(4, subtreeLevels)); // 4^N
    availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
    childSubtreeAvailabilityByteLength = static_cast<int>(ceil(static_cast<double>(childSubtreeCount) / 8.0));
    m_impl->nodeAvailabilityByteLengthWithPadding = availabilityByteLength;
    m_impl->childSubtreeAvailabilityByteLengthWithPadding = childSubtreeAvailabilityByteLength;
    converter.setSubtreeLevels(subtreeLevels);
    converter.setUse3dTilesNext(true);
    converter.convert();
    SECTION("Verify GTFeature and vector geocell tileset")
    {
        std::filesystem::path geoCellJson = output / "Tiles" / "N32" / "W118" / "N32W118_GTandVectors.json";
        REQUIRE(std::filesystem::exists(geoCellJson));
        std::ifstream fs(geoCellJson);
        nlohmann::json tilesetJson = nlohmann::json::parse(fs);
        nlohmann::json child = tilesetJson["root"];

        // Get down to where the road network is
        while(child.find("extensions") == child.end())
        {
            child = child["children"][0];
        }
        nlohmann::json multipleContents = child["extensions"]["3DTILES_multiple_contents"];
        REQUIRE(multipleContents["content"].size() == 1); 
        REQUIRE(multipleContents["content"][0]["uri"] == "RoadNetwork/2_3/N32W118_D201_S002_T003_LC5_U0_R0.b3dm");

        // Get down to the last explicitly defined tile
        while(child.find("children") != child.end())
        {
            child = child["children"][0];
        }
        nlohmann::json implicitTiling = child["extensions"]["3DTILES_implicit_tiling"];
        // maximumLevel is 0 because in the test data, there are no instances for level 1, though there exists files.
        //  So no .cmpt's are writting for level 1.
        REQUIRE(implicitTiling["maximumLevel"] == 0);
        REQUIRE(implicitTiling["subdivisionScheme"] == "QUADTREE");
        REQUIRE(implicitTiling["subtreeLevels"] == subtreeLevels);
        REQUIRE(implicitTiling["subtrees"]["uri"] == "subtrees/GTandVectors/{level}_{x}_{y}.subtree");

        multipleContents = child["extensions"]["3DTILES_multiple_contents"];
        REQUIRE(multipleContents["content"].size() == 2); // only GTFeature for now, with component selectors 1_1 and 2_1
        REQUIRE(multipleContents["content"][0]["uri"] == "GTModels/1_1/N32W118_D101_S001_T001_L{level}_U{y}_R{x}.cmpt");
        REQUIRE(multipleContents["content"][1]["uri"] == "GTModels/2_1/N32W118_D101_S002_T001_L{level}_U{y}_R{x}.cmpt");

        // Make sure extensions are in extensionsUsed and extensionsRequired
        nlohmann::json extensionsUsed = tilesetJson["extensionsUsed"];
        REQUIRE(std::find(extensionsUsed.begin(), extensionsUsed.end(), "3DTILES_implicit_tiling") != extensionsUsed.end());
        REQUIRE(std::find(extensionsUsed.begin(), extensionsUsed.end(), "3DTILES_multiple_contents") != extensionsUsed.end());

        nlohmann::json extensionsRequired = tilesetJson["extensionsRequired"];
        REQUIRE(std::find(extensionsRequired.begin(), extensionsRequired.end(), "3DTILES_implicit_tiling") != extensionsRequired.end());
        REQUIRE(std::find(extensionsRequired.begin(), extensionsRequired.end(), "3DTILES_multiple_contents") != extensionsRequired.end());
    }

    SECTION("Subtree JSON mutliple contents have same order as geocell JSON.")
    {
        std::filesystem::path subtreeBinary = output / "Tiles" / "N32" / "W118" / "subtrees" / "GTandVectors" / "0_0_0.subtree";
        REQUIRE(std::filesystem::exists(subtreeBinary));

        std::ifstream inputStream(subtreeBinary, std::ios_base::binary);
        std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputStream), {});
        uint64_t jsonStringByteLength = *(uint64_t*)&buffer[8];

        int headerByteLength = 24;
        std::vector<unsigned char>::iterator jsonBeginning = buffer.begin() + headerByteLength;
        std::string jsonString(jsonBeginning, jsonBeginning + jsonStringByteLength);
        nlohmann::json subtreeJson = nlohmann::json::parse(jsonString);

        nlohmann::json buffers = subtreeJson["buffers"];
        REQUIRE(buffers.size() == 3); // tile availability, 1_1, 2_1 buffers
        REQUIRE(buffers[1]["uri"] == "../../GTModels/1_1/availability/0_0_0.bin");
        REQUIRE(buffers[2]["uri"] == "../../GTModels/2_1/availability/0_0_0.bin");

        nlohmann::json contentAvailability = subtreeJson["extensions"]["3DTILES_multiple_contents"]["contentAvailability"];
        REQUIRE(contentAvailability[0]["bufferView"] == 1);
        REQUIRE(contentAvailability[1]["bufferView"] == 2);
    }

    // Would need to add more levels of test data for this test, a couple of kilobytes.
    // SECTION("Datasets absent from a subtree get constant 0 availability.")
    // {
    //     std::filesystem::path subtreeBinary = output / "Tiles" / "N32" / "W118" / "subtrees" / "GTandVectors" / "2_0_0.subtree";
    //     REQUIRE(std::filesystem::exists(subtreeBinary));

    //     std::ifstream inputStream(subtreeBinary, std::ios_base::binary);
    //     std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputStream), {});
    //     uint64_t jsonStringByteLength = *(uint64_t*)&buffer[8];

    //     int headerByteLength = 24;
    //     std::vector<unsigned char>::iterator jsonBeginning = buffer.begin() + headerByteLength;
    //     std::string jsonString(jsonBeginning, jsonBeginning + jsonStringByteLength);
    //     nlohmann::json subtreeJson = nlohmann::json::parse(jsonString);

    //     nlohmann::json contentAvailability = subtreeJson["extensions"]["3DTILES_multiple_contents"]["contentAvailability"];
    //     REQUIRE(contentAvailability[0]["constant"] == 0);
    //     REQUIRE(contentAvailability[1]["bufferView"] == 1);
    // }
    std::filesystem::remove_all(output);
}

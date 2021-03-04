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

inline uint64_t alignTo8(uint64_t v)
{
    return (v + 7) & ~7;
}

// static void createAvailabilityBuffers(int subtreeLevels, uint8_t *&nodeAvailabilityBufferPointer, std::unique_ptr<uint8_t> &childSubtreeAvailabilityBufferPointer)
// static void createAvailabilityBuffers(int subtreeLevels, std::unique_ptr<uint8_t> &nodeAvailabilityBufferPointer, std::unique_ptr<uint8_t> &childSubtreeAvailabilityBufferPointer)
// {
//   const uint64_t subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels)-1) / 3);
//   const uint64_t childSubtreeCount = static_cast<int>(pow(4, subtreeLevels)); // 4^N
//   const uint64_t availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
//   const uint64_t childSubtreeAvailabilityByteLength = static_cast<int>(ceil(static_cast<double>(childSubtreeCount) / 8.0));

//   std::vector<uint8_t> nodeAvailabilityBuffer(availabilityByteLength);
//   nodeAvailabilityBufferPointer = std::make_unique<uint8_t>(nodeAvailabilityBuffer.at(0));
//   std::vector<uint8_t> childSubtreeAvailabilityBuffer(childSubtreeAvailabilityByteLength);
//   childSubtreeAvailabilityBufferPointer = std::make_unique<uint8_t>(childSubtreeAvailabilityBuffer.at(0));

//   nodeAvailabilityBufferPointer = std::make_unique<uint8_t>(uint8_t [availabilityByteLength]);
//   childSubtreeAvailabilityBufferPointer = std::make_unique<uint8_t>(uint8_t[childSubtreeAvailabilityByteLength]);
// }

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
  std::filesystem::path input = dataPath / "CombineTilesets";
  CDB cdb(input);
  std::filesystem::path output = "CombineTilesets";
  std::filesystem::path elevationTilePath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L02" / "U2" / "N32W119_D001_S001_T001_L02_U2_R3.tif";
  std::unique_ptr<ConverterImpl> m_impl = std::make_unique<ConverterImpl>(input, output);
  std::optional<CDBElevation> elevation = CDBElevation::createFromFile(elevationTilePath);
  SECTION("Test converter errors out of 3D Tiles Next conversion with uninitialized availabilty buffer.")
  {
    uint8_t* nullPointer = NULL;
    uint64_t dummyInt;
    REQUIRE_THROWS_AS(m_impl->addElevationAvailability(*elevation, cdb, nullPointer, nullPointer, &dummyInt, &dummyInt), std::invalid_argument);
  }

  
  // TODO write function for creating buffer given subtree level
  int subtreeLevels = 3;
  m_impl->subtreeLevels = subtreeLevels;
  uint64_t subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels)-1) / 3);
  uint64_t childSubtreeCount = static_cast<int>(pow(4, subtreeLevels)); // 4^N
  uint64_t availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
  uint64_t childSubtreeAvailabilityByteLength = static_cast<int>(ceil(static_cast<double>(childSubtreeCount) / 8.0));
  std::vector<uint8_t> nodeAvailabilityBuffer(availabilityByteLength);
  std::vector<uint8_t> childSubtreeAvailabilityBuffer(childSubtreeAvailabilityByteLength);
  uint64_t availableNodeCount = 0;
  uint64_t availableChildSubtreeCount = 0;

  m_impl->addElevationAvailability(*elevation, cdb, &nodeAvailabilityBuffer.at(0), &childSubtreeAvailabilityBuffer.at(0), &availableNodeCount, &availableChildSubtreeCount, 0);
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
    REQUIRE(nodeAvailabilityBuffer[byte] == availability);
  }

  SECTION("Test available node count is being incremented.")
  {
    REQUIRE(availableNodeCount == 1);
  }

  subtreeLevels = 2;
  m_impl->subtreeLevels = subtreeLevels;
  subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels)-1) / 3);
  childSubtreeCount = static_cast<int>(pow(4, subtreeLevels)); // 4^N
  availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
  childSubtreeAvailabilityByteLength = static_cast<int>(ceil(static_cast<double>(childSubtreeCount) / 8.0));
  nodeAvailabilityBuffer.resize(availabilityByteLength);
  childSubtreeAvailabilityBuffer.resize(childSubtreeAvailabilityByteLength);
  std::vector<uint8_t> childSubtreeAvailabilityBufferVerified(childSubtreeAvailabilityByteLength);
  availableNodeCount = 0;
  availableChildSubtreeCount = 0;
  elevationTilePath = input / "Tiles" / "N32" / "W119" / "001_Elevation" / "L01" / "U1" / "N32W119_D001_S001_T001_L01_U1_R1.tif";
  elevation = CDBElevation::createFromFile(elevationTilePath);
  m_impl->addElevationAvailability(*elevation, cdb, &nodeAvailabilityBuffer.at(0), &childSubtreeAvailabilityBuffer.at(0), &availableNodeCount, &availableChildSubtreeCount, 0);
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
    REQUIRE(childSubtreeAvailabilityBufferVerified == childSubtreeAvailabilityBuffer);
  }

  SECTION("Test available child subtree count is being incremented.")
  {
    REQUIRE(availableChildSubtreeCount == 4);
  }

  SECTION("Test availability buffer correct length for subtree level.")
  {
    subtreeLevels = 4;
    Converter converter(input, output);
    converter.setSubtreeLevels(subtreeLevels);
    converter.setThreeDTilesNext(true);
    converter.convert();

    std::filesystem::path subtreeBinary = output / "Tiles" / "N32" / "W119" / "Elevation" / "subtrees" / "0_0_0.subtree";
    REQUIRE(std::filesystem::exists(subtreeBinary));

    subtreeNodeCount = static_cast<int>((pow(4, subtreeLevels)-1) / 3);
    availabilityByteLength = static_cast<int>(ceil(static_cast<double>(subtreeNodeCount) / 8.0));
    const uint64_t nodeAvailabilityByteLengthWithPadding = alignTo8(availabilityByteLength);

    const uint64_t headerByteLength = 24;

    // buffer length is header + json + node availability buffer + child subtree availability (constant in this case, so no buffer)
    std::ifstream inputStream(subtreeBinary, std::ios_base::binary);
    std::vector<unsigned char> buffer(std::istreambuf_iterator<char>(inputStream), {});
    uint32_t jsonStringByteLength = buffer[8];
    uint32_t binaryByteLength = static_cast<uint32_t>(buffer.size());
    REQUIRE(binaryByteLength - headerByteLength - jsonStringByteLength == nodeAvailabilityByteLengthWithPadding);
  }

}

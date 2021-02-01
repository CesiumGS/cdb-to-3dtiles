#include "CDBGeoCell.h"
#include "CDBTile.h"
#include "CDBTo3DTiles.h"
#include "Config.h"
#include "catch2/catch.hpp"
#include "glm/glm.hpp"
#include "nlohmann/json.hpp"
#include <fstream>
#include "CDBElevation.h"

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

TEST_CASE("Test converter errors out of 3D Tiles Next conversion with uninitialized availabilty buffer.", "[CombineTilesets]")
{
  std::filesystem::path input = dataPath / "CombineTilesets";
  std::filesystem::path output = "CombineTilesets";
  std::filesystem::path elevationTilePath = dataPath / "CombineTilesets" / "Elevation" / "N34W119_D001_S001_T001_L06_U0_R0.tif";

  std::unique_ptr<ConverterImpl> m_impl = std::make_unique<ConverterImpl>(input, output);
  // std::optional<CDBElevation> elevation = CDBElevation::createFromFile(elevationTilePath);
  std::optional<CDBElevation> elevation = CDBElevation::createFromFile(elevationTilePath);
  uint8_t* nullPointer = NULL;
  REQUIRE_THROWS_AS(m_impl->addElevationTileAvailability(*elevation, nullPointer), std::invalid_argument);
}

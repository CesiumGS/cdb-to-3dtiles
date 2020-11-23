#include "CDBGeoCell.h"
#include "CDBTile.h"
#include "CDBTo3DTiles.h"
#include "Config.h"
#include "catch2/catch.hpp"
#include "glm/glm.hpp"
#include "nlohmann/json.hpp"
#include <fstream>

using namespace CDBTo3DTiles;

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
        nlohmann::json elevationJson = nlohmann::json::parse(elevationFs);

        auto geoCell = CDBGeoCell(32, -119);
        auto boundRegion = CDBTile::calcBoundRegion(geoCell, -10, 0, 0);
        const auto &boundRectangle = boundRegion.getRectangle();
        double boundNorth = boundRectangle.getNorth();
        double boundWest = boundRectangle.getWest();
        double boundSouth = boundRectangle.getSouth();
        double boundEast = boundRectangle.getEast();
        REQUIRE(elevationJson["root"]["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(elevationJson["root"]["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(elevationJson["root"]["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(elevationJson["root"]["boundingVolume"]["region"][3] == Approx(boundNorth));

        auto child = elevationJson["root"]["children"][0];
        REQUIRE(child["content"]["uri"]
                == (geoCell.getRelativePath() / "Elevation" / "1_1" / "tileset.json").string());
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
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][3] == Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        REQUIRE(child["content"]["uri"]
                == (geoCell.getRelativePath() / "GTModels" / "2_1" / "tileset.json").string());
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
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][3] == Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        REQUIRE(child["content"]["uri"]
                == (geoCell.getRelativePath() / "GTModels" / "1_1" / "tileset.json").string());
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
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(tilesetJson["root"]["boundingVolume"]["region"][3] == Approx(boundNorth));

        auto child = tilesetJson["root"]["children"][0];
        REQUIRE(child["content"]["uri"]
                == (geoCell.getRelativePath() / "RoadNetwork" / "2_3" / "tileset.json").string());
        REQUIRE(child["boundingVolume"]["region"][0] == Approx(boundWest));
        REQUIRE(child["boundingVolume"]["region"][1] == Approx(boundSouth));
        REQUIRE(child["boundingVolume"]["region"][2] == Approx(boundEast));
        REQUIRE(child["boundingVolume"]["region"][3] == Approx(boundNorth));
    }

    std::filesystem::remove_all(output);
}

TEST_CASE("Test converer combine requested dataset", "[CombineTilesets]")
{
    std::filesystem::path input = dataPath / "CombineTilesets";
    std::filesystem::path output = "CombineTilesets";

    Converter converter(input, output);
    converter.combineDataset({"Elevation_1_1", "RoadNetwork_2_3", "GTModels_1_1"});
    converter.convert();

    std::filesystem::path tilesetOutput = output / "Elevation_1_1RoadNetwork_2_3GTModels_1_1.json";
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

    std::filesystem::remove_all(output);
}

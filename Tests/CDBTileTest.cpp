#include "BoundingRegion.h"
#include "CDBTileset.h"
#include "Utility.h"
#include "catch2/catch.hpp"
#include "glm/glm.hpp"

using namespace CDBTo3DTiles;
using namespace Core;

static void checkGeoCellExtent(const CDBGeoCell &geoCell, const GlobeRectangle &rectangeToTest)
{
    double geoCellLongitude = static_cast<double>(geoCell.getLongitude());
    double geoCellLatitude = static_cast<double>(geoCell.getLatitude());
    double boundWest = glm::radians(geoCellLongitude);
    double boundSouth = glm::radians(geoCellLatitude);
    double boundEast = glm::radians(geoCellLongitude + geoCell.getLongitudeExtentInDegree());
    double boundNorth = glm::radians(geoCellLatitude + geoCell.getLatitudeExtentInDegree());
    REQUIRE(rectangeToTest.getWest() == Approx(boundWest));
    REQUIRE(rectangeToTest.getSouth() == Approx(boundSouth));
    REQUIRE(rectangeToTest.getEast() == Approx(boundEast));
    REQUIRE(rectangeToTest.getNorth() == Approx(boundNorth));
}

TEST_CASE("Test CDBTile constructor", "[CDBTile]")
{
    SECTION("Test valid constructor")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);

        double boundNorth = glm::radians(32.0 + geoCell.getLatitudeExtentInDegree());
        double boundWest = glm::radians(-118.0);
        double boundSouth = glm::radians(32.0);
        double boundEast = glm::radians(-118.0 + geoCell.getLongitudeExtentInDegree());
        BoundingRegion region = tile.getBoundRegion();
        GlobeRectangle rectangle = region.getRectangle();
        REQUIRE(rectangle.getNorth() == Approx(boundNorth));
        REQUIRE(rectangle.getWest() == Approx(boundWest));
        REQUIRE(rectangle.getSouth() == Approx(boundSouth));
        REQUIRE(rectangle.getEast() == Approx(boundEast));
        REQUIRE(region.getMinimumHeight() == Approx(0.0));
        REQUIRE(region.getMaximumHeight() == Approx(0.0));

        REQUIRE(tile.getRelativePath()
                == "Tiles/N32/W118/001_Elevation/LC/U0/N32W118_D001_S001_T002_LC10_U0_R0");
        REQUIRE(tile.getGeoCell() == geoCell);
        REQUIRE(tile.getDataset() == CDBDataset::Elevation);
        REQUIRE(tile.getCS_1() == 1);
        REQUIRE(tile.getCS_2() == 2);
        REQUIRE(tile.getLevel() == -10);
        REQUIRE(tile.getUREF() == 0);
        REQUIRE(tile.getRREF() == 0);
        REQUIRE(tile.getChildren().size() == 0);
        REQUIRE(tile.getCustomContentURI() == nullptr);
    }

    SECTION("Test below -10 level invalid argument")
    {
        CDBGeoCell geoCell(32, -118);
        REQUIRE_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, -11, 0, 0), std::invalid_argument);
    }

    SECTION("Test above 23 level invalid argument")
    {
        CDBGeoCell geoCell(32, -118);
        REQUIRE_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, 24, 0, 0), std::invalid_argument);
    }

    SECTION("Test negative level with UREF and RREF not 0 invalid argument")
    {
        CDBGeoCell geoCell(32, -118);
        REQUIRE_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, -9, 1, 0), std::invalid_argument);
        REQUIRE_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, -9, 0, 1), std::invalid_argument);
    }

    SECTION("Test positive level with negative UREF and RREF invalid argument")
    {
        CDBGeoCell geoCell(32, -118);
        REQUIRE_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, 9, -1, 0), std::invalid_argument);
        REQUIRE_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, 9, 0, -1), std::invalid_argument);
    }

    SECTION("Test positive level with UREF and RREF out of range invalid argument")
    {
        CDBGeoCell geoCell(32, -118);

        for (unsigned level = 0; level <= 23; ++level) {
            REQUIRE_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, level, 1 << level, 0),
                              std::invalid_argument);
            REQUIRE_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, level, 0, 1 << level),
                              std::invalid_argument);
        }
    }
}

TEST_CASE("Test CDBTile copy constructor", "[CDBTile]")
{
    // Create a child tile for tested tile below
    CDBGeoCell geoCell(32, -118);
    std::unique_ptr<CDBTile> child = std::make_unique<CDBTile>(geoCell, CDBDataset::Elevation, 1, 2, -9, 0, 0);

    // Create parent tile. Test to make sure it has good properties
    CDBTile tile(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);
    tile.getChildren().emplace_back(child.get());

    double boundNorth = glm::radians(32.0 + geoCell.getLatitudeExtentInDegree());
    double boundWest = glm::radians(-118.0);
    double boundSouth = glm::radians(32.0);
    double boundEast = glm::radians(-118.0 + geoCell.getLongitudeExtentInDegree());
    const BoundingRegion &region = tile.getBoundRegion();
    const GlobeRectangle &rectangle = region.getRectangle();
    REQUIRE(rectangle.getNorth() == Approx(boundNorth));
    REQUIRE(rectangle.getWest() == Approx(boundWest));
    REQUIRE(rectangle.getSouth() == Approx(boundSouth));
    REQUIRE(rectangle.getEast() == Approx(boundEast));
    REQUIRE(region.getMinimumHeight() == Approx(0.0));
    REQUIRE(region.getMaximumHeight() == Approx(0.0));

    REQUIRE(tile.getRelativePath() == "Tiles/N32/W118/001_Elevation/LC/U0/N32W118_D001_S001_T002_LC10_U0_R0");
    REQUIRE(tile.getGeoCell() == geoCell);
    REQUIRE(tile.getDataset() == CDBDataset::Elevation);
    REQUIRE(tile.getCS_1() == 1);
    REQUIRE(tile.getCS_2() == 2);
    REQUIRE(tile.getLevel() == -10);
    REQUIRE(tile.getUREF() == 0);
    REQUIRE(tile.getRREF() == 0);
    REQUIRE(tile.getChildren().front() == child.get());
    REQUIRE(tile.getCustomContentURI() == nullptr);

    // Create copy tile. Make sure all the properties are the same except children
    CDBTile copiedTile = tile;
    const BoundingRegion &copiedRegion = copiedTile.getBoundRegion();
    const GlobeRectangle &copiedRectangle = copiedRegion.getRectangle();
    REQUIRE(copiedRectangle.getNorth() == Approx(boundNorth));
    REQUIRE(copiedRectangle.getWest() == Approx(boundWest));
    REQUIRE(copiedRectangle.getSouth() == Approx(boundSouth));
    REQUIRE(copiedRectangle.getEast() == Approx(boundEast));
    REQUIRE(copiedRegion.getMinimumHeight() == Approx(0.0));
    REQUIRE(copiedRegion.getMaximumHeight() == Approx(0.0));

    REQUIRE(copiedTile.getRelativePath()
            == "Tiles/N32/W118/001_Elevation/LC/U0/N32W118_D001_S001_T002_LC10_U0_R0");
    REQUIRE(copiedTile.getGeoCell() == geoCell);
    REQUIRE(copiedTile.getDataset() == CDBDataset::Elevation);
    REQUIRE(copiedTile.getCS_1() == 1);
    REQUIRE(copiedTile.getCS_2() == 2);
    REQUIRE(copiedTile.getLevel() == -10);
    REQUIRE(copiedTile.getUREF() == 0);
    REQUIRE(copiedTile.getRREF() == 0);
    REQUIRE(copiedTile.getChildren().size() == 0);
    REQUIRE(copiedTile.getCustomContentURI() == nullptr);
}

TEST_CASE("Test CDBTile move constructor", "[CDBTile]")
{
    // Create a child tile for tested tile below
    CDBGeoCell geoCell(32, -118);
    std::unique_ptr<CDBTile> child = std::make_unique<CDBTile>(geoCell, CDBDataset::Elevation, 1, 2, -9, 0, 0);

    // Create parent tile.
    CDBTile tile(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);
    tile.getChildren().emplace_back(child.get());

    // Move tile to the new one. Make sure the moved tile has all the property of the previous tile
    CDBTile movedTile(std::move(tile));

    double boundNorth = glm::radians(32.0 + geoCell.getLatitudeExtentInDegree());
    double boundWest = glm::radians(-118.0);
    double boundSouth = glm::radians(32.0);
    double boundEast = glm::radians(-118.0 + geoCell.getLongitudeExtentInDegree());
    const BoundingRegion &movedRegion = movedTile.getBoundRegion();
    const GlobeRectangle &movedRectangle = movedRegion.getRectangle();
    REQUIRE(movedRectangle.getNorth() == Approx(boundNorth));
    REQUIRE(movedRectangle.getWest() == Approx(boundWest));
    REQUIRE(movedRectangle.getSouth() == Approx(boundSouth));
    REQUIRE(movedRectangle.getEast() == Approx(boundEast));
    REQUIRE(movedRegion.getMinimumHeight() == Approx(0.0));
    REQUIRE(movedRegion.getMaximumHeight() == Approx(0.0));

    REQUIRE(movedTile.getRelativePath()
            == "Tiles/N32/W118/001_Elevation/LC/U0/N32W118_D001_S001_T002_LC10_U0_R0");
    REQUIRE(movedTile.getGeoCell() == geoCell);
    REQUIRE(movedTile.getDataset() == CDBDataset::Elevation);
    REQUIRE(movedTile.getCS_1() == 1);
    REQUIRE(movedTile.getCS_2() == 2);
    REQUIRE(movedTile.getLevel() == -10);
    REQUIRE(movedTile.getUREF() == 0);
    REQUIRE(movedTile.getRREF() == 0);
    REQUIRE(movedTile.getChildren().front() == child.get());
    REQUIRE(movedTile.getCustomContentURI() == nullptr);
}

TEST_CASE("Test CDBTile copy assignment", "[CDBTile]")
{
    // Create a child tile for tested tile below
    CDBGeoCell geoCell(32, -118);
    std::unique_ptr<CDBTile> child = std::make_unique<CDBTile>(geoCell, CDBDataset::Elevation, 1, 2, -9, 0, 0);

    // Create parent tile.
    CDBTile tile(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);
    tile.getChildren().emplace_back(child.get());

    // copy tile to the new one. Make sure the copied tile don't have any children
    CDBTile copiedTile(geoCell, CDBDataset::Elevation, 1, 1, -8, 0, 0);
    copiedTile = tile;

    double boundNorth = glm::radians(32.0 + geoCell.getLatitudeExtentInDegree());
    double boundWest = glm::radians(-118.0);
    double boundSouth = glm::radians(32.0);
    double boundEast = glm::radians(-118.0 + geoCell.getLongitudeExtentInDegree());
    const BoundingRegion &movedRegion = copiedTile.getBoundRegion();
    const GlobeRectangle &movedRectangle = movedRegion.getRectangle();
    REQUIRE(movedRectangle.getNorth() == Approx(boundNorth));
    REQUIRE(movedRectangle.getWest() == Approx(boundWest));
    REQUIRE(movedRectangle.getSouth() == Approx(boundSouth));
    REQUIRE(movedRectangle.getEast() == Approx(boundEast));
    REQUIRE(movedRegion.getMinimumHeight() == Approx(0.0));
    REQUIRE(movedRegion.getMaximumHeight() == Approx(0.0));

    REQUIRE(copiedTile.getRelativePath()
            == "Tiles/N32/W118/001_Elevation/LC/U0/N32W118_D001_S001_T002_LC10_U0_R0");
    REQUIRE(copiedTile.getGeoCell() == geoCell);
    REQUIRE(copiedTile.getDataset() == CDBDataset::Elevation);
    REQUIRE(copiedTile.getCS_1() == 1);
    REQUIRE(copiedTile.getCS_2() == 2);
    REQUIRE(copiedTile.getLevel() == -10);
    REQUIRE(copiedTile.getUREF() == 0);
    REQUIRE(copiedTile.getRREF() == 0);
    REQUIRE(copiedTile.getChildren().size() == 0);
    REQUIRE(copiedTile.getCustomContentURI() == nullptr);
}

TEST_CASE("Test CDBTile move assignment", "[CDBTile]")
{
    // Create a child tile for tested tile below
    CDBGeoCell geoCell(32, -118);
    std::unique_ptr<CDBTile> child = std::make_unique<CDBTile>(geoCell, CDBDataset::Elevation, 1, 2, -9, 0, 0);

    // Create parent tile.
    CDBTile tile(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);
    tile.getChildren().emplace_back(child.get());

    // Move tile to the new one. Make sure the moved tile has all the property of the previous tile
    CDBTile movedTile(geoCell, CDBDataset::Elevation, 1, 1, -8, 0, 0);
    movedTile = std::move(tile);

    double boundNorth = glm::radians(32.0 + geoCell.getLatitudeExtentInDegree());
    double boundWest = glm::radians(-118.0);
    double boundSouth = glm::radians(32.0);
    double boundEast = glm::radians(-118.0 + geoCell.getLongitudeExtentInDegree());
    const BoundingRegion &movedRegion = movedTile.getBoundRegion();
    const GlobeRectangle &movedRectangle = movedRegion.getRectangle();
    REQUIRE(movedRectangle.getNorth() == Approx(boundNorth));
    REQUIRE(movedRectangle.getWest() == Approx(boundWest));
    REQUIRE(movedRectangle.getSouth() == Approx(boundSouth));
    REQUIRE(movedRectangle.getEast() == Approx(boundEast));
    REQUIRE(movedRegion.getMinimumHeight() == Approx(0.0));
    REQUIRE(movedRegion.getMaximumHeight() == Approx(0.0));

    REQUIRE(movedTile.getRelativePath()
            == "Tiles/N32/W118/001_Elevation/LC/U0/N32W118_D001_S001_T002_LC10_U0_R0");
    REQUIRE(movedTile.getGeoCell() == geoCell);
    REQUIRE(movedTile.getDataset() == CDBDataset::Elevation);
    REQUIRE(movedTile.getCS_1() == 1);
    REQUIRE(movedTile.getCS_2() == 2);
    REQUIRE(movedTile.getLevel() == -10);
    REQUIRE(movedTile.getUREF() == 0);
    REQUIRE(movedTile.getRREF() == 0);
    REQUIRE(movedTile.getChildren().front() == child.get());
    REQUIRE(movedTile.getCustomContentURI() == nullptr);
}

TEST_CASE("Test CDBTile equal operator", "[CDBTile]")
{
    SECTION("rhs and lhs are equals")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);
        CDBTile rhs(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);
        REQUIRE(lhs == rhs);
    }

    SECTION("rhs and lhs are different in GeoCell")
    {
        CDBGeoCell lhsGeoCell(32, -118);
        CDBTile lhs(lhsGeoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);

        CDBGeoCell rhsGeoCell(40, -118);
        CDBTile rhs(rhsGeoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);

        REQUIRE_FALSE(lhs == rhs);
    }

    SECTION("rhs and lhs are different in dataset")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);
        CDBTile rhs(geoCell, CDBDataset::GSFeature, 1, 2, -10, 0, 0);

        REQUIRE_FALSE(lhs == rhs);
    }

    SECTION("rhs and lhs are different in Component Selector 1")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 2, 2, -10, 0, 0);
        CDBTile rhs(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);

        REQUIRE_FALSE(lhs == rhs);
    }

    SECTION("rhs and lhs are different in Component Selector 2")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 1, 3, -10, 0, 0);
        CDBTile rhs(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);

        REQUIRE_FALSE(lhs == rhs);
    }

    SECTION("rhs and lhs are different in level")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 1, 2, 0, 0, 0);
        CDBTile rhs(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);

        REQUIRE_FALSE(lhs == rhs);
    }

    SECTION("rhs and lhs are different in UREF")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 1, 2, 2, 2, 0);
        CDBTile rhs(geoCell, CDBDataset::Elevation, 1, 2, 2, 0, 0);

        REQUIRE_FALSE(lhs == rhs);
    }

    SECTION("rhs and lhs are different in RREF")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 1, 2, 2, 2, 1);
        CDBTile rhs(geoCell, CDBDataset::Elevation, 1, 2, 2, 2, 0);

        REQUIRE_FALSE(lhs == rhs);
    }
}

TEST_CASE("Test create parent for a given CDBTile", "[CDBTile]")
{
    SECTION("Test negative LOD tile")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, -8, 0, 0);
        auto parent = CDBTile::createParentTile(tile);
        REQUIRE(parent != std::nullopt);
        REQUIRE(parent->getGeoCell() == geoCell);
        REQUIRE(parent->getDataset() == CDBDataset::Elevation);
        REQUIRE(parent->getCS_1() == 1);
        REQUIRE(parent->getCS_2() == 1);
        REQUIRE(parent->getLevel() == -9);
        REQUIRE(parent->getUREF() == 0);
        REQUIRE(parent->getRREF() == 0);
    }

    SECTION("Test positive LOD tile")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 2, 2, 3);
        auto parent = CDBTile::createParentTile(tile);
        REQUIRE(parent != std::nullopt);
        REQUIRE(parent->getGeoCell() == geoCell);
        REQUIRE(parent->getDataset() == CDBDataset::Elevation);
        REQUIRE(parent->getCS_1() == 1);
        REQUIRE(parent->getCS_2() == 1);
        REQUIRE(parent->getLevel() == 1);
        REQUIRE(parent->getUREF() == 1);
        REQUIRE(parent->getRREF() == 1);
    }

    SECTION("Test LOD -10 tile")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, -10, 0, 0);
        auto parent = CDBTile::createParentTile(tile);
        REQUIRE(parent == std::nullopt);
    }
}

TEST_CASE("Test create child tile for negative level tile", "[CDBTile]")
{
    SECTION("Test valid negative level tile")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, -1, 0, 0);
        auto child = CDBTile::createChildForNegativeLOD(tile);
        REQUIRE(child.getGeoCell() == geoCell);
        REQUIRE(child.getDataset() == CDBDataset::Elevation);
        REQUIRE(child.getCS_1() == 1);
        REQUIRE(child.getCS_2() == 1);
        REQUIRE(child.getLevel() == 0);
        REQUIRE(child.getUREF() == 0);
        REQUIRE(child.getRREF() == 0);
    }

    SECTION("Test invalid positive level tile")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        REQUIRE_THROWS_AS(CDBTile::createChildForNegativeLOD(tile), std::invalid_argument);
    }
}

TEST_CASE("Test create child tile for positive level tile", "[CDBTile]")
{
    // caution quadtree in CDB with index (0, 0) begins from bottom left, not top left
    SECTION("Test north west child")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 2, 2, 2);
        auto child = CDBTile::createNorthWestForPositiveLOD(tile);
        REQUIRE(child.getGeoCell() == geoCell);
        REQUIRE(child.getDataset() == CDBDataset::Elevation);
        REQUIRE(child.getCS_1() == 1);
        REQUIRE(child.getCS_2() == 1);
        REQUIRE(child.getLevel() == 3);
        REQUIRE(child.getUREF() == 5);
        REQUIRE(child.getRREF() == 4);

        CDBTile negativeTile(geoCell, CDBDataset::Elevation, 1, 1, -1, 0, 0);
        REQUIRE_THROWS_AS(CDBTile::createNorthWestForPositiveLOD(negativeTile), std::invalid_argument);
    }

    SECTION("Test north east child")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 2, 2, 2);
        auto child = CDBTile::createNorthEastForPositiveLOD(tile);
        REQUIRE(child.getGeoCell() == geoCell);
        REQUIRE(child.getDataset() == CDBDataset::Elevation);
        REQUIRE(child.getCS_1() == 1);
        REQUIRE(child.getCS_2() == 1);
        REQUIRE(child.getLevel() == 3);
        REQUIRE(child.getUREF() == 5);
        REQUIRE(child.getRREF() == 5);

        CDBTile negativeTile(geoCell, CDBDataset::Elevation, 1, 1, -1, 0, 0);
        REQUIRE_THROWS_AS(CDBTile::createNorthEastForPositiveLOD(negativeTile), std::invalid_argument);
    }

    SECTION("Test south west child")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 2, 2, 2);
        auto child = CDBTile::createSouthWestForPositiveLOD(tile);
        REQUIRE(child.getGeoCell() == geoCell);
        REQUIRE(child.getDataset() == CDBDataset::Elevation);
        REQUIRE(child.getCS_1() == 1);
        REQUIRE(child.getCS_2() == 1);
        REQUIRE(child.getLevel() == 3);
        REQUIRE(child.getUREF() == 4);
        REQUIRE(child.getRREF() == 4);

        CDBTile negativeTile(geoCell, CDBDataset::Elevation, 1, 1, -1, 0, 0);
        REQUIRE_THROWS_AS(CDBTile::createSouthWestForPositiveLOD(negativeTile), std::invalid_argument);
    }

    SECTION("Test south east child")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 2, 2, 2);
        auto child = CDBTile::createSouthEastForPositiveLOD(tile);
        REQUIRE(child.getGeoCell() == geoCell);
        REQUIRE(child.getDataset() == CDBDataset::Elevation);
        REQUIRE(child.getCS_1() == 1);
        REQUIRE(child.getCS_2() == 1);
        REQUIRE(child.getLevel() == 3);
        REQUIRE(child.getUREF() == 4);
        REQUIRE(child.getRREF() == 5);

        CDBTile negativeTile(geoCell, CDBDataset::Elevation, 1, 1, -1, 0, 0);
        REQUIRE_THROWS_AS(CDBTile::createSouthEastForPositiveLOD(negativeTile), std::invalid_argument);
    }
}

TEST_CASE("Test create from file", "[CDBTile]")
{
    SECTION("Test create from valid file name with positive level")
    {
        CDBGeoCell geoCell(32, -118);
        auto tile = CDBTile::createFromFile("N32W118_D001_S001_T001_L03_U5_R5");
        REQUIRE(tile != std::nullopt);
        REQUIRE(tile->getGeoCell() == geoCell);
        REQUIRE(tile->getDataset() == CDBDataset::Elevation);
        REQUIRE(tile->getCS_1() == 1);
        REQUIRE(tile->getCS_2() == 1);
        REQUIRE(tile->getLevel() == 3);
        REQUIRE(tile->getUREF() == 5);
        REQUIRE(tile->getRREF() == 5);
    }

    SECTION("Test create from valid file name with negative level")
    {
        CDBGeoCell geoCell(32, -118);
        auto tile = CDBTile::createFromFile("N32W118_D001_S001_T001_LC03_U0_R0");
        REQUIRE(tile != std::nullopt);
        REQUIRE(tile->getGeoCell() == geoCell);
        REQUIRE(tile->getDataset() == CDBDataset::Elevation);
        REQUIRE(tile->getCS_1() == 1);
        REQUIRE(tile->getCS_2() == 1);
        REQUIRE(tile->getLevel() == -3);
        REQUIRE(tile->getUREF() == 0);
        REQUIRE(tile->getRREF() == 0);
    }

    SECTION("Test create from invalid latitude")
    {
        auto tile = CDBTile::createFromFile("M32W118_D001_S001_T001_L03_U5_R5");
        REQUIRE(tile == std::nullopt);
    }

    SECTION("Test create from out of bound latitude")
    {
        REQUIRE(CDBTile::createFromFile("N90W118_D001_S001_T001_L03_U5_R5") == std::nullopt);
        REQUIRE(CDBTile::createFromFile("S91W118_D001_S001_T001_L03_U5_R5") == std::nullopt);
    }

    SECTION("Test create from invalid longitude")
    {
        auto tile = CDBTile::createFromFile("N32M118_D001_S001_T001_L03_U5_R5");
        REQUIRE(tile == std::nullopt);
    }

    SECTION("Test create from out of bound longitude")
    {
        REQUIRE(CDBTile::createFromFile("N32W181_D001_S001_T001_L03_U5_R5") == std::nullopt);
        REQUIRE(CDBTile::createFromFile("S32E181_D001_S001_T001_L03_U5_R5") == std::nullopt);
    }

    SECTION("Test create from invalid dataset")
    {
        auto tile = CDBTile::createFromFile("N32W118_D12_S001_T001_L03_U5_R5");
        REQUIRE(tile == std::nullopt);
    }

    SECTION("Test create from invalid component selector 1")
    {
        auto tile = CDBTile::createFromFile("N32W118_D12_ST1_T001_L03_U5_R5");
        REQUIRE(tile == std::nullopt);
    }

    SECTION("Test create from invalid component selector 2")
    {
        auto tile = CDBTile::createFromFile("N32W118_D12_S001_T00S1_L03_U5_R5");
        REQUIRE(tile == std::nullopt);
    }

    SECTION("Test create from invalid level")
    {
        auto belowNeg10Level = CDBTile::createFromFile("N32W118_D001_S001_T001_LC13_U5_R5");
        REQUIRE(belowNeg10Level == std::nullopt);

        auto above23Level = CDBTile::createFromFile("N32W118_D001_S001_T001_L24_U5_R5");
        REQUIRE(above23Level == std::nullopt);
    }

    SECTION("Test create from UREF and RREF for negative level")
    {
        auto tile = CDBTile::createFromFile("N32W118_D001_S001_T001_LC04_U5_R0");
        REQUIRE(tile == std::nullopt);

        tile = CDBTile::createFromFile("N32W118_D001_S001_T001_LC04_U0_R4");
        REQUIRE(tile == std::nullopt);
    }

    SECTION("Test create from invalid UREF and RREF for positive level")
    {
        for (int i = 0; i <= 23; ++i) {
            std::string UREFFilename = "N32W118_D001_S001_T001_L" + toStringWithZeroPadding(2, i) + "_U"
                                       + std::to_string(1 << i) + "_R0";
            REQUIRE(CDBTile::createFromFile(UREFFilename) == std::nullopt);

            std::string RREFFilename = "N32W118_D001_S001_T001_L" + toStringWithZeroPadding(2, i) + "_U0_R"
                                       + std::to_string(1 << i);
            REQUIRE(CDBTile::createFromFile(RREFFilename) == std::nullopt);
        }
    }

    SECTION("Test create from nonsense file")
    {
        auto tile = CDBTile::createFromFile("asas_N32_asasas_12_asss");
        REQUIRE(tile == std::nullopt);
    }
}

TEST_CASE("Test bounding regions", "[CDBTile]")
{
    SECTION("tile at zone 10")
    {
        CDBGeoCell geoCell(89, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SECTION("tile at region 9")
    {
        CDBGeoCell geoCell(80, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SECTION("tile at region 8")
    {
        CDBGeoCell geoCell(75, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SECTION("tile at region 7")
    {
        CDBGeoCell geoCell(70, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SECTION("tile at region 6")
    {
        CDBGeoCell geoCell(50, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SECTION("tile at region 5")
    {
        CDBGeoCell geoCell(-50, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SECTION("tile at region 4")
    {
        CDBGeoCell geoCell(-70, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SECTION("tile at region 3")
    {
        CDBGeoCell geoCell(-75, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SECTION("tile at region 2")
    {
        CDBGeoCell geoCell(-80, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SECTION("tile at region 1")
    {
        CDBGeoCell geoCell(-89, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SECTION("tile at region 0")
    {
        CDBGeoCell geoCell(-90, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }
}

TEST_CASE("Test retrieving GeoCell and Dataset from tile name", "[CDBTile]")
{
    CDBGeoCell geoCell(32, -118);
    CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
    REQUIRE(CDBTile::retrieveGeoCellDatasetFromTileName(tile) == "N32W118_D001_S001_T001");
}

#include "BoundingRegion.h"
#include "CDBTileset.h"
#include "Utility.h"
#include <doctest/doctest.h>
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
    CHECK(rectangeToTest.getWest() == doctest::Approx(boundWest));
    CHECK(rectangeToTest.getSouth() == doctest::Approx(boundSouth));
    CHECK(rectangeToTest.getEast() == doctest::Approx(boundEast));
    CHECK(rectangeToTest.getNorth() == doctest::Approx(boundNorth));
}

TEST_SUITE_BEGIN("CDBTile");

TEST_CASE("Test CDBTile constructor")
{
    SUBCASE("Test valid constructor")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);

        double boundNorth = glm::radians(32.0 + geoCell.getLatitudeExtentInDegree());
        double boundWest = glm::radians(-118.0);
        double boundSouth = glm::radians(32.0);
        double boundEast = glm::radians(-118.0 + geoCell.getLongitudeExtentInDegree());
        BoundingRegion region = tile.getBoundRegion();
        GlobeRectangle rectangle = region.getRectangle();
        CHECK(rectangle.getNorth() == doctest::Approx(boundNorth));
        CHECK(rectangle.getWest() == doctest::Approx(boundWest));
        CHECK(rectangle.getSouth() == doctest::Approx(boundSouth));
        CHECK(rectangle.getEast() == doctest::Approx(boundEast));
        CHECK(region.getMinimumHeight() == doctest::Approx(0.0));
        CHECK(region.getMaximumHeight() == doctest::Approx(0.0));

        CHECK(tile.getRelativePath()
                == "Tiles/N32/W118/001_Elevation/LC/U0/N32W118_D001_S001_T002_LC10_U0_R0");
        CHECK(tile.getGeoCell() == geoCell);
        CHECK(tile.getDataset() == CDBDataset::Elevation);
        CHECK(tile.getCS_1() == 1);
        CHECK(tile.getCS_2() == 2);
        CHECK(tile.getLevel() == -10);
        CHECK(tile.getUREF() == 0);
        CHECK(tile.getRREF() == 0);
        CHECK(tile.getChildren().size() == 0);
        CHECK(tile.getCustomContentURI() == nullptr);
    }

    SUBCASE("Test below -10 level invalid argument")
    {
        CDBGeoCell geoCell(32, -118);
        CHECK_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, -11, 0, 0), std::invalid_argument);
    }

    SUBCASE("Test above 23 level invalid argument")
    {
        CDBGeoCell geoCell(32, -118);
        CHECK_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, 24, 0, 0), std::invalid_argument);
    }

    SUBCASE("Test negative level with UREF and RREF not 0 invalid argument")
    {
        CDBGeoCell geoCell(32, -118);
        CHECK_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, -9, 1, 0), std::invalid_argument);
        CHECK_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, -9, 0, 1), std::invalid_argument);
    }

    SUBCASE("Test positive level with negative UREF and RREF invalid argument")
    {
        CDBGeoCell geoCell(32, -118);
        CHECK_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, 9, -1, 0), std::invalid_argument);
        CHECK_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, 9, 0, -1), std::invalid_argument);
    }

    SUBCASE("Test positive level with UREF and RREF out of range invalid argument")
    {
        CDBGeoCell geoCell(32, -118);

        for (unsigned level = 0; level <= 23; ++level) {
            CHECK_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, level, 1 << level, 0),
                              std::invalid_argument);
            CHECK_THROWS_AS(CDBTile(geoCell, CDBDataset::Elevation, 1, 2, level, 0, 1 << level),
                              std::invalid_argument);
        }
    }
}

TEST_CASE("Test CDBTile copy constructor")
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
    CHECK(rectangle.getNorth() == doctest::Approx(boundNorth));
    CHECK(rectangle.getWest() == doctest::Approx(boundWest));
    CHECK(rectangle.getSouth() == doctest::Approx(boundSouth));
    CHECK(rectangle.getEast() == doctest::Approx(boundEast));
    CHECK(region.getMinimumHeight() == doctest::Approx(0.0));
    CHECK(region.getMaximumHeight() == doctest::Approx(0.0));

    CHECK(tile.getRelativePath() == "Tiles/N32/W118/001_Elevation/LC/U0/N32W118_D001_S001_T002_LC10_U0_R0");
    CHECK(tile.getGeoCell() == geoCell);
    CHECK(tile.getDataset() == CDBDataset::Elevation);
    CHECK(tile.getCS_1() == 1);
    CHECK(tile.getCS_2() == 2);
    CHECK(tile.getLevel() == -10);
    CHECK(tile.getUREF() == 0);
    CHECK(tile.getRREF() == 0);
    CHECK(tile.getChildren().front() == child.get());
    CHECK(tile.getCustomContentURI() == nullptr);

    // Create copy tile. Make sure all the properties are the same except children
    CDBTile copiedTile = tile;
    const BoundingRegion &copiedRegion = copiedTile.getBoundRegion();
    const GlobeRectangle &copiedRectangle = copiedRegion.getRectangle();
    CHECK(copiedRectangle.getNorth() == doctest::Approx(boundNorth));
    CHECK(copiedRectangle.getWest() == doctest::Approx(boundWest));
    CHECK(copiedRectangle.getSouth() == doctest::Approx(boundSouth));
    CHECK(copiedRectangle.getEast() == doctest::Approx(boundEast));
    CHECK(copiedRegion.getMinimumHeight() == doctest::Approx(0.0));
    CHECK(copiedRegion.getMaximumHeight() == doctest::Approx(0.0));

    CHECK(copiedTile.getRelativePath()
            == "Tiles/N32/W118/001_Elevation/LC/U0/N32W118_D001_S001_T002_LC10_U0_R0");
    CHECK(copiedTile.getGeoCell() == geoCell);
    CHECK(copiedTile.getDataset() == CDBDataset::Elevation);
    CHECK(copiedTile.getCS_1() == 1);
    CHECK(copiedTile.getCS_2() == 2);
    CHECK(copiedTile.getLevel() == -10);
    CHECK(copiedTile.getUREF() == 0);
    CHECK(copiedTile.getRREF() == 0);
    CHECK(copiedTile.getChildren().size() == 0);
    CHECK(copiedTile.getCustomContentURI() == nullptr);
}

TEST_CASE("Test CDBTile move constructor")
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
    CHECK(movedRectangle.getNorth() == doctest::Approx(boundNorth));
    CHECK(movedRectangle.getWest() == doctest::Approx(boundWest));
    CHECK(movedRectangle.getSouth() == doctest::Approx(boundSouth));
    CHECK(movedRectangle.getEast() == doctest::Approx(boundEast));
    CHECK(movedRegion.getMinimumHeight() == doctest::Approx(0.0));
    CHECK(movedRegion.getMaximumHeight() == doctest::Approx(0.0));

    CHECK(movedTile.getRelativePath()
            == "Tiles/N32/W118/001_Elevation/LC/U0/N32W118_D001_S001_T002_LC10_U0_R0");
    CHECK(movedTile.getGeoCell() == geoCell);
    CHECK(movedTile.getDataset() == CDBDataset::Elevation);
    CHECK(movedTile.getCS_1() == 1);
    CHECK(movedTile.getCS_2() == 2);
    CHECK(movedTile.getLevel() == -10);
    CHECK(movedTile.getUREF() == 0);
    CHECK(movedTile.getRREF() == 0);
    CHECK(movedTile.getChildren().front() == child.get());
    CHECK(movedTile.getCustomContentURI() == nullptr);
}

TEST_CASE("Test CDBTile copy assignment")
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
    CHECK(movedRectangle.getNorth() == doctest::Approx(boundNorth));
    CHECK(movedRectangle.getWest() == doctest::Approx(boundWest));
    CHECK(movedRectangle.getSouth() == doctest::Approx(boundSouth));
    CHECK(movedRectangle.getEast() == doctest::Approx(boundEast));
    CHECK(movedRegion.getMinimumHeight() == doctest::Approx(0.0));
    CHECK(movedRegion.getMaximumHeight() == doctest::Approx(0.0));

    CHECK(copiedTile.getRelativePath()
            == "Tiles/N32/W118/001_Elevation/LC/U0/N32W118_D001_S001_T002_LC10_U0_R0");
    CHECK(copiedTile.getGeoCell() == geoCell);
    CHECK(copiedTile.getDataset() == CDBDataset::Elevation);
    CHECK(copiedTile.getCS_1() == 1);
    CHECK(copiedTile.getCS_2() == 2);
    CHECK(copiedTile.getLevel() == -10);
    CHECK(copiedTile.getUREF() == 0);
    CHECK(copiedTile.getRREF() == 0);
    CHECK(copiedTile.getChildren().size() == 0);
    CHECK(copiedTile.getCustomContentURI() == nullptr);
}

TEST_CASE("Test CDBTile move assignment")
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
    CHECK(movedRectangle.getNorth() == doctest::Approx(boundNorth));
    CHECK(movedRectangle.getWest() == doctest::Approx(boundWest));
    CHECK(movedRectangle.getSouth() == doctest::Approx(boundSouth));
    CHECK(movedRectangle.getEast() == doctest::Approx(boundEast));
    CHECK(movedRegion.getMinimumHeight() == doctest::Approx(0.0));
    CHECK(movedRegion.getMaximumHeight() == doctest::Approx(0.0));

    CHECK(movedTile.getRelativePath()
            == "Tiles/N32/W118/001_Elevation/LC/U0/N32W118_D001_S001_T002_LC10_U0_R0");
    CHECK(movedTile.getGeoCell() == geoCell);
    CHECK(movedTile.getDataset() == CDBDataset::Elevation);
    CHECK(movedTile.getCS_1() == 1);
    CHECK(movedTile.getCS_2() == 2);
    CHECK(movedTile.getLevel() == -10);
    CHECK(movedTile.getUREF() == 0);
    CHECK(movedTile.getRREF() == 0);
    CHECK(movedTile.getChildren().front() == child.get());
    CHECK(movedTile.getCustomContentURI() == nullptr);
}

TEST_CASE("Test CDBTile equal operator")
{
    SUBCASE("rhs and lhs are equals")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);
        CDBTile rhs(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);
        CHECK(lhs == rhs);
    }

    SUBCASE("rhs and lhs are different in GeoCell")
    {
        CDBGeoCell lhsGeoCell(32, -118);
        CDBTile lhs(lhsGeoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);

        CDBGeoCell rhsGeoCell(40, -118);
        CDBTile rhs(rhsGeoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);

        CHECK_FALSE(lhs == rhs);
    }

    SUBCASE("rhs and lhs are different in dataset")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);
        CDBTile rhs(geoCell, CDBDataset::GSFeature, 1, 2, -10, 0, 0);

        CHECK_FALSE(lhs == rhs);
    }

    SUBCASE("rhs and lhs are different in Component Selector 1")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 2, 2, -10, 0, 0);
        CDBTile rhs(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);

        CHECK_FALSE(lhs == rhs);
    }

    SUBCASE("rhs and lhs are different in Component Selector 2")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 1, 3, -10, 0, 0);
        CDBTile rhs(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);

        CHECK_FALSE(lhs == rhs);
    }

    SUBCASE("rhs and lhs are different in level")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 1, 2, 0, 0, 0);
        CDBTile rhs(geoCell, CDBDataset::Elevation, 1, 2, -10, 0, 0);

        CHECK_FALSE(lhs == rhs);
    }

    SUBCASE("rhs and lhs are different in UREF")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 1, 2, 2, 2, 0);
        CDBTile rhs(geoCell, CDBDataset::Elevation, 1, 2, 2, 0, 0);

        CHECK_FALSE(lhs == rhs);
    }

    SUBCASE("rhs and lhs are different in RREF")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile lhs(geoCell, CDBDataset::Elevation, 1, 2, 2, 2, 1);
        CDBTile rhs(geoCell, CDBDataset::Elevation, 1, 2, 2, 2, 0);

        CHECK_FALSE(lhs == rhs);
    }
}

TEST_CASE("Test create parent for a given CDBTile")
{
    SUBCASE("Test negative LOD tile")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, -8, 0, 0);
        auto parent = CDBTile::createParentTile(tile);
        CHECK(parent != std::nullopt);
        CHECK(parent->getGeoCell() == geoCell);
        CHECK(parent->getDataset() == CDBDataset::Elevation);
        CHECK(parent->getCS_1() == 1);
        CHECK(parent->getCS_2() == 1);
        CHECK(parent->getLevel() == -9);
        CHECK(parent->getUREF() == 0);
        CHECK(parent->getRREF() == 0);
    }

    SUBCASE("Test positive LOD tile")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 2, 2, 3);
        auto parent = CDBTile::createParentTile(tile);
        CHECK(parent != std::nullopt);
        CHECK(parent->getGeoCell() == geoCell);
        CHECK(parent->getDataset() == CDBDataset::Elevation);
        CHECK(parent->getCS_1() == 1);
        CHECK(parent->getCS_2() == 1);
        CHECK(parent->getLevel() == 1);
        CHECK(parent->getUREF() == 1);
        CHECK(parent->getRREF() == 1);
    }

    SUBCASE("Test LOD -10 tile")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, -10, 0, 0);
        auto parent = CDBTile::createParentTile(tile);
        CHECK(parent == std::nullopt);
    }
}

TEST_CASE("Test create child tile for negative level tile")
{
    SUBCASE("Test valid negative level tile")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, -1, 0, 0);
        auto child = CDBTile::createChildForNegativeLOD(tile);
        CHECK(child.getGeoCell() == geoCell);
        CHECK(child.getDataset() == CDBDataset::Elevation);
        CHECK(child.getCS_1() == 1);
        CHECK(child.getCS_2() == 1);
        CHECK(child.getLevel() == 0);
        CHECK(child.getUREF() == 0);
        CHECK(child.getRREF() == 0);
    }

    SUBCASE("Test invalid positive level tile")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        CHECK_THROWS_AS(CDBTile::createChildForNegativeLOD(tile), std::invalid_argument);
    }
}

TEST_CASE("Test create child tile for positive level tile")
{
    // caution quadtree in CDB with index (0, 0) begins from bottom left, not top left
    SUBCASE("Test north west child")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 2, 2, 2);
        auto child = CDBTile::createNorthWestForPositiveLOD(tile);
        CHECK(child.getGeoCell() == geoCell);
        CHECK(child.getDataset() == CDBDataset::Elevation);
        CHECK(child.getCS_1() == 1);
        CHECK(child.getCS_2() == 1);
        CHECK(child.getLevel() == 3);
        CHECK(child.getUREF() == 5);
        CHECK(child.getRREF() == 4);

        CDBTile negativeTile(geoCell, CDBDataset::Elevation, 1, 1, -1, 0, 0);
        CHECK_THROWS_AS(CDBTile::createNorthWestForPositiveLOD(negativeTile), std::invalid_argument);
    }

    SUBCASE("Test north east child")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 2, 2, 2);
        auto child = CDBTile::createNorthEastForPositiveLOD(tile);
        CHECK(child.getGeoCell() == geoCell);
        CHECK(child.getDataset() == CDBDataset::Elevation);
        CHECK(child.getCS_1() == 1);
        CHECK(child.getCS_2() == 1);
        CHECK(child.getLevel() == 3);
        CHECK(child.getUREF() == 5);
        CHECK(child.getRREF() == 5);

        CDBTile negativeTile(geoCell, CDBDataset::Elevation, 1, 1, -1, 0, 0);
        CHECK_THROWS_AS(CDBTile::createNorthEastForPositiveLOD(negativeTile), std::invalid_argument);
    }

    SUBCASE("Test south west child")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 2, 2, 2);
        auto child = CDBTile::createSouthWestForPositiveLOD(tile);
        CHECK(child.getGeoCell() == geoCell);
        CHECK(child.getDataset() == CDBDataset::Elevation);
        CHECK(child.getCS_1() == 1);
        CHECK(child.getCS_2() == 1);
        CHECK(child.getLevel() == 3);
        CHECK(child.getUREF() == 4);
        CHECK(child.getRREF() == 4);

        CDBTile negativeTile(geoCell, CDBDataset::Elevation, 1, 1, -1, 0, 0);
        CHECK_THROWS_AS(CDBTile::createSouthWestForPositiveLOD(negativeTile), std::invalid_argument);
    }

    SUBCASE("Test south east child")
    {
        CDBGeoCell geoCell(32, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 2, 2, 2);
        auto child = CDBTile::createSouthEastForPositiveLOD(tile);
        CHECK(child.getGeoCell() == geoCell);
        CHECK(child.getDataset() == CDBDataset::Elevation);
        CHECK(child.getCS_1() == 1);
        CHECK(child.getCS_2() == 1);
        CHECK(child.getLevel() == 3);
        CHECK(child.getUREF() == 4);
        CHECK(child.getRREF() == 5);

        CDBTile negativeTile(geoCell, CDBDataset::Elevation, 1, 1, -1, 0, 0);
        CHECK_THROWS_AS(CDBTile::createSouthEastForPositiveLOD(negativeTile), std::invalid_argument);
    }
}

TEST_CASE("Test create from file")
{
    SUBCASE("Test create from valid file name with positive level")
    {
        CDBGeoCell geoCell(32, -118);
        auto tile = CDBTile::createFromFile("N32W118_D001_S001_T001_L03_U5_R5");
        CHECK(tile != std::nullopt);
        CHECK(tile->getGeoCell() == geoCell);
        CHECK(tile->getDataset() == CDBDataset::Elevation);
        CHECK(tile->getCS_1() == 1);
        CHECK(tile->getCS_2() == 1);
        CHECK(tile->getLevel() == 3);
        CHECK(tile->getUREF() == 5);
        CHECK(tile->getRREF() == 5);
        std::string path = tile->getRelativePathWithNonZeroPaddedLevel();

        // get the level part out of the path
        for(int i = 0 ; i < 5 ; i++)
        {
          path.erase(0, path.find("_") + 1);
        }
        std::string nonZeroPaddedLevel = path.substr(0, path.find("_"));
        CHECK(nonZeroPaddedLevel == "L3");
    }

    SUBCASE("Test create from valid file name with negative level")
    {
        CDBGeoCell geoCell(32, -118);
        auto tile = CDBTile::createFromFile("N32W118_D001_S001_T001_LC03_U0_R0");
        CHECK(tile != std::nullopt);
        CHECK(tile->getGeoCell() == geoCell);
        CHECK(tile->getDataset() == CDBDataset::Elevation);
        CHECK(tile->getCS_1() == 1);
        CHECK(tile->getCS_2() == 1);
        CHECK(tile->getLevel() == -3);
        CHECK(tile->getUREF() == 0);
        CHECK(tile->getRREF() == 0);
        std::string path = tile->getRelativePathWithNonZeroPaddedLevel();

        // get the level part out of the path
        for(int i = 0 ; i < 5 ; i++)
        {
          path.erase(0, path.find("_") + 1);
        }
        std::string nonZeroPaddedLevel = path.substr(0, path.find("_"));
        CHECK(nonZeroPaddedLevel == "LC3");
    }

    SUBCASE("Test create from invalid latitude")
    {
        auto tile = CDBTile::createFromFile("M32W118_D001_S001_T001_L03_U5_R5");
        CHECK(tile == std::nullopt);
    }

    SUBCASE("Test create from out of bound latitude")
    {
        CHECK(CDBTile::createFromFile("N90W118_D001_S001_T001_L03_U5_R5") == std::nullopt);
        CHECK(CDBTile::createFromFile("S91W118_D001_S001_T001_L03_U5_R5") == std::nullopt);
    }

    SUBCASE("Test create from invalid longitude")
    {
        auto tile = CDBTile::createFromFile("N32M118_D001_S001_T001_L03_U5_R5");
        CHECK(tile == std::nullopt);
    }

    SUBCASE("Test create from out of bound longitude")
    {
        CHECK(CDBTile::createFromFile("N32W181_D001_S001_T001_L03_U5_R5") == std::nullopt);
        CHECK(CDBTile::createFromFile("S32E181_D001_S001_T001_L03_U5_R5") == std::nullopt);
    }

    SUBCASE("Test create from invalid dataset")
    {
        auto tile = CDBTile::createFromFile("N32W118_D12_S001_T001_L03_U5_R5");
        CHECK(tile == std::nullopt);
    }

    SUBCASE("Test create from invalid component selector 1")
    {
        auto tile = CDBTile::createFromFile("N32W118_D12_ST1_T001_L03_U5_R5");
        CHECK(tile == std::nullopt);
    }

    SUBCASE("Test create from invalid component selector 2")
    {
        auto tile = CDBTile::createFromFile("N32W118_D12_S001_T00S1_L03_U5_R5");
        CHECK(tile == std::nullopt);
    }

    SUBCASE("Test create from invalid level")
    {
        auto belowNeg10Level = CDBTile::createFromFile("N32W118_D001_S001_T001_LC13_U5_R5");
        CHECK(belowNeg10Level == std::nullopt);

        auto above23Level = CDBTile::createFromFile("N32W118_D001_S001_T001_L24_U5_R5");
        CHECK(above23Level == std::nullopt);
    }

    SUBCASE("Test create from UREF and RREF for negative level")
    {
        auto tile = CDBTile::createFromFile("N32W118_D001_S001_T001_LC04_U5_R0");
        CHECK(tile == std::nullopt);

        tile = CDBTile::createFromFile("N32W118_D001_S001_T001_LC04_U0_R4");
        CHECK(tile == std::nullopt);
    }

    SUBCASE("Test create from invalid UREF and RREF for positive level")
    {
        for (int i = 0; i <= 23; ++i) {
            std::string UREFFilename = "N32W118_D001_S001_T001_L" + toStringWithZeroPadding(2, i) + "_U"
                                       + std::to_string(1 << i) + "_R0";
            CHECK(CDBTile::createFromFile(UREFFilename) == std::nullopt);

            std::string RREFFilename = "N32W118_D001_S001_T001_L" + toStringWithZeroPadding(2, i) + "_U0_R"
                                       + std::to_string(1 << i);
            CHECK(CDBTile::createFromFile(RREFFilename) == std::nullopt);
        }
    }

    SUBCASE("Test create from nonsense file")
    {
        auto tile = CDBTile::createFromFile("asas_N32_asasas_12_asss");
        CHECK(tile == std::nullopt);
    }
}

TEST_CASE("Test bounding regions")
{
    SUBCASE("tile at zone 10")
    {
        CDBGeoCell geoCell(89, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SUBCASE("tile at region 9")
    {
        CDBGeoCell geoCell(80, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SUBCASE("tile at region 8")
    {
        CDBGeoCell geoCell(75, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SUBCASE("tile at region 7")
    {
        CDBGeoCell geoCell(70, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SUBCASE("tile at region 6")
    {
        CDBGeoCell geoCell(50, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SUBCASE("tile at region 5")
    {
        CDBGeoCell geoCell(-50, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SUBCASE("tile at region 4")
    {
        CDBGeoCell geoCell(-70, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SUBCASE("tile at region 3")
    {
        CDBGeoCell geoCell(-75, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SUBCASE("tile at region 2")
    {
        CDBGeoCell geoCell(-80, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SUBCASE("tile at region 1")
    {
        CDBGeoCell geoCell(-89, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }

    SUBCASE("tile at region 0")
    {
        CDBGeoCell geoCell(-90, -118);
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
        const auto &boundRegion = tile.getBoundRegion();
        const auto &rectangle = boundRegion.getRectangle();
        checkGeoCellExtent(geoCell, rectangle);
    }
}

TEST_CASE("Test retrieving GeoCell and Dataset from tile name")
{
    CDBGeoCell geoCell(32, -118);
    CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0);
    CHECK(CDBTile::retrieveGeoCellDatasetFromTileName(tile) == "N32W118_D001_S001_T001");
}

TEST_SUITE_END();

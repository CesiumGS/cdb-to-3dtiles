#include "CDBTileset.h"
#include "catch2/catch.hpp"

using namespace CDBTo3DTiles;
using namespace Core;

static void checkTileInsertedSuccessfully(const CDBTile *node, const CDBTile &tileToCheck, bool &seeNode)
{
    if (node == nullptr) {
        return;
    }

    if (node->getLevel() > tileToCheck.getLevel()) {
        return;
    }

    if (*node == tileToCheck) {
        seeNode = true;
        return;
    }

    for (auto child : node->getChildren()) {
        checkTileInsertedSuccessfully(child, tileToCheck, seeNode);
    }
}

static bool isNodeInTree(const CDBTile *node, const CDBTile &tileToCheck)
{
    bool seeNode = false;
    checkTileInsertedSuccessfully(node, tileToCheck, seeNode);
    return seeNode;
}

TEST_CASE("Test insertion", "[CDBTileset]")
{
    SECTION("Insert with default constructor")
    {
        CDBTileset tileset;
        REQUIRE(tileset.getRoot() == nullptr);

        // insert root
        CDBGeoCell geoCell(32, -118);
        CDBTile root(geoCell, CDBDataset::Elevation, 1, 1, -10, 0, 0);
        REQUIRE(tileset.insertTile(root) != nullptr);
        REQUIRE(isNodeInTree(tileset.getRoot(), root));

        // insert tile not root
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 10, 0, 0);
        REQUIRE(tileset.insertTile(tile) != nullptr);
        REQUIRE(isNodeInTree(tileset.getRoot(), tile));
    }

    SECTION("Insert with constructor that specify root level, UREF, and RREF")
    {
        CDBTileset tileset(3, 0, 0);
        REQUIRE(tileset.getRoot() == nullptr);

        CDBGeoCell geoCell(32, -118);
        CDBTile root(geoCell, CDBDataset::Elevation, 1, 1, 3, 0, 0);
        REQUIRE(tileset.insertTile(root) != nullptr);
        REQUIRE(isNodeInTree(tileset.getRoot(), root));

        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 10, 0, 0);
        REQUIRE(tileset.insertTile(tile) != nullptr);
        REQUIRE(isNodeInTree(tileset.getRoot(), tile));
    }

    SECTION("Insert invalid node ")
    {
        CDBTileset tileset(3, 0, 0);
        REQUIRE(tileset.getRoot() == nullptr);

        CDBGeoCell geoCell(32, -118);

        // insert tile that has lower LOD than root
        REQUIRE(tileset.insertTile(CDBTile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0)) == nullptr);

        // insert tile that has the same level as root but different UREF
        REQUIRE(tileset.insertTile(CDBTile(geoCell, CDBDataset::Elevation, 1, 1, 3, 1, 0)) == nullptr);

        // insert tile that has the same level as root but different RREF
        REQUIRE(tileset.insertTile(CDBTile(geoCell, CDBDataset::Elevation, 1, 1, 3, 0, 1)) == nullptr);

        // insert tile that has higher level than root, but not cover by root
        REQUIRE(tileset.insertTile(CDBTile(geoCell, CDBDataset::Elevation, 1, 1, 5, 17, 0)) == nullptr);

        // insert valid tile
        REQUIRE(tileset.insertTile(CDBTile(geoCell, CDBDataset::Elevation, 1, 1, 5, 3, 0)) != nullptr);
    }
}

TEST_CASE("Test get fit tile", "[CDBTileset]")
{
    // fill the tileset
    CDBTileset tileset;

    CDBGeoCell geoCell(32, -118);
    CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 10, 0, 0);
    tileset.insertTile(tile);

    SECTION("Get the leaf tile that contains the point")
    {
        BoundingRegion region = tile.getBoundRegion();
        GlobeRectangle rectangle = region.getRectangle();
        Cartographic center = rectangle.computeCenter();
        const CDBTile *fitTile = tileset.getFitTile(center);
        REQUIRE(fitTile != nullptr);
        REQUIRE(*fitTile == tile);
    }

    SECTION("Get the parent tile that contains the point")
    {
        CDBTile testTile(geoCell, CDBDataset::Elevation, 1, 1, 10, 1, 0);
        BoundingRegion region = testTile.getBoundRegion();
        GlobeRectangle rectangle = region.getRectangle();
        Cartographic center = rectangle.computeCenter();
        const CDBTile *fitTile = tileset.getFitTile(center);
        REQUIRE(fitTile != nullptr);
        REQUIRE(*fitTile == CDBTile(geoCell, CDBDataset::Elevation, 1, 1, 9, 0, 0));
    }

    SECTION("Point falls outside of the tileset region")
    {
        CDBGeoCell nextGeoCell(32, -117);
        CDBTile nextTile(nextGeoCell, CDBDataset::Elevation, 1, 1, 10, 0, 0);
        BoundingRegion region = nextTile.getBoundRegion();
        GlobeRectangle rectangle = region.getRectangle();
        Cartographic center = rectangle.computeCenter();
        const CDBTile *fitTile = tileset.getFitTile(center);
        REQUIRE(fitTile == nullptr);
    }

    SECTION("Find point with empty tileset")
    {
        CDBTileset emptyTileset;
        BoundingRegion region = tile.getBoundRegion();
        GlobeRectangle rectangle = region.getRectangle();
        Cartographic center = rectangle.computeCenter();
        const CDBTile *fitTile = emptyTileset.getFitTile(center);
        REQUIRE(fitTile == nullptr);
    }
}

TEST_CASE("Test get first tile at level", "[CDBTileset]")
{
    // fill the tileset
    CDBTileset tileset;

    CDBGeoCell geoCell(32, -118);
    CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 10, 0, 0);
    tileset.insertTile(tile);
    
    SECTION("Return tile at level 7")
    {
        int level = 7;
        REQUIRE(*tileset.getFirstTileAtLevel(level) == CDBTile(geoCell, CDBDataset::Elevation, 1, 1, level, 0, 0));
    }
}

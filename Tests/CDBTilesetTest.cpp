#include "CDBTileset.h"
#include <doctest/doctest.h>

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

TEST_SUITE_BEGIN("CDBTileset");

TEST_CASE("Test insertion")
{
    SUBCASE("Insert with default constructor")
    {
        CDBTileset tileset;
        CHECK(tileset.getRoot() == nullptr);

        // insert root
        CDBGeoCell geoCell(32, -118);
        CDBTile root(geoCell, CDBDataset::Elevation, 1, 1, -10, 0, 0);
        CHECK(tileset.insertTile(root) != nullptr);
        CHECK(isNodeInTree(tileset.getRoot(), root));

        // insert tile not root
        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 10, 0, 0);
        CHECK(tileset.insertTile(tile) != nullptr);
        CHECK(isNodeInTree(tileset.getRoot(), tile));
    }

    SUBCASE("Insert with constructor that specify root level, UREF, and RREF")
    {
        CDBTileset tileset(3, 0, 0);
        CHECK(tileset.getRoot() == nullptr);

        CDBGeoCell geoCell(32, -118);
        CDBTile root(geoCell, CDBDataset::Elevation, 1, 1, 3, 0, 0);
        CHECK(tileset.insertTile(root) != nullptr);
        CHECK(isNodeInTree(tileset.getRoot(), root));

        CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 10, 0, 0);
        CHECK(tileset.insertTile(tile) != nullptr);
        CHECK(isNodeInTree(tileset.getRoot(), tile));
    }

    SUBCASE("Insert invalid node ")
    {
        CDBTileset tileset(3, 0, 0);
        CHECK(tileset.getRoot() == nullptr);

        CDBGeoCell geoCell(32, -118);

        // insert tile that has lower LOD than root
        CHECK(tileset.insertTile(CDBTile(geoCell, CDBDataset::Elevation, 1, 1, 0, 0, 0)) == nullptr);

        // insert tile that has the same level as root but different UREF
        CHECK(tileset.insertTile(CDBTile(geoCell, CDBDataset::Elevation, 1, 1, 3, 1, 0)) == nullptr);

        // insert tile that has the same level as root but different RREF
        CHECK(tileset.insertTile(CDBTile(geoCell, CDBDataset::Elevation, 1, 1, 3, 0, 1)) == nullptr);

        // insert tile that has higher level than root, but not cover by root
        CHECK(tileset.insertTile(CDBTile(geoCell, CDBDataset::Elevation, 1, 1, 5, 17, 0)) == nullptr);

        // insert valid tile
        CHECK(tileset.insertTile(CDBTile(geoCell, CDBDataset::Elevation, 1, 1, 5, 3, 0)) != nullptr);
    }
}

TEST_CASE("Test get fit tile")
{
    // fill the tileset
    CDBTileset tileset;

    CDBGeoCell geoCell(32, -118);
    CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 10, 0, 0);
    tileset.insertTile(tile);

    SUBCASE("Get the leaf tile that contains the point")
    {
        BoundingRegion region = tile.getBoundRegion();
        GlobeRectangle rectangle = region.getRectangle();
        Cartographic center = rectangle.computeCenter();
        const CDBTile *fitTile = tileset.getFitTile(center);
        CHECK(fitTile != nullptr);
        CHECK(*fitTile == tile);
    }

    SUBCASE("Get the parent tile that contains the point")
    {
        CDBTile testTile(geoCell, CDBDataset::Elevation, 1, 1, 10, 1, 0);
        BoundingRegion region = testTile.getBoundRegion();
        GlobeRectangle rectangle = region.getRectangle();
        Cartographic center = rectangle.computeCenter();
        const CDBTile *fitTile = tileset.getFitTile(center);
        CHECK(fitTile != nullptr);
        CHECK(*fitTile == CDBTile(geoCell, CDBDataset::Elevation, 1, 1, 9, 0, 0));
    }

    SUBCASE("Point falls outside of the tileset region")
    {
        CDBGeoCell nextGeoCell(32, -117);
        CDBTile nextTile(nextGeoCell, CDBDataset::Elevation, 1, 1, 10, 0, 0);
        BoundingRegion region = nextTile.getBoundRegion();
        GlobeRectangle rectangle = region.getRectangle();
        Cartographic center = rectangle.computeCenter();
        const CDBTile *fitTile = tileset.getFitTile(center);
        CHECK(fitTile == nullptr);
    }

    SUBCASE("Find point with empty tileset")
    {
        CDBTileset emptyTileset;
        BoundingRegion region = tile.getBoundRegion();
        GlobeRectangle rectangle = region.getRectangle();
        Cartographic center = rectangle.computeCenter();
        const CDBTile *fitTile = emptyTileset.getFitTile(center);
        CHECK(fitTile == nullptr);
    }
}

TEST_CASE("Test get first tile at level")
{
    // fill the tileset
    CDBTileset tileset;

    CDBGeoCell geoCell(32, -118);
    CDBTile tile(geoCell, CDBDataset::Elevation, 1, 1, 10, 0, 0);
    tileset.insertTile(tile);
    
    SUBCASE("Return tile at level 7")
    {
        int level = 7;
        CHECK(*tileset.getFirstTileAtLevel(level) == CDBTile(geoCell, CDBDataset::Elevation, 1, 1, level, 0, 0));
    }

    SUBCASE("Return tile at level 11 with non zero UREF and RREF")
    {
        int level = 11;
        tileset.insertTile(CDBTile(geoCell, CDBDataset::Elevation, 1, 1, level, 6, 2));
        const CDBTile *tileAtLevel = tileset.getFirstTileAtLevel(level);
        CHECK(*tileAtLevel == CDBTile(geoCell, CDBDataset::Elevation, 1, 1, level, 6, 2));
    }
}


TEST_SUITE_END();
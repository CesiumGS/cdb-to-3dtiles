#include "CDBGeoCell.h"
#include <doctest/doctest.h>

using namespace CDBTo3DTiles;

TEST_SUITE_BEGIN("CDBGeoCell");

TEST_CASE("Test zone of GeoCell")
{
    CHECK_THROWS_AS(CDBGeoCell(90, -118), std::invalid_argument);
    CHECK(CDBGeoCell(89, -118).getZone() == 10);
    CHECK(CDBGeoCell(80, -118).getZone() == 9);
    CHECK(CDBGeoCell(75, -118).getZone() == 8);
    CHECK(CDBGeoCell(70, -118).getZone() == 7);
    CHECK(CDBGeoCell(50, -118).getZone() == 6);
    CHECK(CDBGeoCell(-50, -118).getZone() == 5);
    CHECK(CDBGeoCell(-70, -118).getZone() == 4);
    CHECK(CDBGeoCell(-75, -118).getZone() == 3);
    CHECK(CDBGeoCell(-80, -118).getZone() == 2);
    CHECK(CDBGeoCell(-89, -118).getZone() == 1);
    CHECK(CDBGeoCell(-90, -118).getZone() == 0);
    CHECK_THROWS_AS(CDBGeoCell(-91, -118), std::invalid_argument);
}

TEST_CASE("Test zone extent in degree")
{
    CHECK(CDBGeoCell(89, -118).getLongitudeExtentInDegree() == 12);
    CHECK(CDBGeoCell(80, -118).getLongitudeExtentInDegree() == 6);
    CHECK(CDBGeoCell(75, -118).getLongitudeExtentInDegree() == 4);
    CHECK(CDBGeoCell(70, -118).getLongitudeExtentInDegree() == 3);
    CHECK(CDBGeoCell(50, -118).getLongitudeExtentInDegree() == 2);
    CHECK(CDBGeoCell(-50, -118).getLongitudeExtentInDegree() == 1);
    CHECK(CDBGeoCell(-70, -118).getLongitudeExtentInDegree() == 2);
    CHECK(CDBGeoCell(-75, -118).getLongitudeExtentInDegree() == 3);
    CHECK(CDBGeoCell(-80, -118).getLongitudeExtentInDegree() == 4);
    CHECK(CDBGeoCell(-89, -118).getLongitudeExtentInDegree() == 6);
    CHECK(CDBGeoCell(-90, -118).getLongitudeExtentInDegree() == 12);
}

TEST_CASE("Test get relative path for GeoCell")
{
    CHECK(CDBGeoCell(32, -118).getRelativePath() == "Tiles/N32/W118");
    CHECK(CDBGeoCell(-32, 118).getRelativePath() == "Tiles/S32/E118");
}

TEST_CASE("Test get latitude directory name")
{
    CHECK(CDBGeoCell(32, -118).getLatitudeDirectoryName() == "N32");
    CHECK(CDBGeoCell(-32, -118).getLatitudeDirectoryName() == "S32");
}

TEST_CASE("Test get longitude directory name")
{
    CHECK(CDBGeoCell(-32, 118).getLongitudeDirectoryName() == "E118");
    CHECK(CDBGeoCell(-32, -118).getLongitudeDirectoryName() == "W118");
}

TEST_CASE("Test parse latitude directory name")
{
    SUBCASE("Parse valid directory name")
    {
        auto N32 = CDBGeoCell::parseLatFromFilename("N32");
        CHECK(N32 != std::nullopt);
        CHECK(*N32 == 32);

        auto S32 = CDBGeoCell::parseLatFromFilename("S32");
        CHECK(S32 != std::nullopt);
        CHECK(*S32 == -32);
    }

    SUBCASE("Parse out of bound latitude")
    {
        CHECK(CDBGeoCell::parseLatFromFilename("N90") == std::nullopt);
        CHECK(CDBGeoCell::parseLatFromFilename("S91") == std::nullopt);
    }

    SUBCASE("Parse invalid directory")
    {
        CHECK(CDBGeoCell::parseLatFromFilename("W21") == std::nullopt);
        CHECK(CDBGeoCell::parseLatFromFilename("E32") == std::nullopt);
        CHECK(CDBGeoCell::parseLatFromFilename("ASAS") == std::nullopt);
    }
}

TEST_CASE("Test parse longitude directory name")
{
    SUBCASE("Parse valid directory name")
    {
        auto W118 = CDBGeoCell::parseLongFromFilename("W118");
        CHECK(W118 != std::nullopt);
        CHECK(*W118 == -118);

        auto E118 = CDBGeoCell::parseLongFromFilename("E118");
        CHECK(E118 != std::nullopt);
        CHECK(*E118 == 118);
    }

    SUBCASE("Parse out of bound latitude")
    {
        CHECK(CDBGeoCell::parseLongFromFilename("W181") == std::nullopt);
        CHECK(CDBGeoCell::parseLongFromFilename("E181") == std::nullopt);
    }

    SUBCASE("Parse invalid directory")
    {
        CHECK(CDBGeoCell::parseLongFromFilename("N118") == std::nullopt);
        CHECK(CDBGeoCell::parseLongFromFilename("S118") == std::nullopt);
        CHECK(CDBGeoCell::parseLongFromFilename("ASASSASAS") == std::nullopt);
    }
}
TEST_SUITE_END();
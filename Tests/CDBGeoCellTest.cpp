#include "CDBGeoCell.h"
#include "catch2/catch.hpp"

using namespace CDBTo3DTiles;

TEST_CASE("Test zone of GeoCell", "[CDBGeoCell]")
{
    REQUIRE_THROWS_AS(CDBGeoCell(90, -118), std::invalid_argument);
    REQUIRE(CDBGeoCell(89, -118).getZone() == 10);
    REQUIRE(CDBGeoCell(80, -118).getZone() == 9);
    REQUIRE(CDBGeoCell(75, -118).getZone() == 8);
    REQUIRE(CDBGeoCell(70, -118).getZone() == 7);
    REQUIRE(CDBGeoCell(50, -118).getZone() == 6);
    REQUIRE(CDBGeoCell(-50, -118).getZone() == 5);
    REQUIRE(CDBGeoCell(-70, -118).getZone() == 4);
    REQUIRE(CDBGeoCell(-75, -118).getZone() == 3);
    REQUIRE(CDBGeoCell(-80, -118).getZone() == 2);
    REQUIRE(CDBGeoCell(-89, -118).getZone() == 1);
    REQUIRE(CDBGeoCell(-90, -118).getZone() == 0);
    REQUIRE_THROWS_AS(CDBGeoCell(-91, -118), std::invalid_argument);
}

TEST_CASE("Test zone extent in degree", "[CDBGeoCell]")
{
    REQUIRE(CDBGeoCell(89, -118).getLongitudeExtentInDegree() == 12);
    REQUIRE(CDBGeoCell(80, -118).getLongitudeExtentInDegree() == 6);
    REQUIRE(CDBGeoCell(75, -118).getLongitudeExtentInDegree() == 4);
    REQUIRE(CDBGeoCell(70, -118).getLongitudeExtentInDegree() == 3);
    REQUIRE(CDBGeoCell(50, -118).getLongitudeExtentInDegree() == 2);
    REQUIRE(CDBGeoCell(-50, -118).getLongitudeExtentInDegree() == 1);
    REQUIRE(CDBGeoCell(-70, -118).getLongitudeExtentInDegree() == 2);
    REQUIRE(CDBGeoCell(-75, -118).getLongitudeExtentInDegree() == 3);
    REQUIRE(CDBGeoCell(-80, -118).getLongitudeExtentInDegree() == 4);
    REQUIRE(CDBGeoCell(-89, -118).getLongitudeExtentInDegree() == 6);
    REQUIRE(CDBGeoCell(-90, -118).getLongitudeExtentInDegree() == 12);
}

TEST_CASE("Test get relative path for GeoCell", "[CDBGeoCell]")
{
    REQUIRE(CDBGeoCell(32, -118).getRelativePath() == "Tiles/N32/W118");
    REQUIRE(CDBGeoCell(-32, 118).getRelativePath() == "Tiles/S32/E118");
}

TEST_CASE("Test get latitude directory name", "[CDBGeoCell]")
{
    REQUIRE(CDBGeoCell(32, -118).getLatitudeDirectoryName() == "N32");
    REQUIRE(CDBGeoCell(-32, -118).getLatitudeDirectoryName() == "S32");
}

TEST_CASE("Test get longitude directory name", "[CDBGeoCell]")
{
    REQUIRE(CDBGeoCell(-32, 118).getLongitudeDirectoryName() == "E118");
    REQUIRE(CDBGeoCell(-32, -118).getLongitudeDirectoryName() == "W118");
}

TEST_CASE("Test parse latitude directory name", "[CDBGeoCell]")
{
    SECTION("Parse valid directory name")
    {
        auto N32 = CDBGeoCell::parseLatFromFilename("N32");
        REQUIRE(N32 != std::nullopt);
        REQUIRE(*N32 == 32);

        auto S32 = CDBGeoCell::parseLatFromFilename("S32");
        REQUIRE(S32 != std::nullopt);
        REQUIRE(*S32 == -32);
    }

    SECTION("Parse out of bound latitude")
    {
        REQUIRE(CDBGeoCell::parseLatFromFilename("N90") == std::nullopt);
        REQUIRE(CDBGeoCell::parseLatFromFilename("S91") == std::nullopt);
    }

    SECTION("Parse invalid directory")
    {
        REQUIRE(CDBGeoCell::parseLatFromFilename("W21") == std::nullopt);
        REQUIRE(CDBGeoCell::parseLatFromFilename("E32") == std::nullopt);
        REQUIRE(CDBGeoCell::parseLatFromFilename("ASAS") == std::nullopt);
    }
}

TEST_CASE("Test parse longitude directory name", "[CDBGeoCell]")
{
    SECTION("Parse valid directory name")
    {
        auto W118 = CDBGeoCell::parseLongFromFilename("W118");
        REQUIRE(W118 != std::nullopt);
        REQUIRE(*W118 == -118);

        auto E118 = CDBGeoCell::parseLongFromFilename("E118");
        REQUIRE(E118 != std::nullopt);
        REQUIRE(*E118 == 118);
    }

    SECTION("Parse out of bound latitude")
    {
        REQUIRE(CDBGeoCell::parseLongFromFilename("W181") == std::nullopt);
        REQUIRE(CDBGeoCell::parseLongFromFilename("E181") == std::nullopt);
    }

    SECTION("Parse invalid directory")
    {
        REQUIRE(CDBGeoCell::parseLongFromFilename("N118") == std::nullopt);
        REQUIRE(CDBGeoCell::parseLongFromFilename("S118") == std::nullopt);
        REQUIRE(CDBGeoCell::parseLongFromFilename("ASASSASAS") == std::nullopt);
    }
}

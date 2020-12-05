#include "CDBGeometryVectors.h"
#include "CDBTo3DTiles.h"
#include "Config.h"
#include "catch2/catch.hpp"
#include <filesystem>

using namespace CDBTo3DTiles;

TEST_CASE("Test create CDBGeometryVector", "[CDBGeometryVectors]")
{
    SECTION("Test valid file with line geometry")
    {
        std::filesystem::path CDBPath = dataPath / "RoadNetwork";
        std::filesystem::path vectorFile = CDBPath / "Tiles" / "N32" / "W118" / "201_RoadNetwork" / "LC"
                                           / "U0" / "N32W118_D201_S002_T003_LC05_U0_R0.dbf";

        auto vector = CDBGeometryVectors::createFromFile(vectorFile, CDBPath);
        REQUIRE(vector != std::nullopt);

        const auto &mesh = vector->getMesh();
        REQUIRE(mesh.primitiveType == PrimitiveType::Lines);
        REQUIRE(mesh.positions.size() > 0);
        REQUIRE(mesh.positionRTCs.size() > 0);
        REQUIRE(mesh.normals.size() == 0);
        REQUIRE(mesh.UVs.size() == 0);

        // check instance attributes share the same class attribute
        auto attribsInstances = vector->getInstancesAttributes();
        size_t instancesCount = attribsInstances.getInstancesCount();
        REQUIRE(instancesCount == 8);

        const auto &CNAMs = attribsInstances.getCNAMs();
        for (const auto &CNAM : CNAMs) {
            REQUIRE(CNAM == "AP030000-AP030-000U31R31-0");
        }

        const auto &stringAttribs = attribsInstances.getStringAttribs();
        const auto &integerAttribs = attribsInstances.getIntegerAttribs();
        const auto &doubleAttribs = attribsInstances.getDoubleAttribs();
        for (size_t i = 0; i < instancesCount; ++i) {
            REQUIRE(stringAttribs.at("AHGT")[i] == "F");
            REQUIRE(stringAttribs.at("FACC")[i] == "AP030");
            REQUIRE(stringAttribs.at("MODT")[i] == "T");

            REQUIRE(integerAttribs.at("CMIX")[i] == 3);
            REQUIRE(integerAttribs.at("DIR")[i] == 3);
            REQUIRE(integerAttribs.at("FSC")[i] == 0);
            REQUIRE(integerAttribs.at("LTN")[i] == 2);
            REQUIRE(integerAttribs.at("TRF")[i] == 4);

            REQUIRE(doubleAttribs.at("HGT")[i] == 0.0);
        }

        REQUIRE(stringAttribs.at("EJID").size() == instancesCount);
        REQUIRE(stringAttribs.at("SJID").size() == instancesCount);
        REQUIRE(integerAttribs.at("LENL").size() == instancesCount);
        REQUIRE(integerAttribs.at("RTAI").size() == instancesCount);
        REQUIRE(doubleAttribs.at("WGP").size() == instancesCount);
    }

    SECTION("Test valid file with point geometry")
    {
        std::filesystem::path CDBPath = dataPath / "GSFeature";
        std::filesystem::path vectorFile = CDBPath / "Tiles" / "N32" / "W118" / "100_GSFeature" / "LC" / "U0"
                                           / "N32W118_D100_S004_T001_LC01_U0_R0.dbf";

        auto vector = CDBGeometryVectors::createFromFile(vectorFile, CDBPath);
        REQUIRE(vector != std::nullopt);

        const auto &mesh = vector->getMesh();
        REQUIRE(mesh.primitiveType == PrimitiveType::Points);
        REQUIRE(mesh.positions.size() == 1);
        REQUIRE(mesh.positionRTCs.size() == 1);
        REQUIRE(mesh.normals.size() == 0);
        REQUIRE(mesh.UVs.size() == 0);

        // check instance attributes share the same class attribute
        auto attribsInstances = vector->getInstancesAttributes();
        size_t instancesCount = attribsInstances.getInstancesCount();
        REQUIRE(instancesCount == 1);

        const auto &CNAM = attribsInstances.getCNAMs().front();
        const auto &stringAttribs = attribsInstances.getStringAttribs();
        const auto &integerAttribs = attribsInstances.getIntegerAttribs();
        const auto &doubleAttribs = attribsInstances.getDoubleAttribs();
        REQUIRE(CNAM == "GB010000-GB010-000U44R51-0");
        REQUIRE(stringAttribs.at("APID").front() == "KNZY");
        REQUIRE(stringAttribs.at("RWID").front() == "");
        REQUIRE(stringAttribs.at("AHGT").front() == "F");
        REQUIRE(stringAttribs.at("FACC").front() == "GB010");

        REQUIRE(integerAttribs.at("LPH").front() == 0);
        REQUIRE(integerAttribs.at("RTAI").front() == 100);
        REQUIRE(integerAttribs.at("CMIX").front() == 3);
        REQUIRE(integerAttribs.at("FSC").front() == 0);
        REQUIRE(integerAttribs.at("LTYP").front() == 0);

        REQUIRE(doubleAttribs.at("AO1").front() == Approx(0.0));
    }

    SECTION("Test valid file with polygon geometry")
    {
        std::filesystem::path CDBPath = dataPath / "HydrographyNetwork";
        std::filesystem::path vectorFile = CDBPath / "Tiles" / "N32" / "W118" / "204_HydrographyNetwork"
                                           / "LC" / "U0" / "N32W118_D204_S002_T005_LC06_U0_R0.dbf";

        auto vector = CDBGeometryVectors::createFromFile(vectorFile, CDBPath);
        REQUIRE(vector != std::nullopt);

        const auto &mesh = vector->getMesh();
        REQUIRE(mesh.primitiveType == PrimitiveType::Triangles);
        REQUIRE(mesh.positions.size() > 0);
        REQUIRE(mesh.positionRTCs.size() > 0);
        REQUIRE(mesh.normals.size() == 0);
        REQUIRE(mesh.UVs.size() == 0);

        // check instance attributes share the same class attribute
        auto attribsInstances = vector->getInstancesAttributes();
        size_t instancesCount = attribsInstances.getInstancesCount();
        REQUIRE(instancesCount == 1);

        const auto &CNAM = attribsInstances.getCNAMs().front();
        const auto &stringAttribs = attribsInstances.getStringAttribs();
        const auto &integerAttribs = attribsInstances.getIntegerAttribs();
        const auto &doubleAttribs = attribsInstances.getDoubleAttribs();
        REQUIRE(CNAM == "SA010000-SA010-000U30R24-0");
        REQUIRE(stringAttribs.at("AHGT").front() == "F");
        REQUIRE(stringAttribs.at("FACC").front() == "SA010");
        REQUIRE(stringAttribs.at("MODT").front() == "T");

        REQUIRE(integerAttribs.at("LPN").front() == 5);
        REQUIRE(integerAttribs.at("RTAI").front() == 100);
        REQUIRE(integerAttribs.at("CMIX").front() == 1);
        REQUIRE(integerAttribs.at("FSC").front() == 0);

        REQUIRE(doubleAttribs.at("WGP").front() == Approx(0.0));
        REQUIRE(doubleAttribs.at("HGT").front() == Approx(0.0));
    }

    SECTION("Test invalid file extension")
    {
        std::filesystem::path CDBPath = dataPath / "RoadNetwork";
        std::filesystem::path vectorFile = CDBPath / "Tiles" / "N32" / "W118" / "201_RoadNetwork" / "LC"
                                           / "U0" / "N32W118_D201_S002_T003_LC05_U0_R0.tif";

        auto vector = CDBGeometryVectors::createFromFile(vectorFile, CDBPath);
        REQUIRE(vector == std::nullopt);
    }

    SECTION("Test invalid file component selector")
    {
        std::filesystem::path CDBPath = dataPath / "RoadNetwork";
        std::filesystem::path vectorFile = CDBPath / "Tiles" / "N32" / "W118" / "201_RoadNetwork" / "LC"
                                           / "U0" / "N32W118_D201_S002_T004_LC05_U0_R0.dbf";

        auto vector = CDBGeometryVectors::createFromFile(vectorFile, CDBPath);
        REQUIRE(vector == std::nullopt);
    }
}

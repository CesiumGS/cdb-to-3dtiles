#include "CDBGeometryVectors.h"
#include "CDBTo3DTiles.h"
#include "Config.h"
#include <doctest/doctest.h>
#include <filesystem>

using namespace CDBTo3DTiles;

TEST_SUITE_BEGIN("CDBGeometryVectors");

TEST_CASE("Test create CDBGeometryVector")
{
    SUBCASE("Test valid file with line geometry")
    {
        std::filesystem::path CDBPath = dataPath / "RoadNetwork";
        std::filesystem::path vectorFile = CDBPath / "Tiles" / "N32" / "W118" / "201_RoadNetwork" / "LC"
                                           / "U0" / "N32W118_D201_S002_T003_LC05_U0_R0.dbf";

        auto vector = CDBGeometryVectors::createFromFile(vectorFile, CDBPath);
        CHECK(vector != std::nullopt);

        const auto &mesh = vector->getMesh();
        CHECK(mesh.primitiveType == PrimitiveType::Lines);
        CHECK(mesh.positions.size() > 0);
        CHECK(mesh.positionRTCs.size() > 0);
        CHECK(mesh.normals.size() == 0);
        CHECK(mesh.UVs.size() == 0);

        // check instance attributes share the same class attribute
        auto attribsInstances = vector->getInstancesAttributes();
        size_t instancesCount = attribsInstances.getInstancesCount();
        CHECK(instancesCount == 8);

        const auto &CNAMs = attribsInstances.getCNAMs();
        for (const auto &CNAM : CNAMs) {
            CHECK(CNAM == "AP030000-AP030-000U31R31-0");
        }

        const auto &stringAttribs = attribsInstances.getStringAttribs();
        const auto &integerAttribs = attribsInstances.getIntegerAttribs();
        const auto &doubleAttribs = attribsInstances.getDoubleAttribs();
        for (size_t i = 0; i < instancesCount; ++i) {
            CHECK(stringAttribs.at("AHGT")[i] == "F");
            CHECK(stringAttribs.at("FACC")[i] == "AP030");
            CHECK(stringAttribs.at("MODT")[i] == "T");

            CHECK(integerAttribs.at("CMIX")[i] == 3);
            CHECK(integerAttribs.at("DIR")[i] == 3);
            CHECK(integerAttribs.at("FSC")[i] == 0);
            CHECK(integerAttribs.at("LTN")[i] == 2);
            CHECK(integerAttribs.at("TRF")[i] == 4);

            CHECK(doubleAttribs.at("HGT")[i] == 0.0);
        }

        CHECK(stringAttribs.at("EJID").size() == instancesCount);
        CHECK(stringAttribs.at("SJID").size() == instancesCount);
        CHECK(integerAttribs.at("LENL").size() == instancesCount);
        CHECK(integerAttribs.at("RTAI").size() == instancesCount);
        CHECK(doubleAttribs.at("WGP").size() == instancesCount);
    }

    SUBCASE("Test valid file with point geometry")
    {
        std::filesystem::path CDBPath = dataPath / "GSFeature";
        std::filesystem::path vectorFile = CDBPath / "Tiles" / "N32" / "W118" / "100_GSFeature" / "LC" / "U0"
                                           / "N32W118_D100_S004_T001_LC01_U0_R0.dbf";

        auto vector = CDBGeometryVectors::createFromFile(vectorFile, CDBPath);
        CHECK(vector != std::nullopt);

        const auto &mesh = vector->getMesh();
        CHECK(mesh.primitiveType == PrimitiveType::Points);
        CHECK(mesh.positions.size() == 1);
        CHECK(mesh.positionRTCs.size() == 1);
        CHECK(mesh.normals.size() == 0);
        CHECK(mesh.UVs.size() == 0);

        // check instance attributes share the same class attribute
        auto attribsInstances = vector->getInstancesAttributes();
        size_t instancesCount = attribsInstances.getInstancesCount();
        CHECK(instancesCount == 1);

        const auto &CNAM = attribsInstances.getCNAMs().front();
        const auto &stringAttribs = attribsInstances.getStringAttribs();
        const auto &integerAttribs = attribsInstances.getIntegerAttribs();
        const auto &doubleAttribs = attribsInstances.getDoubleAttribs();
        CHECK(CNAM == "GB010000-GB010-000U44R51-0");
        CHECK(stringAttribs.at("APID").front() == "KNZY");
        CHECK(stringAttribs.at("RWID").front() == "");
        CHECK(stringAttribs.at("AHGT").front() == "F");
        CHECK(stringAttribs.at("FACC").front() == "GB010");

        CHECK(integerAttribs.at("LPH").front() == 0);
        CHECK(integerAttribs.at("RTAI").front() == 100);
        CHECK(integerAttribs.at("CMIX").front() == 3);
        CHECK(integerAttribs.at("FSC").front() == 0);
        CHECK(integerAttribs.at("LTYP").front() == 0);

        CHECK(doubleAttribs.at("AO1").front() == doctest::Approx(0.0));
    }

    SUBCASE("Test valid file with polygon geometry")
    {
        std::filesystem::path CDBPath = dataPath / "HydrographyNetwork";
        std::filesystem::path vectorFile = CDBPath / "Tiles" / "N32" / "W118" / "204_HydrographyNetwork"
                                           / "LC" / "U0" / "N32W118_D204_S002_T005_LC06_U0_R0.dbf";

        auto vector = CDBGeometryVectors::createFromFile(vectorFile, CDBPath);
        CHECK(vector != std::nullopt);

        const auto &mesh = vector->getMesh();
        CHECK(mesh.primitiveType == PrimitiveType::Triangles);
        CHECK(mesh.positions.size() > 0);
        CHECK(mesh.positionRTCs.size() > 0);
        CHECK(mesh.normals.size() == 0);
        CHECK(mesh.UVs.size() == 0);

        // check instance attributes share the same class attribute
        auto attribsInstances = vector->getInstancesAttributes();
        size_t instancesCount = attribsInstances.getInstancesCount();
        CHECK(instancesCount == 1);

        const auto &CNAM = attribsInstances.getCNAMs().front();
        const auto &stringAttribs = attribsInstances.getStringAttribs();
        const auto &integerAttribs = attribsInstances.getIntegerAttribs();
        const auto &doubleAttribs = attribsInstances.getDoubleAttribs();
        CHECK(CNAM == "SA010000-SA010-000U30R24-0");
        CHECK(stringAttribs.at("AHGT").front() == "F");
        CHECK(stringAttribs.at("FACC").front() == "SA010");
        CHECK(stringAttribs.at("MODT").front() == "T");

        CHECK(integerAttribs.at("LPN").front() == 5);
        CHECK(integerAttribs.at("RTAI").front() == 100);
        CHECK(integerAttribs.at("CMIX").front() == 1);
        CHECK(integerAttribs.at("FSC").front() == 0);

        CHECK(doubleAttribs.at("WGP").front() == doctest::Approx(0.0));
        CHECK(doubleAttribs.at("HGT").front() == doctest::Approx(0.0));
    }

    SUBCASE("Test invalid file extension")
    {
        std::filesystem::path CDBPath = dataPath / "RoadNetwork";
        std::filesystem::path vectorFile = CDBPath / "Tiles" / "N32" / "W118" / "201_RoadNetwork" / "LC"
                                           / "U0" / "N32W118_D201_S002_T003_LC05_U0_R0.tif";

        auto vector = CDBGeometryVectors::createFromFile(vectorFile, CDBPath);
        CHECK(vector == std::nullopt);
    }

    SUBCASE("Test invalid file component selector")
    {
        std::filesystem::path CDBPath = dataPath / "RoadNetwork";
        std::filesystem::path vectorFile = CDBPath / "Tiles" / "N32" / "W118" / "201_RoadNetwork" / "LC"
                                           / "U0" / "N32W118_D201_S002_T004_LC05_U0_R0.dbf";

        auto vector = CDBGeometryVectors::createFromFile(vectorFile, CDBPath);
        CHECK(vector == std::nullopt);
    }
}

TEST_SUITE_END();
project(Tests)

add_executable(Tests
    CombineTilesetsTest.cpp
    CDBTileTest.cpp
    CDBTilesetTest.cpp
    CDBGeoCellTest.cpp
    CDBElevationTest.cpp
    CDBGeometryVectorsTest.cpp
    CDBGTModelsTest.cpp
    CDBGSModelsTest.cpp
    GltfTest.cpp
    main.cpp)

target_link_libraries(Tests
    PRIVATE
        Catch2::Catch2
        Core
        CDBTo3DTiles)

set_property(TARGET Tests
    PROPERTY
        CDBTo3DTiles_INCLUDE_PRIVATE 1)

set_property(TARGET Tests
    PROPERTY
        CDBTo3DTiles_THIRD_PARTY_INCLUDE_PRIVATE 1)

configure_project(Tests)

target_compile_definitions(Tests PUBLIC TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Data")

project(CDBTo3DTiles)

find_package(GDAL 3.0.4 REQUIRED)

add_library(CDBTo3DTiles
    src/Scene.cpp
    src/Gltf.cpp
    src/TileFormatIO.cpp
    src/CDBGeometryVectors.cpp
    src/CDBElevation.cpp
    src/CDBImagery.cpp
    src/CDBModels.cpp
    src/CDBAttributes.cpp
    src/CDBDataset.cpp
    src/CDBGeoCell.cpp
    src/CDBTile.cpp
    src/CDBTileset.cpp
    src/CDB.cpp
    src/CDBTo3DTiles.cpp
    src/ConverterImpl.cpp)

set(PRIVATE_INCLUDE_PATHS
    ${PROJECT_SOURCE_DIR}/src
)

set(PRIVATE_THIRD_PARTY_INCLUDE_PATHS
    ${osg_INCLUDE_DIRS}
    ${earcut_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
    ${GDAL_INCLUDE_DIRS}
    ${tinygltf_INCLUDE_DIRS}
    ${LIBMORTON_INCLUDE_DIR}
)

target_include_directories(CDBTo3DTiles
    SYSTEM PUBLIC
        "$<BUILD_INTERFACE:$<$<BOOL:$<TARGET_PROPERTY:CDBTo3DTiles_THIRD_PARTY_INCLUDE_PRIVATE>>:${PRIVATE_THIRD_PARTY_INCLUDE_PATHS}>>"

    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        "$<BUILD_INTERFACE:$<$<BOOL:$<TARGET_PROPERTY:CDBTo3DTiles_INCLUDE_PRIVATE>>:${PRIVATE_INCLUDE_PATHS}>>")

target_link_libraries(CDBTo3DTiles
    PRIVATE
        osgdb_openflight
        osgdb_rgb
        osgdb_png
        osgdb_jpeg
        osgdb_zip
        osgDB
        osg
        OpenThreads
        meshoptimizer
        Core
        ${GDAL_LIBRARIES})

set_property(TARGET CDBTo3DTiles
    PROPERTY
        CDBTo3DTiles_INCLUDE_PRIVATE 1)

set_property(TARGET CDBTo3DTiles
    PROPERTY
        CDBTo3DTiles_THIRD_PARTY_INCLUDE_PRIVATE 1)

configure_project(CDBTo3DTiles)

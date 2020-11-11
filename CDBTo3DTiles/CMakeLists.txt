project(CDBTo3DTiles)

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
    src/CDBTo3DTiles.cpp)

set(PRIVATE_INCLUDE_PATHS
    ${PROJECT_SOURCE_DIR}/src
)

set(PRIVATE_THIRD_PARTY_INCLUDE_PATHS
    ${osg_INCLUDE_DIRS}
    ${earcut_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
    /usr/local/include
    /usr/include
    ${tinygltf_INCLUDE_DIRS}
)

target_include_directories(CDBTo3DTiles
    SYSTEM PUBLIC
        "$<BUILD_INTERFACE:$<$<BOOL:$<TARGET_PROPERTY:CDBTo3DTiles_THIRD_PARTY_INCLUDE_PRIVATE>>:${PRIVATE_THIRD_PARTY_INCLUDE_PATHS}>>"

    PUBLIC
        ${PROJECT_SOURCE_DIR}/include
        "$<BUILD_INTERFACE:$<$<BOOL:$<TARGET_PROPERTY:CDBTo3DTiles_INCLUDE_PRIVATE>>:${PRIVATE_INCLUDE_PATHS}>>")

add_library(libtiff SHARED IMPORTED)
set_target_properties(libtiff PROPERTIES IMPORTED_LOCATION /usr/lib64/libtiff.so)

add_library(libjpeg STATIC IMPORTED)
set_target_properties(libjpeg PROPERTIES IMPORTED_LOCATION /usr/lib64/libjpeg.a)

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
        /usr/local/lib/libgdal.a
        /usr/local/lib/libsqlite3.a
        /usr/local/lib/libproj.a
        libtiff
        libjpeg)

set_property(TARGET CDBTo3DTiles
    PROPERTY
        CDBTo3DTiles_INCLUDE_PRIVATE 1)

set_property(TARGET CDBTo3DTiles
    PROPERTY
        CDBTo3DTiles_THIRD_PARTY_INCLUDE_PRIVATE 1)

#target_compile_options(CDBTo3DTiles  PRIVATE -lgdal)
target_compile_options(CDBTo3DTiles  PRIVATE -ljbig)

configure_project(CDBTo3DTiles)

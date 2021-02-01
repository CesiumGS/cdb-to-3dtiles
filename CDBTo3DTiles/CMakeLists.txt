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
    ${glm_INCLUDE_DIR}
)

get_target_property(VAR CDBTo3DTiles CDBTo3DTiles_THIRD_PARTY_INCLUDE_PRIVATE)
message("This is from line 38:" ${VAR})
# target_include_directories(CDBTo3DTiles
#     SYSTEM PUBLIC
#         "$<BUILD_INTERFACE:$<$<BOOL:$<TARGET_PROPERTY:CDBTo3DTiles_THIRD_PARTY_INCLUDE_PRIVATE>>:${PRIVATE_THIRD_PARTY_INCLUDE_PATHS}>>"

#     PUBLIC
#         ${PROJECT_SOURCE_DIR}/include
#         ${CMAKE_CURRENT_LIST_DIR}/Core/include
#         foreach(INCLUDE_DIR ${PRIVATE_THIRD_PARTY_INCLUDE_PATHS})

#         # "$<BUILD_INTERFACE:<$<BOOL:$<TARGET_PROPERTY:CDBTo3DTiles_INCLUDE_PRIVATE>>:${PRIVATE_INCLUDE_PATHS}>>"
# )

foreach(INCLUDE_DIR ${PRIVATE_THIRD_PARTY_INCLUDE_PATHS})
  message(${INCLUDE_DIR})
  target_include_directories(CDBTo3DTiles
    SYSTEM PUBLIC
      ${INCLUDE_DIR}
  )
endforeach()

message(${CMAKE_CURRENT_LIST_DIR}/Core/include)
target_include_directories(CDBTo3DTiles
    PUBLIC
      ${CMAKE_CURRENT_LIST_DIR}/../Core/include
      ${PROJECT_SOURCE_DIR}/include
      ${PRIVATE_INCLUDE_PATHS}
)

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
link_libraries(Core)

set_property(TARGET CDBTo3DTiles
    PROPERTY
        CDBTo3DTiles_INCLUDE_PRIVATE 1)

set_property(TARGET CDBTo3DTiles
    PROPERTY
        CDBTo3DTiles_THIRD_PARTY_INCLUDE_PRIVATE 1)

configure_project(CDBTo3DTiles)

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
    src/CDBTo3DTiles.cpp
    src/CDBTilesetBuilder.cpp)

set(PRIVATE_INCLUDE_PATHS
    ${PROJECT_SOURCE_DIR}/src
)

set(PRIVATE_THIRD_PARTY_INCLUDE_PATHS
    ${CONAN_INCLUDE_DIRS}
)

foreach(INCLUDE_DIR ${PRIVATE_THIRD_PARTY_INCLUDE_PATHS})
  target_include_directories(CDBTo3DTiles
    SYSTEM PUBLIC
      ${INCLUDE_DIR}
  )
endforeach()

target_include_directories(CDBTo3DTiles
    PUBLIC
      ${CMAKE_CURRENT_LIST_DIR}/../Core/include
      ${PROJECT_SOURCE_DIR}/include
      ${PRIVATE_INCLUDE_PATHS}
)

target_link_libraries(CDBTo3DTiles
    PRIVATE
        Core
        ${CONAN_LIBS})
link_libraries(Core)

set_property(TARGET CDBTo3DTiles
    PROPERTY
        CDBTo3DTiles_INCLUDE_PRIVATE 1)

set_property(TARGET CDBTo3DTiles
    PROPERTY
        CDBTo3DTiles_THIRD_PARTY_INCLUDE_PRIVATE 1)

configure_project(CDBTo3DTiles)

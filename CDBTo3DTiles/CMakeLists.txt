project(CDBTo3DTiles)

add_library(
  CDBTo3DTiles
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

set(PRIVATE_INCLUDE_PATHS ${PROJECT_SOURCE_DIR}/src)

target_include_directories(
  CDBTo3DTiles PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../Core/include
                      ${PROJECT_SOURCE_DIR}/include ${PRIVATE_INCLUDE_PATHS})

target_link_libraries(
  CDBTo3DTiles
  cxxopts::cxxopts
  GDAL::GDAL
  glm::glm
  earcut::earcut
  libmorton::libmorton
  meshoptimizer::meshoptimizer
  nlohmann_json::nlohmann_json
  OPENSCENEGRAPH::OPENSCENEGRAPH
  TinyGLTF::TinyGLTF)

set_property(TARGET CDBTo3DTiles PROPERTY CDBTo3DTiles_INCLUDE_PRIVATE 1)

set_property(TARGET CDBTo3DTiles
             PROPERTY CDBTo3DTiles_THIRD_PARTY_INCLUDE_PRIVATE 1)

configure_project(CDBTo3DTiles)

project(Tests)

add_executable(
  Tests
  CDBGTModelsTest.cpp
  # CDBTileTest.cpp
  # CDBTilesetTest.cpp
  # CDBTilesetBuilderTest.cpp
  # GltfTest.cpp
  main.cpp)

target_link_libraries(
  Tests
  Core
  CDBTo3DTiles
  cxxopts::cxxopts
  doctest::doctest
  OPENSCENEGRAPH::OPENSCENEGRAPH
  GDAL::GDAL
  glm::glm
  earcut::earcut
  TinyGLTF::TinyGLTF
  meshoptimizer::meshoptimizer
  earcut::earcut
  libmorton::libmorton
  nlohmann_json::nlohmann_json)

set_property(TARGET Tests PROPERTY CDBTo3DTiles_INCLUDE_PRIVATE 1)

set_property(TARGET Tests PROPERTY CDBTo3DTiles_THIRD_PARTY_INCLUDE_PRIVATE 1)

configure_project(Tests)

target_compile_definitions(
  Tests PUBLIC TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/Data")

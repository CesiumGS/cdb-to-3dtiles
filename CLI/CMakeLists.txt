project(CLI)

add_executable(CDBConverter main.cpp)
find_package(cxxopts)
target_include_directories(CDBConverter SYSTEM
                           PRIVATE ${cxxopts_INCLUDE_DIRS_DEBUG})

target_link_libraries(
  CDBConverter
  Core
  CDBTo3DTiles
  cxxopts::cxxopts
  OPENSCENEGRAPH::OPENSCENEGRAPH
  GDAL::GDAL
  glm::glm
  earcut::earcut
  TinyGLTF::TinyGLTF
  meshoptimizer::meshoptimizer
  earcut::earcut
  libmorton::libmorton
  nlohmann_json::nlohmann_json)

configure_project(CDBConverter)

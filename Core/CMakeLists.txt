project(Core)

set(sources
    "src/MathLib.cpp"
    "src/Transforms.cpp"
    "src/IntersectionTests.cpp"
    "src/EllipsoidTangentPlane.cpp"
    "src/Ellipsoid.cpp"
    "src/Plane.cpp"
    "src/Ray.cpp"
    "src/BoundingRegion.cpp"
    "src/GlobeRectangle.cpp"
    "src/Cartographic.cpp"
    "src/FileUtil.cpp")

add_library(Core ${sources})
target_include_directories(
  Core
  PUBLIC ${PROJECT_SOURCE_DIR}/include
  PRIVATE ${PROJECT_SOURCE_DIR}/src)

target_link_libraries(Core glm::glm)

configure_project(Core)

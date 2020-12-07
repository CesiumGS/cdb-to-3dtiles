project(Core)

set(sources
    "src/MathUtility.cpp"
    "src/Transforms.cpp"
    "src/IntersectionTests.cpp"
    "src/EllipsoidTangentPlane.cpp"
    "src/Ellipsoid.cpp"
    "src/Plane.cpp"
    "src/Ray.cpp"
    "src/BoundingRegion.cpp"
    "src/GlobeRectangle.cpp"
    "src/Cartographic.cpp")

add_library(Core ${sources})
target_include_directories(Core
    SYSTEM PUBLIC
        ${glm_INCLUDE_DIRS}

    PUBLIC
        ${PROJECT_SOURCE_DIR}/include

    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
)

configure_project(Core)

project(Core)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(glm CONFIG REQUIRED)
find_package(GDAL REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(Boost COMPONENTS system filesystem REQUIRED)

set(sources
    "FLTLoader.cpp"
    "GDALDatasetWrapper.cpp"
    "TIFLoader.cpp"
    "Ellipsoid.cpp")

set(headers
    "FLTLoader.h"
    "TIFLoader.h"
    "GDALDatasetWrapper.h"
    "TerrainMesh.h"
    "BoundRegion.h"
    "AABB.h"
    "Cartographic.h"
    "Ellipsoid.h")

# external flt
add_library(libflt SHARED IMPORTED)
set_target_properties(libflt PROPERTIES IMPORTED_LOCATION ${CMAKE_BINARY_DIR}/Shared/libflt/libflt.so)
add_library(Core ${sources} ${headers})
target_include_directories(Core
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS}
        ${GDAL_INCLUDE_DIRS})

target_link_libraries(Core
    PUBLIC
        libflt
        nlohmann_json
        nlohmann_json::nlohmann_json
        glm
        ${GDAL_LIBRARIES}
        ${Boost_FILESYSTEM_LIBRARY}
        ${Boost_SYSTEM_LIBRARY}
        )

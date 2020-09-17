project(CDBTo3DTiles)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_path(STB_INCLUDE_DIRS "stb.h")

set(sources
    "CDBTo3DTiles.cpp"
    "tiny_gltf.cpp"
    "CDB.cpp")

set(headers
    "CDBTo3DTiles.h"
    "tiny_gltf.h"
    "CDB.h")

add_library(CDBTo3DTiles ${sources} ${headers})
target_include_directories(CDBTo3DTiles
    PUBLIC
        ${STB_INCLUDE_DIRS}
        ${PROJECT_SOURCE_DIR})

target_link_libraries(CDBTo3DTiles
    PUBLIC
        Core)

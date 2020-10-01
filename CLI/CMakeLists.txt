project(CLI)

add_executable(CDBConverter main.cpp)
target_link_libraries(CDBConverter CesiumUtility CesiumGeospatial CesiumGeometry Cesium3DTiles)

if (MSVC)
    target_compile_options(CDBConverter PRIVATE /W4 /WX /wd4201)
else()
    target_compile_options(CDBConverter PRIVATE -Werror -Wall -Wextra -Wconversion -Wpedantic -Wshadow)
endif()

set_target_properties(CDBConverter PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

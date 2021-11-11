project(CLI)

add_executable(CDBConverter main.cpp)

target_include_directories(CDBConverter
    SYSTEM PRIVATE
        ${CONAN_INCLUDE_DIRS})

target_link_libraries(CDBConverter
    PRIVATE
        CDBTo3DTiles
        ${CONAN_LIB_DIRS_GDAL}/libgdal.29.dylib)

message(${CONAN_LIB_DIRS_GDAL})

configure_project(CDBConverter)

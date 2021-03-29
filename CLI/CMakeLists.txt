project(CLI)
find_package(OpenMP REQUIRED)
add_executable(CDBConverter main.cpp)

target_include_directories(CDBConverter
    SYSTEM PRIVATE
        ${cxxopts_INCLUDE_DIRS})

target_link_libraries(CDBConverter
    PRIVATE
        CDBTo3DTiles
        OpenMP::OpenMP_CXX)

configure_project(CDBConverter)

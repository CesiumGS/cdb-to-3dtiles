project(CLI)

add_executable(CDBConverter main.cpp)

target_include_directories(CDBConverter
    SYSTEM PRIVATE
        ${cxxopts_INCLUDE_DIRS})

target_link_libraries(CDBConverter
    PRIVATE
        CDBTo3DTiles)

configure_project(CDBConverter)

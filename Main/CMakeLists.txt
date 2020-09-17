project(Main)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Boost COMPONENTS program_options REQUIRED)

set(sources
    "main.cpp")

add_executable(Main ${sources})

target_include_directories(Main
    PUBLIC
        ${PROJECT_SOURCE_DIR}
        ${Boost_INCLUDE_DIRS})

target_link_libraries(Main
    PUBLIC
        ${Boost_PROGRAM_OPTIONS_LIBRARY}
        CDBTo3DTiles)

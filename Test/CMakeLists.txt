project(Test)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Catch2 CONFIG REQUIRED)

set(sources
    "main.cpp")

add_executable(Test ${sources})
target_link_libraries(Test PRIVATE Core CDBTo3DTiles Catch2::Catch2)

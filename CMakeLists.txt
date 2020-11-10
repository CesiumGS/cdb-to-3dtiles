cmake_minimum_required(VERSION 3.15)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(
	CDBTo3DTiles
	VERSION 0.1.0
	LANGUAGES C CXX
)
enable_testing()

function(configure_project target_name)
    if (MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /WX /wd4201)
    else()
        target_compile_options(${target_name} PRIVATE -Werror -Wall -Wextra -Wconversion -Wpedantic -Wshadow)
    endif()
endfunction()

# add third party libraries
add_subdirectory(extern)

# main target built
add_subdirectory(Core)
add_subdirectory(CDBTo3DTiles)
add_subdirectory(CLI)

# typically needed if we are at the top level project
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_subdirectory(Tests)
endif()

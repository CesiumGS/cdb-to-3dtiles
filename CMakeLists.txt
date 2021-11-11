cmake_minimum_required(VERSION 3.15)
project(
	CDBTo3DTiles
	VERSION 0.0.0
	LANGUAGES C CXX
)
enable_testing()

function(configure_project target_name)
    if (MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /WX /wd4201)
    else()
        target_compile_options(${target_name} PRIVATE -Werror -Wall -Wextra -Wpedantic -Wshadow)
    endif()

    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
endfunction()

set(CONAN_DISABLE_CHECK_COMPILER On)
# add third party libraries
include(conan/conanbuildinfo.cmake)
conan_basic_setup()

# main target built
add_subdirectory(Core)
add_subdirectory(CDBTo3DTiles)
add_subdirectory(CLI)

# typically needed if we are at the top level project
# if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
#     add_subdirectory(Tests)
# endif()

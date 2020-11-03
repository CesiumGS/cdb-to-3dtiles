cmake_minimum_required(VERSION 3.15)
project(
	CDBTo3DTiles
	VERSION 0.1.0
	LANGUAGES C CXX
)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()
string(TOUPPER ${CMAKE_BUILD_TYPE} UPPER_CMAKE_BUILD_TYPE)
string(TOLOWER ${CMAKE_BUILD_TYPE} LOWER_CMAKE_BUILD_TYPE)

if ("${UPPER_CMAKE_BUILD_TYPE}" STREQUAL "DEBUG")
    set(IS_DEBUG TRUE)
else ()
    set(IS_DEBUG FALSE)
endif ()

# Travis has --as-needed set as default, which is atypical
# Since we list almost all static libraries anyway, we
# set --no-as-needed globally
if (NOT MSVC)
    set(CMAKE_EXE_LINKER_FLAGS "-Wl,--no-as-needed -pthread")
endif()

# Globally disallow non-static libraries from being linked
if (MSVC)
    set(STATIC_FIND_LIBRARY_SUFFIXES .lib)
else()
    set(STATIC_FIND_LIBRARY_SUFFIXES .a)
endif()
set(CMAKE_FIND_LIBRARY_SUFFIXES ${STATIC_FIND_LIBRARY_SUFFIXES})

# Use solution folders.
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Preprocessor Definitions/Compiler Flags
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if (MSVC)
    add_definitions(-D_SCL_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
    add_definitions(-D_SCL_SECURE_NO_DEPRECATE)
else ()
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")

    if (CMAKE_COMPILER_IS_GNUCXX)
        # clang does not accept stripping from the CXX flags
        # adding -s only for gcc.
        set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
        set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s")
    endif()

    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fvisibility=hidden")
    # Enable generating compile_commands.json file for use by IDEs such as VSCode
    set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
endif()

set(ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}")
set(BUILD_DIR "${PROJECT_BINARY_DIR}")
set(BUILD_LIB_DIR "${PROJECT_BINARY_DIR}/lib")
set(BUILD_DEBUG_LIB_DIR "${PROJECT_BINARY_DIR}/lib/Debug")
set(BUILD_RELEASE_LIB_DIR "${PROJECT_BINARY_DIR}/lib/Release")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${BUILD_LIB_DIR})

set(BUILD_BIN_DIR "${PROJECT_BINARY_DIR}/bin")
set(BUILD_DEBUG_BIN_DIR "${PROJECT_BINARY_DIR}/bin/Debug")
set(BUILD_RELEASE_BIN_DIR "${PROJECT_BINARY_DIR}/bin/Release")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${BUILD_BIN_DIR})

if (WIN32)
    set(NULL_DEVICE NUL)
else ()
    set(NULL_DEVICE /dev/null)
endif ()

if (UNIX)
    # Link in filesystem
    set(GLOBAL_LINK_LIBRARIES stdc++fs)
endif()

# Sets the folder for MSVC projects
macro (set_folder target folder)
    set_target_properties(${target} PROPERTIES FOLDER ${folder})
endmacro ()

set(DATA_OUTPUT_DIR ${BUILD_BIN_DIR}/../Data)

function(configure_project target_name)
    if (MSVC)
        target_compile_options(${target_name} PRIVATE /W4 /WX /wd4201)
    else()
        target_compile_options(${target_name} PRIVATE -Werror -Wall -Wextra -Wconversion -Wpedantic -Wshadow)
    endif()

    set_target_properties(${target_name} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
    )
endfunction()

# add third party libraries
add_subdirectory(extern)

enable_testing()

# main target built
add_subdirectory(Core)
add_subdirectory(CDBTo3DTiles)
add_subdirectory(CLI)

# typically needed if we are at the top level project
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    add_subdirectory(Tests)
endif()

if (WIN32)
    # Push data into the bin directory for each build configuration
    FILE(COPY ${DATA_OUTPUT_DIR} DESTINATION ${BUILD_BIN_DIR}/Debug)
    FILE(COPY ${DATA_OUTPUT_DIR} DESTINATION ${BUILD_BIN_DIR}/Release)
else()
    FILE(COPY ${DATA_OUTPUT_DIR} DESTINATION ${BUILD_BIN_DIR})
endif()

MESSAGE(STATUS "Building ${CMAKE_BUILD_TYPE}")

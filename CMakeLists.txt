cmake_minimum_required(VERSION 3.15)
project(
  CDBTo3DTiles
  VERSION 0.0.0
  LANGUAGES C CXX)
enable_testing()

function(configure_project target_name)
  if(MSVC)
    target_compile_options(${target_name} PRIVATE /W4 /WX /wd4201)
  else()
    target_compile_options(${target_name} PRIVATE -Werror -Wall -Wextra
                                                  -Wpedantic -Wshadow)
  endif()

  set_target_properties(
    ${target_name}
    PROPERTIES CXX_STANDARD 17
               CXX_STANDARD_REQUIRED YES
               CXX_EXTENSIONS NO)
endfunction()

# Download conan.cmake, if it does not exist.
if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(
    STATUS
      "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/conan-io/cmake-conan/v0.16.1/conan.cmake"
    "${CMAKE_BINARY_DIR}/conan.cmake"
    EXPECTED_HASH
      SHA256=396e16d0f5eabdc6a14afddbcfff62a54a7ee75c6da23f32f7a31bc85db23484
    TLS_VERIFY ON)
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_configure(
  REQUIRES
  cxxopts/2.2.1
  doctest/2.4.6
  earcut/2.2.3
  gdal/3.3.3
  glm/0.9.9.8
  libmorton/0.2.7
  meshoptimizer/0.16
  nlohmann_json/3.10.4
  openjpeg/2.4.0
  openscenegraph/3.6.5
  tinygltf/2.5.0
  OPTIONS
  gdal:shared=True
  gdal:with_openjpeg=True
  zlib:shared=True
  GENERATORS
  cmake_find_package_multi)

set(CONAN_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES}")
if(NOT CONAN_CONFIGURATION_TYPES)
  set(CONAN_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE})
endif()

set(CONAN_PACKAGE_INSTALL_FOLDER "${CMAKE_BINARY_DIR}/Conan_Packages")
list(APPEND CMAKE_MODULE_PATH ${CONAN_PACKAGE_INSTALL_FOLDER})
list(APPEND CMAKE_PREFIX_PATH ${CONAN_PACKAGE_INSTALL_FOLDER})

foreach(TYPE ${CONAN_CONFIGURATION_TYPES})
  conan_cmake_autodetect(settings BUILD_TYPE ${TYPE})
  conan_cmake_install(
    PATH_OR_REFERENCE
    ${CMAKE_BINARY_DIR}
    BUILD
    missing
    INSTALL_FOLDER
    ${CONAN_PACKAGE_INSTALL_FOLDER}
    REMOTE
    conancenter
    SETTINGS
    ${settings})
endforeach()

set(CMAKE_FIND_PACKAGE_PREFER_CONFIG TRUE)
find_package(glm)
find_package(GDAL)
find_package(doctest)
find_package(cxxopts)
find_package(earcut)
find_package(libmorton)
find_package(meshoptimizer)
find_package(nlohmann_json)
find_package(openjpeg)
find_package(OpenSceneGraph)
find_package(tinygltf)

# main target built
add_subdirectory(Core)
add_subdirectory(CDBTo3DTiles)
add_subdirectory(CLI)

# typically needed if we are at the top level project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
  add_subdirectory(Tests)
endif()

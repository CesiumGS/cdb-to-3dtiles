project(ThirdParty)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}")

set(ROOT_THIRDPARTY_DIR "${CMAKE_CURRENT_LIST_DIR}")

include(add_sqlite3)
include(add_proj)
include(add_gdal)

add_subdirectory(meshoptimizer)

add_subdirectory(cxxopts)

add_subdirectory(Catch2)

set(DYNAMIC_OPENTHREADS OFF CACHE BOOL "Build OpenThreads dynamic")
set(DYNAMIC_OPENSCENEGRAPH OFF CACHE BOOL "Build OSG dynamic")
set(BUILD_OSG_APPLICATIONS OFF CACHE BOOL "Build OSG Applications")
set(BUILD_OSG_DEPRECATED_SERIALIZERS OFF CACHE BOOL "Build OSG Applications")
set(BUILD_OSG_PLUGINS_BY_DEFAULT OFF CACHE BOOL "Build All default plugin")
set(BUILD_OSG_PLUGIN_OPENFLIGHT 1)
set(BUILD_OSG_PLUGIN_PNG 1)
set(BUILD_OSG_PLUGIN_JPEG 1)
set(BUILD_OSG_PLUGIN_ZIP 1)
set(BUILD_OSG_PLUGIN_RGB 1)
add_subdirectory(OpenSceneGraph)
set(osg_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/OpenSceneGraph/include ${CMAKE_CURRENT_BINARY_DIR}/OpenSceneGraph/include)
set(osg_INCLUDE_DIR ${osg_INCLUDE_DIR} PARENT_SCOPE)
set(osg_INCLUDE_DIRS ${osg_INCLUDE_DIR} PARENT_SCOPE)

set(cxxopts_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/cxxopts/include)
set(cxxopts_INCLUDE_DIR ${cxxopts_INCLUDE_DIR} PARENT_SCOPE)
set(cxxopts_INCLUDE_DIRS ${cxxopts_INCLUDE_DIR} PARENT_SCOPE)

set(glm_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/glm)
set(glm_INCLUDE_DIR ${glm_INCLUDE_DIR} PARENT_SCOPE)
set(glm_INCLUDE_DIRS ${glm_INCLUDE_DIR} PARENT_SCOPE)

set(earcut_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/earcut.hpp/include)
set(earcut_INCLUDE_DIR ${earcut_INCLUDE_DIR} PARENT_SCOPE)
set(earcut_INCLUDE_DIRS ${earcut_INCLUDE_DIR} PARENT_SCOPE)

set(tinygltf_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf)
set(tinygltf_INCLUDE_DIR ${tinygltf_INCLUDE_DIR} PARENT_SCOPE)
set(tinygltf_INCLUDE_DIRS ${tinygltf_INCLUDE_DIR} PARENT_SCOPE)

set(nlohmann_json_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/nlohmann_json/include)
set(nlohmann_json_INCLUDE_DIR ${nlohmann_json_INCLUDE_DIR} PARENT_SCOPE)
set(nlohmann_json_INCLUDE_DIRS ${nlohmann_json_INCLUDE_DIR} PARENT_SCOPE)

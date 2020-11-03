set(BUILD_CCT OFF CACHE BOOL "Build cct (coordinate conversion and transformation tool)")
set(BUILD_CS2CS OFF CACHE BOOL "Don't build cs2cs executable")
set(BUILD_GEOD OFF CACHE BOOL "Don't build geod executable")
set(BUILD_GIE OFF CACHE BOOL "Build gie (geospatial integrity investigation environment - a PROJ.4 test tool)")
set(BUILD_PROJ OFF CACHE BOOL "Don't build proj executable")
set(BUILD_PROJINFO OFF CACHE BOOL "Build projinfo (SRS and coordinate operation metadata/query tool)")
set(PROJ_TESTS OFF CACHE BOOL "Don't build tests")

set(BUILD_LIBPROJ_SHARED OFF CACHE BOOL "Always build static library")
set(PROJ4_SUBDIRECTORY "${ROOT_THIRDPARTY_DIR}/PROJ")
set(PROJ4_DATA_DIR ${PROJECT_BINARY_DIR}/../Data/PROJ)
set(PROJ4_OUTPUT_PATH ${PROJECT_BINARY_DIR}/PROJ)
set(PROJ4_LIB_PATH ${PROJ4_OUTPUT_PATH}/lib)
set(PROJ4_INCLUDE_DIR ${PROJ4_OUTPUT_PATH}/include)

set(PROJ_DICTIONARY null world other.extra nad27 GL27 nad83 nad.lst CH ITRF2000 ITRF2008 ITRF2014)
foreach(filename ${PROJ_DICTIONARY})
	file(COPY "${PROJ4_SUBDIRECTORY}/data/${filename}" DESTINATION ${PROJ4_DATA_DIR})
endforeach()

# Copy files
file(GLOB DATA_FILES "${ROOT_THIRDPARTY_DIR}/proj.4-data/*")
file(COPY ${DATA_FILES} DESTINATION ${PROJ4_DATA_DIR})

add_subdirectory(${PROJ4_SUBDIRECTORY})

target_include_directories(proj PUBLIC $<BUILD_INTERFACE:${PROJ4_INCLUDE_DIR}>)

# Copy the header files because GDAL needs them to be in a certain spot
file(GLOB INCLUDE_FILES "${ROOT_THIRDPARTY_DIR}/PROJ/src/*.h*")
file(COPY ${INCLUDE_FILES} DESTINATION ${PROJ4_INCLUDE_DIR})

file(GLOB INCLUDE_PROJ_FILES "${ROOT_THIRDPARTY_DIR}/PROJ/include/proj/*.h*")
file(COPY ${INCLUDE_PROJ_FILES} DESTINATION ${PROJ4_INCLUDE_DIR}/proj)

file(GLOB INCLUDE_PROJ_INTERNAL_FILES "${ROOT_THIRDPARTY_DIR}/PROJ/include/proj/internal/*.h*")
file(COPY ${INCLUDE_PROJ_INTERNAL_FILES} DESTINATION ${PROJ4_INCLUDE_DIR}/proj/internal)

file(GLOB INCLUDE_PROJ_NLOHMANN_JSON_FILES "${ROOT_THIRDPARTY_DIR}/PROJ/include/proj/internal/nlohmann/*.h*")
file(COPY ${INCLUDE_PROJ_NLOHMANN_JSON_FILES} DESTINATION ${PROJ4_INCLUDE_DIR}/proj/internal/nlohmann/)

# Remove the _d so GDAL recognizes the static library
set_target_properties (proj PROPERTIES
      DEBUG_POSTFIX "")

set(PROJ4_INCLUDE_DIR ${PROJ4_INCLUDE_DIR} PARENT_SCOPE)
set(PROJ4_INCLUDE_DIRS ${PROJ4_INCLUDE_DIR} PARENT_SCOPE)
set(PROJ4_LIBRARY proj)
set(PROJ4_LIBRARY ${PROJ4_LIBRARY} PARENT_SCOPE)
set(PROJ4_LIBRARIES ${PROJ4_LIBRARY})
set(PROJ4_LIBRARIES ${PROJ4_LIBRARY} PARENT_SCOPE)

set(PROJ4_DEPENDENCIES proj)
set(PROJ4_DEPENDENCIES ${PROJ4_DEPENDENCIES} PARENT_SCOPE)

# After the proj.db is generated, copy it to the Data directory along with the bin/Data directory
add_custom_target(copy_proj_db ALL)
add_custom_command(TARGET copy_proj_db POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJ4_OUTPUT_PATH}/data/proj.db" "${PROJ4_DATA_DIR}/proj.db")
if (WIN32)
    add_custom_command(TARGET copy_proj_db POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJ4_OUTPUT_PATH}/data/proj.db" "${BUILD_BIN_DIR}/$(Configuration)/Data/PROJ/proj.db")
else()
    add_custom_command(TARGET copy_proj_db POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PROJ4_OUTPUT_PATH}/data/proj.db" "${BUILD_BIN_DIR}/Data/PROJ/proj.db")
endif()


add_dependencies(copy_proj_db ${SQLITE3_DEPENDENCIES} generate_proj_db)

# Generate the proj.db when proj gets built
add_dependencies(proj copy_proj_db)

set_folder(proj ThirdParty/proj)
set_folder(generate_all_sql_in ThirdParty/proj)
set_folder(generate_proj_db ThirdParty/proj)
set_folder(copy_proj_db ThirdParty/proj)

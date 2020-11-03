include(ExternalProject)

# Set output directories
set(GDAL_SOURCE_DIR ${ROOT_THIRDPARTY_DIR}/gdal/gdal)
set(GDAL_OUTPUT_DIR ${PROJECT_BINARY_DIR}/gdal)
set(GDAL_DATA_DIR ${PROJECT_BINARY_DIR}/../Data/gdal)
set(GDAL_INCLUDE_DIR ${GDAL_OUTPUT_DIR}/include)
set(GDAL_LIBRARY_DIR ${GDAL_OUTPUT_DIR}/lib)

function(cat IN_FILE OUT_FILE)
  file(READ ${IN_FILE} CONTENTS)
  file(APPEND ${OUT_FILE} "${CONTENTS}")
endfunction()

if (MSVC)
    #Create output libraries
    file(MAKE_DIRECTORY ${GDAL_LIBRARY_DIR}/Release)
    file(MAKE_DIRECTORY ${GDAL_LIBRARY_DIR}/Debug)

    # No Configure Command
    set(CONFIG_CMD echo configure step not required on this platform)

    # Build Command
    #  1. Run setup_env.bat to setup the build environment
    #  2. cd into the GDAL source directory
    #  3. Run nmake to build GDAL
    #  4. Move gdal.lib into the correct directory
    #  5. Delete *.obj files so if we built Release, then the Debug build won't use the existing obj files
    set(BUILD_CMD cmd "/C ${ROOT_THIRDPARTY_DIR}/setup_env.bat && \
                       cd  ${GDAL_SOURCE_DIR} && \
                       nmake /f makefile.vc clean EXT_NMAKE_OPT=${ROOT_THIRDPARTY_DIR}/gdal-nmake.opt && \
                       nmake /f makefile.vc staticlib EXT_NMAKE_OPT=${ROOT_THIRDPARTY_DIR}/gdal-nmake.opt $(Configuration)=1 BUILD_DIR=${PROJECT_BINARY_DIR}/.. && \
                       move /Y gdal.lib ${GDAL_LIBRARY_DIR}/$(Configuration) && \
                       del /s /q *.obj")

    # No Install Command
    set(INSTALL_CMD echo install step not required on this platform)

    # Copy header files
    FILE(GLOB PUBLIC_HEADERS
        "${GDAL_SOURCE_DIR}/**/*.h"
        "${GDAL_SOURCE_DIR}/ogr/ogrsf_frmts/*.h"
        "${GDAL_SOURCE_DIR}/frmts/gtiff/*.h"
        "${GDAL_SOURCE_DIR}/frmts/gtiff/libgeotiff/*.h"
        "${GDAL_SOURCE_DIR}/frmts/gtiff/libgeotiff/*.inc"
        "${GDAL_SOURCE_DIR}/frmts/gtiff/libtiff/*.h"
    )

    # Prepare a temporary file to "cat" to:
    set(GDAL_TEMP_CONFIG_FILE ${GDAL_INCLUDE_DIR}/cpl_config.h.in)
    file(WRITE ${GDAL_TEMP_CONFIG_FILE} "")

    # Call the "cat" function for each input file
    set(GDAL_CONFIG_FILE_VC_EXT begin common no_dll end)
    foreach(EXTENSION ${GDAL_CONFIG_FILE_VC_EXT})
      cat(${GDAL_SOURCE_DIR}/port/cpl_config.h.vc.${EXTENSION} ${GDAL_TEMP_CONFIG_FILE})
    endforeach()

    # Copy the temporary file to the final location
    configure_file(${GDAL_TEMP_CONFIG_FILE} ${GDAL_INCLUDE_DIR}/cpl_config.h COPYONLY)

    # Set library path
    set(GDAL_LIBRARY optimized ${GDAL_LIBRARY_DIR}/Release/gdal.lib
                     debug ${GDAL_LIBRARY_DIR}/Debug/gdal.lib)
else ()
    # We need to touch config.rpath so ./configure doesn't fail during the continous integration build
    set(_gdal_ld_flags "-L${BUILD_LIB_DIR} -lsqlite3 -pthread")
    if (IS_DEBUG)
        set(_gdal_c_flags "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_DEBUG}")
        set(_gdal_cxx_flags "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}")
    else ()
        set(_gdal_c_flags "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_RELEASE}")
        set(_gdal_cxx_flags "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE}")
    endif()

    set(CONFIG_CMD cd ${GDAL_SOURCE_DIR}  && touch config.rpath && ./configure LDFLAGS=${_gdal_ld_flags} 
        CXXFLAGS=${_gdal_cxx_flags} CFLAGS=${_gdal_c_flags} --with-libz=internal --without-qhull --without-gif --without-png --with-jpeg=internal
         --with-libtiff=internal --with-geotiff=internal --with-proj=${PROJ4_OUTPUT_PATH} --without-ld-shared --without-libtool --without-xml2 
         --without-jasper --without-odbc --without-libkml --without-hdf5 --without-curl --without-expat --without-pg --without-geos --without-sqlite3 --without-pcre 
         --without-webp --enable-static --disable-shared --prefix=<INSTALL_DIR> > ${NULL_DEVICE} && rm config.rpath)
    
    set(BUILD_CMD cd ${GDAL_SOURCE_DIR} && $(MAKE))
    set(INSTALL_CMD cd ${GDAL_SOURCE_DIR} && $(MAKE) install)

    FILE(GLOB PUBLIC_HEADERS
        "${GDAL_SOURCE_DIR}/frmts/gtiff/*.h"
        "${GDAL_SOURCE_DIR}/frmts/gtiff/libgeotiff/*.h"
        "${GDAL_SOURCE_DIR}/frmts/gtiff/libgeotiff/*.inc"
        "${GDAL_SOURCE_DIR}/frmts/gtiff/libtiff/*.h"
    )

    # Set library path
    set(GDAL_LIBRARY ${GDAL_LIBRARY_DIR}/libgdal.a)
endif ()

# Copy Public Header Files
FILE(COPY ${PUBLIC_HEADERS} DESTINATION ${GDAL_INCLUDE_DIR})

# Copy Data
FILE(GLOB DATA_FILES "${GDAL_SOURCE_DIR}/data/*")
FILE(COPY ${DATA_FILES} DESTINATION ${GDAL_DATA_DIR})

# Add external project to build GDAL
ExternalProject_Add(gdal
    SOURCE_DIR ${GDAL_SOURCE_DIR}
    PREFIX ${GDAL_OUTPUT_DIR}
    CONFIGURE_COMMAND ${CONFIG_CMD}
    BUILD_COMMAND ${BUILD_CMD}
    INSTALL_DIR ${GDAL_OUTPUT_DIR}
    INSTALL_COMMAND ${INSTALL_CMD})

# Set variables in parent scope
set(GDAL_FOUND ON PARENT_SCOPE)
set(GDAL_INCLUDE_DIR ${GDAL_INCLUDE_DIR} PARENT_SCOPE)
set(GDAL_INCLUDE_DIRS ${GDAL_INCLUDE_DIR} PARENT_SCOPE)
set(GDAL_LIBRARY ${GDAL_LIBRARY} PARENT_SCOPE)
set(GDAL_LIBRARIES ${GDAL_LIBRARY} PARENT_SCOPE)
SET(GDAL_LIBRARY_PATH ${GDAL_LIBRARY})
SET(GDAL_LIBRARY_PATH ${GDAL_LIBRARY} PARENT_SCOPE)
set(GDAL_DEPENDENCIES gdal)
set(GDAL_DEPENDENCIES ${GDAL_DEPENDENCIES} PARENT_SCOPE)

add_dependencies(${GDAL_DEPENDENCIES} ${PROJ4_DEPENDENCIES})

if (MSVC)
    add_custom_command(TARGET gdal POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${GDAL_SOURCE_DIR}/gcore/gdal_version.h" "${GDAL_INCLUDE_DIR}/gdal_version.h")
endif ()

# Add to ThirdParty folder
set_folder(gdal ThirdParty)

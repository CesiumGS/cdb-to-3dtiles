include(ExternalProject)

ExternalProject_Add(
	cesium-native
	SOURCE_DIR ${ROOT_THIRDPARTY_DIR}/cesium-native
	BINARY_DIR ${ROOT_THIRDPARTY_DIR}/build/cesium-native
	CMAKE_ARGS
		-DCMAKE_BUILD_TYPE=Release
	STEP_TARGETS build
	INSTALL_COMMAND ""
)

ExternalProject_Add(
	draco
	SOURCE_DIR ${ROOT_THIRDPARTY_DIR}/cesium-native/extern/draco
	BINARY_DIR ${ROOT_THIRDPARTY_DIR}/cesium-native/extern/build/draco
	CMAKE_ARGS
		-DCMAKE_BUILD_TYPE=Release
	STEP_TARGETS build
	INSTALL_COMMAND ""
)

ExternalProject_Add(
	uriparser
	SOURCE_DIR ${ROOT_THIRDPARTY_DIR}/cesium-native/extern/uriparser
	BINARY_DIR ${ROOT_THIRDPARTY_DIR}/cesium-native/extern/build/uriparser
	CMAKE_ARGS 
		-DCMAKE_BUILD_TYPE=Release
		-DURIPARSER_BUILD_TESTS:BOOL=OFF 
		-DURIPARSER_BUILD_DOCS:BOOL=OFF 
		-DBUILD_SHARED_LIBS:BOOL=OFF 
		-DURIPARSER_ENABLE_INSTALL:BOOL=OFF 
		-DURIPARSER_BUILD_TOOLS:BOOL=OFF
	STEP_TARGETS build
	INSTALL_COMMAND ""
)

ExternalProject_Add_StepDependencies(cesium-native build draco uriparser)
ExternalProject_Get_Property(cesium-native BINARY_DIR SOURCE_DIR)
set(cesium-native_INCLUDE_DIR
    "${SOURCE_DIR}/CesiumUtility/include"
    "${SOURCE_DIR}/CesiumGeospatial/include"
    "${SOURCE_DIR}/CesiumGeometry/include"
    "${SOURCE_DIR}/Cesium3DTiles/include"
    PARENT_SCOPE)
set(cesium-native_LIBRARIES
    "${BINARY_DIR}/CesiumGeospatial/${CMAKE_STATIC_LIBRARY_PREFIX}CesiumGeospatial${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${BINARY_DIR}/CesiumUtility/${CMAKE_STATIC_LIBRARY_PREFIX}CesiumUtility${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${BINARY_DIR}/CesiumGeometry/${CMAKE_STATIC_LIBRARY_PREFIX}CesiumGeometry${CMAKE_STATIC_LIBRARY_SUFFIX}"
    "${BINARY_DIR}/Cesium3DTiles/${CMAKE_STATIC_LIBRARY_PREFIX}Cesium3DTiles${CMAKE_STATIC_LIBRARY_SUFFIX}"
	PARENT_SCOPE)

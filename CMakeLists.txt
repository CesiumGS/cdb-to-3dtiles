cmake_minimum_required(VERSION 3.15)
project(
	CDBTo3DTiles
	VERSION 0.1.0
	LANGUAGES C CXX
)
enable_testing()

add_subdirectory(extern)

# main target built
add_subdirectory(CDBTo3DTiles)
add_subdirectory(CLI)

# typically needed if we are at the top level project
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	add_subdirectory(Tests)
endif()

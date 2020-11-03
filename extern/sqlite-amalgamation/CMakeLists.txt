project(sqlite3)

set(_definitions SQLITE_OMIT_LOAD_EXTENSION)
set(_libraries sqlite3 ${GLOBAL_LINK_LIBRARIES})

add_library(sqlite3 STATIC sqlite3.c sqlite3.h sqlite3ext.h)
set_target_properties(sqlite3 PROPERTIES COMPILE_DEFINITIONS ${_definitions})
set_folder(sqlite3 ThirdParty)

add_executable(sqlite3_shell shell.c sqlite3.h sqlite3ext.h)
set_target_properties(sqlite3_shell PROPERTIES COMPILE_DEFINITIONS ${_definitions})
add_dependencies(sqlite3_shell sqlite3)
target_link_libraries(sqlite3_shell ${_libraries})

install(TARGETS sqlite3
        EXPORT sqlite3Config
        INCLUDES DESTINATION ${SQLITE3_INCLUDE_DIR}
        ARCHIVE DESTINATION ${BUILD_LIB_DIR})
install(EXPORT sqlite3Config
        DESTINATION ${SQLITE3_SUBDIRECTORY}/cmake)

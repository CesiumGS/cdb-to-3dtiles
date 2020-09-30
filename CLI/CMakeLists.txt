project(CLI)

add_executable(CDBConverter main.cpp)
target_include_directories(CDBConverter PRIVATE ${GLM_INCLUDE_DIR} ${cesium-native_INCLUDE_DIR})
target_link_libraries(CDBConverter PRIVATE ${cesium-native_LIBRARIES})

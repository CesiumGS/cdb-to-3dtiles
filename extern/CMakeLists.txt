project(ThirdParty)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_LIST_DIR})

set(ROOT_THIRDPARTY_DIR ${CMAKE_CURRENT_LIST_DIR})

# add cesium-native
include(add_cesium-native)

# add glm
set(GLM_INCLUDE_DIR "${ROOT_THIRDPARTY_DIR}/glm" PARENT_SCOPE)

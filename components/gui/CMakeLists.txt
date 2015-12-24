set(CMAKE_CXX_FLAGS "${BRAHMS_HOST_DEFINITION} ${BRAHMS_WX_CXXFLAGS}")
set(CMAKE_C_FLAGS "${BRAHMS_HOST_DEFINITION} ${BRAHMS_WX_CFLAGS}")
add_library(image_numeric SHARED image.cpp)
set_target_properties(image_numeric PROPERTIES OUTPUT_NAME "component" PREFIX "")

set(COMP_PATH ${VAR_SYSTEMML_PATH}/Namespace/dev/std/image/numeric)

install(TARGETS image_numeric DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml DESTINATION ${COMP_PATH})

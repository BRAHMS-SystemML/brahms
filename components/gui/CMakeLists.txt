set(CMAKE_CXX_FLAGS "${BRAHMS_HOST_DEFINITION} ${BRAHMS_WX_CXXFLAGS}")
set(CMAKE_C_FLAGS "${BRAHMS_HOST_DEFINITION} ${BRAHMS_WX_CFLAGS}")
add_library(image_numeric SHARED image.cpp)
set_target_properties(image_numeric PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS image_numeric DESTINATION var/SystemML/Namespace/dev/std/image/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/dev/std/image/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/dev/std/image/numeric)

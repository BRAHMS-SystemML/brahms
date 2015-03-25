add_library(math_esum SHARED math_esum.cpp)
set_target_properties(math_esum PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(math_esum brahms-engine brahms-engine-base)
endif(APPLE)

install(TARGETS math_esum DESTINATION var/SystemML/Namespace/std/2009/math/esum/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/math/esum/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/math/esum)

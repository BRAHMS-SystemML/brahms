add_library(source_numeric SHARED source_numeric.cpp)
set_target_properties(source_numeric PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(source_numeric brahms-engine brahms-engine-base)
endif(APPLE)

install(TARGETS source_numeric DESTINATION var/SystemML/Namespace/std/2009/source/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/source/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/source/numeric)

add_library(index_numeric SHARED index.cpp)
set_target_properties(index_numeric PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS index_numeric DESTINATION var/SystemML/Namespace/std/2009/index/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/index/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/index/numeric)

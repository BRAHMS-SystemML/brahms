add_library(cat_numeric SHARED cat.cpp)
set_target_properties(cat_numeric PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS cat_numeric DESTINATION var/SystemML/Namespace/std/2009/cat/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/cat/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/cat/numeric)

add_library(random_numeric SHARED random_numeric.cpp)
set_target_properties(random_numeric PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS random_numeric DESTINATION var/SystemML/Namespace/std/2009/random/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/random/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/random/numeric)

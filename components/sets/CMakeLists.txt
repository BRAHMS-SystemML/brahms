add_library(test_sets SHARED sets.cpp)
set_target_properties(test_sets PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS test_sets DESTINATION var/SystemML/Namespace/client/brahms/test/sets/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/client/brahms/test/sets/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/client/brahms/test/sets)

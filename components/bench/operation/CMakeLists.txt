add_library(bench_operation SHARED operation.cpp)
set_target_properties(bench_operation PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS bench_operation DESTINATION var/SystemML/Namespace/std/2009/bench/operation/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/bench/operation/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/bench/operation)

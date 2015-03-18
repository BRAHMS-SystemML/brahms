add_library(bench_overhead SHARED overhead.cpp)
set_target_properties(bench_overhead PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS bench_overhead DESTINATION var/SystemML/Namespace/client/brahms/bench/overhead/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/client/brahms/bench/overhead/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/client/brahms/bench/overhead)

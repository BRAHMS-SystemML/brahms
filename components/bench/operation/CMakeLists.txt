add_library(bench_operation SHARED operation.cpp)
set_target_properties(bench_operation PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(bench_operation brahms-engine brahms-engine-base)
endif(APPLE)

install(TARGETS bench_operation DESTINATION var/SystemML/Namespace/client/brahms/bench/operation/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/client/brahms/bench/operation/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/client/brahms/bench/operation)

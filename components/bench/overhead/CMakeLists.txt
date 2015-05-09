add_library(bench_overhead SHARED overhead.cpp)
set_target_properties(bench_overhead PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(bench_overhead brahms-engine brahms-engine-base)
endif(APPLE)

install(TARGETS bench_overhead DESTINATION ${BENCH_COMP_PATH}/overhead/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION ${BENCH_COMP_PATH}/overhead/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${BENCH_COMP_PATH}/overhead)

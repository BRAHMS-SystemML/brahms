add_library(bench_operation SHARED operation.cpp)
set_target_properties(bench_operation PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(bench_operation brahms-engine brahms-engine-base)
endif(APPLE)

install(TARGETS bench_operation DESTINATION ${BENCH_COMP_PATH}/operation/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION ${BENCH_COMP_PATH}/operation/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${BENCH_COMP_PATH}/operation)

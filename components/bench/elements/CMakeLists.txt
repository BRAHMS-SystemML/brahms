add_library(bench_elements SHARED elements.cpp scalable.cpp elements_monolithic.cpp)
set_target_properties(bench_elements PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(bench_elements brahms-engine brahms-engine-base)
endif(APPLE)

install(TARGETS bench_elements DESTINATION ${BENCH_COMP_PATH}/elements/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION ${BENCH_COMP_PATH}/elements/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${BENCH_COMP_PATH}/elements)

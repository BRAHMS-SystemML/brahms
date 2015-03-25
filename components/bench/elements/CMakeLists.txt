add_library(bench_elements SHARED elements.cpp scalable.cpp elements_monolithic.cpp)
set_target_properties(bench_elements PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(bench_elements brahms-engine brahms-engine-base)
endif(APPLE)

install(TARGETS bench_elements DESTINATION var/SystemML/Namespace/client/brahms/bench/elements/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/client/brahms/bench/elements/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/client/brahms/bench/elements)

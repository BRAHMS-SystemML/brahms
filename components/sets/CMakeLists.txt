add_library(test_sets SHARED sets.cpp)
set_target_properties(test_sets PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(test_sets brahms-engine brahms-engine-base)
endif(APPLE)

set(COMP_PATH ${VAR_SYSTEMML_PATH}/Namespace/client/brahms/test/sets)

install(TARGETS test_sets DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml DESTINATION ${COMP_PATH})

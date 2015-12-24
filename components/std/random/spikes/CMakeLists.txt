add_library(random_spikes SHARED random_spikes.cpp)
set_target_properties(random_spikes PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(random_spikes brahms-engine brahms-engine-base)
endif(APPLE)

set(COMP_PATH ${VAR_SYSTEMML_PATH}/Namespace/std/2009/random/spikes)

install(TARGETS random_spikes DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml DESTINATION ${COMP_PATH})

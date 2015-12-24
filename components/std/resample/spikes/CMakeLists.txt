add_library(resample_spikes SHARED resample_spikes.cpp)
set_target_properties(resample_spikes PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(resample_spikes brahms-engine brahms-engine-base)
endif(APPLE)

set(COMP_PATH ${VAR_SYSTEMML_PATH}/Namespace/std/2009/resample/spikes)

install(TARGETS resample_spikes DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml DESTINATION ${COMP_PATH})

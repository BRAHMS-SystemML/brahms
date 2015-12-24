add_library(util_rng SHARED util_rng.cpp)
set_target_properties(util_rng PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(util_rng brahms-engine brahms-engine-base)
endif(APPLE)

set(COMP_PATH ${VAR_SYSTEMML_PATH}/Namespace/std/2009/util/rng)

install(TARGETS util_rng DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/utility/node.xml DESTINATION ${COMP_PATH})

# We need util_rng.h for legacy (see brahms-c++-common.h around line 3365)
install(FILES util_rng.h DESTINATION ${INCLUDE_INSTALL_PATH})

add_library(math_eproduct SHARED math_eproduct.cpp)
set_target_properties(math_eproduct PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(math_eproduct brahms-engine brahms-engine-base)
endif(APPLE)

set(COMP_PATH ${VAR_SYSTEMML_PATH}/Namespace/std/2009/math/eproduct)

install(TARGETS math_eproduct DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml DESTINATION ${COMP_PATH})

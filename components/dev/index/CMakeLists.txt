add_library(index_numeric SHARED index.cpp)
set_target_properties(index_numeric PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(index_numeric brahms-engine brahms-engine-base)
endif(APPLE)

install(TARGETS index_numeric DESTINATION ${DEV_COMP_PATH}/index/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION ${DEV_COMP_PATH}/index/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${DEV_COMP_PATH}/index/numeric)

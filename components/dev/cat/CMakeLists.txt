add_library(cat_numeric SHARED cat.cpp)
set_target_properties(cat_numeric PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(cat_numeric brahms-engine brahms-engine-base)
endif(APPLE)

install(TARGETS cat_numeric DESTINATION ${DEV_COMP_PATH}/cat/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION ${DEV_COMP_PATH}/cat/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${DEV_COMP_PATH}/cat/numeric)

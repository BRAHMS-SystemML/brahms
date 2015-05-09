add_library(data_numeric SHARED data_numeric.cpp)
set_target_properties(data_numeric PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(data_numeric brahms-engine brahms-engine-base)
endif(APPLE)

if(STANDALONE_INSTALL)
  set(COMP_PATH SystemML/Namespace/std/2009/data/numeric)
else()
  set(COMP_PATH var/SystemML/Namespace/std/2009/data/numeric)
endif()

install(TARGETS data_numeric DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml DESTINATION ${COMP_PATH}/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/data/node.xml DESTINATION ${COMP_PATH})

# We need the data_numeric.h file installed for legacy support
install(FILES data_numeric.h DESTINATION ${INCLUDE_INSTALL_PATH})

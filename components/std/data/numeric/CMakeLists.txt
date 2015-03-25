add_library(data_numeric SHARED data_numeric.cpp)
set_target_properties(data_numeric PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(data_numeric brahms-engine brahms-engine-base)
endif(APPLE)

install(TARGETS data_numeric DESTINATION var/SystemML/Namespace/std/2009/data/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/data/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/data/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/data/numeric)

# We need the data_numeric.h file installed for legacy support
install(FILES data_numeric.h DESTINATION include)

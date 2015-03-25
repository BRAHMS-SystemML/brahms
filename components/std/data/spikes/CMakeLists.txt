add_library(data_spikes SHARED data_spikes.cpp)
set_target_properties(data_spikes PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(data_spikes brahms-engine brahms-engine-base)
endif(APPLE)

install(TARGETS data_spikes DESTINATION var/SystemML/Namespace/std/2009/data/spikes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/data/spikes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/data/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/data/spikes)

# For legacy interface support need this installed in include/
install(FILES data_spikes.h DESTINATION include)

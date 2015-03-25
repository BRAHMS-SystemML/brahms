add_library(random_spikes SHARED random_spikes.cpp)
set_target_properties(random_spikes PROPERTIES OUTPUT_NAME "component" PREFIX "")
if(APPLE)
target_link_libraries(random_spikes brahms-engine brahms-engine-base)
endif(APPLE)

install(TARGETS random_spikes DESTINATION var/SystemML/Namespace/std/2009/random/spikes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/random/spikes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/random/spikes)

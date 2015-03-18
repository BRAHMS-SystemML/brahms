add_library(random_spikes SHARED random_spikes.cpp)
set_target_properties(random_spikes PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS random_spikes DESTINATION var/SystemML/Namespace/std/2009/random/spikes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/random/spikes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/data/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/random/spikes)

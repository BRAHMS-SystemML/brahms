add_library(source_spikes SHARED source_spikes.cpp)
set_target_properties(source_spikes PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS source_spikes DESTINATION var/SystemML/Namespace/std/2009/source/spikes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/source/spikes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/source/spikes)

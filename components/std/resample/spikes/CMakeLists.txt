add_library(resample_spikes SHARED resample_spikes.cpp)
set_target_properties(resample_spikes PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS resample_spikes DESTINATION var/SystemML/Namespace/std/2009/resample/spikes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/resample/spikes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/resample/spikes)

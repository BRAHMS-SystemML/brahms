add_library(resample_numeric SHARED resample_numeric.cpp)
set_target_properties(resample_numeric PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS resample_numeric DESTINATION var/SystemML/Namespace/std/2009/resample/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/resample/numeric/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/data/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/resample/numeric)

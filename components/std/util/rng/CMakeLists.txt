add_library(util_rng SHARED util_rng.cpp)
set_target_properties(util_rng PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS util_rng DESTINATION var/SystemML/Namespace/std/2009/util/rng/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/util/rng/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/utility/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/util/rng)

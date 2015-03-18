add_library(math_eproduct SHARED math_eproduct.cpp)
set_target_properties(math_eproduct PROPERTIES OUTPUT_NAME "component" PREFIX "")
install(TARGETS math_eproduct DESTINATION var/SystemML/Namespace/std/2009/math/eproduct/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1199/release.xml
  DESTINATION var/SystemML/Namespace/std/2009/math/eproduct/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/data/node.xml
  DESTINATION var/SystemML/Namespace/std/2009/math/eproduct)

set(EXAMPLE_COMP_PATH ${VAR_SYSTEMML_PATH}/Namespace/client/brahms/example)

# Python examples go in Namespace/client/brahms/example/1262
install(FILES head.py DESTINATION ${EXAMPLE_COMP_PATH}/1262/head/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1262/release.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1262/head/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1262/head)

install(FILES brain.py DESTINATION ${EXAMPLE_COMP_PATH}/1262/brain/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1262/release.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1262/brain/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1262/brain)

install(FILES foxes.py DESTINATION ${EXAMPLE_COMP_PATH}/1262/foxes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1262/release.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1262/foxes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1262/foxes)

install(FILES rabbits.py DESTINATION ${EXAMPLE_COMP_PATH}/1262/rabbits/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1262/release.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1262/rabbits/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1262/rabbits)

# Matlab examples go in Namespace/client/brahms/example/1258
install(FILES gui.m DESTINATION ${EXAMPLE_COMP_PATH}/1258/gui/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1258/release.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1258/gui/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1258/gui)

install(FILES head.m DESTINATION ${EXAMPLE_COMP_PATH}/1258/head/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1258/release.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1258/head/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1258/head)

install(FILES brain.m DESTINATION ${EXAMPLE_COMP_PATH}/1258/brain/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1258/release.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1258/brain/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1258/brain)

install(FILES foxes.m DESTINATION ${EXAMPLE_COMP_PATH}/1258/foxes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1258/release.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1258/foxes/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1258/foxes)

install(FILES rabbits.m DESTINATION ${EXAMPLE_COMP_PATH}/1258/rabbits/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/1258/release.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1258/rabbits/brahms/0)
install(FILES ${CMAKE_SOURCE_DIR}/shared/process/node.xml
  DESTINATION ${EXAMPLE_COMP_PATH}/1258/rabbits)

# Add __SOCKET__ to compile brahms-channel-sockets:
set(CMAKE_CXX_FLAGS "${BRAHMS_HOST_DEFINITION} -D__SOCKETS__")
add_library(brahms-channel-sockets SHARED
  ../channel.cpp ../deliverer.cpp
  sockets.cpp sockets-support.cpp sockets-receiver.cpp sockets-sender.cpp
  )
if(APPLE)
  target_link_libraries(brahms-channel-sockets brahms-engine-base)
endif(APPLE)
if(STANDALONE_INSTALL)
  install(TARGETS brahms-channel-sockets DESTINATION BRAHMS/bin)
else()
  install(TARGETS brahms-channel-sockets DESTINATION lib)
endif()

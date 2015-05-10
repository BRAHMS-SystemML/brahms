add_library(brahms-engine-base ${BRAHMS_ENGINE_LINK_TYPE}
  ../channel/channel-common.cpp
  base/core.cpp base/ipm.cpp base/os.cpp base/text.cpp base/constants.cpp
  base/brahms_error.cpp base/brahms_math.cpp base/output.cpp base/thread.cpp
)
add_library(brahms-engine ${BRAHMS_ENGINE_LINK_TYPE}
  systemml/component.cpp systemml/event.cpp systemml/interface.cpp
  systemml/loggable.cpp systemml/process.cpp systemml/ systemml/system.cpp
  systemml/thread.cpp systemml/data.cpp systemml/identifier.cpp
  systemml/link.cpp systemml/port.cpp systemml/set.cpp
  systemml/utility.cpp
  support/environment.cpp support/execution.cpp support/loader.cpp
  support/os.cpp support/error.cpp
  support/helpers.cpp support/module.cpp support/register.cpp
  support/xml.cpp
  main/api.cpp main/api-engine.cpp main/engine.cpp
  main/engine-open.cpp  main/engine-execute.cpp  main/engine-close.cpp
  main/engine-walk.cpp main/engine-monitor.cpp 
  )

# Mac wants to be able to see libbrahms-channel-common in the above.
if(APPLE)
  target_link_libraries(
    brahms-engine brahms-engine-base brahms-gui
    ${CMAKE_THREAD_LIBS_INIT} ${LIB_RT} ${XAW_LDFLAGS}
    )
endif(APPLE)

if(NOT STANDALONE_INSTALL)
  install(TARGETS brahms-engine brahms-engine-base DESTINATION ${LIB_INSTALL_PATH})
endif()

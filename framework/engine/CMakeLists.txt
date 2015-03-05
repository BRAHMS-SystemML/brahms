add_library(brahms-engine SHARED
  base/core.cpp base/ipm.cpp base/os.cpp base/text.cpp base/constants.cpp
  base/brahms_error.cpp base/brahms_math.cpp base/output.cpp base/thread.cpp
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
install(TARGETS brahms-engine DESTINATION lib)

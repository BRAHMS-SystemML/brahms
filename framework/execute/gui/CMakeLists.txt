add_library(brahms-gui ${BRAHMS_ENGINE_LINK_TYPE} gui.cpp Gauge.c)

if(APPLE)
  if(HAVE_CLOCK_GETTIME_IN_RT)
    set(LIB_RT rt)
  endif(HAVE_CLOCK_GETTIME_IN_RT)
  target_link_libraries(
    brahms-gui ${CMAKE_THREAD_LIBS_INIT} ${LIB_RT} ${XAW_LDFLAGS} ${XMU_LDFLAG}
    )
endif(APPLE)

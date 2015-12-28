set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${XAW_CFLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${XAW_CFLAGS}")
message(STATUS "Flags for brahms-execute: " ${CMAKE_CXX_FLAGS})
add_executable(brahms-execute main.cpp info.cpp os.cpp tfs.cpp version.cpp)

# Some Linux systems have clock_gettime in librt, for others it's in libc
if(HAVE_CLOCK_GETTIME_IN_RT)
  set(LIB_RT rt)
endif(HAVE_CLOCK_GETTIME_IN_RT)

if(APPLE)
  target_link_libraries(brahms-execute
    brahms-engine brahms-engine-base brahms-gui
    ${CMAKE_THREAD_LIBS_INIT} ${LIB_RT} ${XAW_LDFLAGS} ${XMU_LDFLAG} ${CMAKE_DL_LIBS}
    )
else()
  target_link_libraries(brahms-execute
    ${BRAHMS_LINK_WHOLE_ARCHIVE} # adds -Wl,--whole-archive for static linking
    brahms-engine brahms-engine-base brahms-gui
    ${BRAHMS_NOT_LINK_WHOLE_ARCHIVE} # resets with -Wl,--no-whole-archive
    ${CMAKE_THREAD_LIBS_INIT} ${LIB_RT} ${XAW_LDFLAGS} ${XMU_LDFLAG} ${CMAKE_DL_LIBS}
    )
endif(APPLE)

install(TARGETS brahms-execute DESTINATION ${BIN_INSTALL_PATH})
install(PROGRAMS brahms DESTINATION ${BIN_INSTALL_PATH})

# The brahms-gui library lives in a subdirectory:
add_subdirectory(gui)

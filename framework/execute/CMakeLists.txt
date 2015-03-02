add_executable(brahms-execute main.cpp info.cpp os.cpp tfs.cpp version.cpp)
target_link_libraries(
  brahms-execute brahms-engine brahms-channel-common
  brahms-gui pthread Xt Xaw
  )
# Compile the gui library:
add_subdirectory(gui)

add_executable(brahms-execute main.cpp info.cpp os.cpp tfs.cpp version.cpp)
# Xaw pulls in the rest of the X11 libs required here. However, the
# full list of linked 3rd party libs on my Ubuntu system includes:
#        libpthread.so.0 => /lib/x86_64-linux-gnu/libpthread.so.0 (0x00007f76c8d7a000)
#        libXaw.so.7 => /usr/lib/x86_64-linux-gnu/libXaw.so.7 (0x00007f76c8b08000)
#        libstdc++.so.6 => /usr/lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f76c8804000)
#        libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f76c85ee000)
#        libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f76c8228000)
#        libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f76c7f22000)
#        /lib64/ld-linux-x86-64.so.2 (0x00007f76c973e000)
#        libXext.so.6 => /usr/lib/x86_64-linux-gnu/libXext.so.6 (0x00007f76c7d10000)
#        libXmu.so.6 => /usr/lib/x86_64-linux-gnu/libXmu.so.6 (0x00007f76c7af6000)
#        libXt.so.6 => /usr/lib/x86_64-linux-gnu/libXt.so.6 (0x00007f76c7890000)
#        libXpm.so.4 => /usr/lib/x86_64-linux-gnu/libXpm.so.4 (0x00007f76c767e000)
#        libX11.so.6 => /usr/lib/x86_64-linux-gnu/libX11.so.6 (0x00007f76c7348000)
#        libSM.so.6 => /usr/lib/x86_64-linux-gnu/libSM.so.6 (0x00007f76c7140000)
#        libICE.so.6 => /usr/lib/x86_64-linux-gnu/libICE.so.6 (0x00007f76c6f23000)
#        libxcb.so.1 => /usr/lib/x86_64-linux-gnu/libxcb.so.1 (0x00007f76c6d04000)
#        libdl.so.2 => /lib/x86_64-linux-gnu/libdl.so.2 (0x00007f76c6b00000)
#        libuuid.so.1 => /lib/x86_64-linux-gnu/libuuid.so.1 (0x00007f76c68fa000)
#        libXau.so.6 => /usr/lib/x86_64-linux-gnu/libXau.so.6 (0x00007f76c66f6000)
#        libXdmcp.so.6 => /usr/lib/x86_64-linux-gnu/libXdmcp.so.6 (0x00007f76c64f0000)
target_link_libraries(
  brahms-execute brahms-engine brahms-channel-common
  brahms-gui pthread Xaw
  )
install(TARGETS brahms-execute DESTINATION bin)
install(PROGRAMS brahms DESTINATION bin)
# Compile the gui library:
add_subdirectory(gui)

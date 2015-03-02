# Because the MPI code needs to be compiled with mpicxx, we need
# another CMakeLists.txt file to compile libbrahms-channel-mpich2.

# This actually wants to be compiled with mpicxx?
#set(CMAKE_CXX_FLAGS "${BRAHMS_HOST_DEFINITION} -D__MPI__")
#add_library(brahms-channel-mpich2 SHARED
#  channel.cpp deliverer.cpp mpi.cpp
#  mpi-receiver.cpp mpi-sender.cpp
#  )

# Add __SOCKET__ to compile brahms-channel-sockets:
set(CMAKE_CXX_FLAGS "${BRAHMS_HOST_DEFINITION} -D__SOCKETS__")
add_library(brahms-channel-sockets SHARED
  channel.cpp deliverer.cpp sockets.cpp
  sockets-support.cpp sockets-receiver.cpp sockets-sender.cpp
  )

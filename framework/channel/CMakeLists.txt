# This actually wants to be compiled with mpicxx?
#add_definitions (-D__MPI__)
#add_library(brahms-channel-mpich2 SHARED
#  channel.cpp deliverer.cpp fifo.cpp
#  mpi.cpp mpi-receiver.cpp  mpi-sender.cpp mpi-support.cpp
#  )

# Add __SOCKET__ to compile brahms-channel-sockets:
set(CMAKE_CXX_FLAGS "-D${BRAHMS_HOST_DEFINITION} -D__SOCKETS__")
add_library(brahms-channel-sockets SHARED
  channel.cpp deliverer.cpp fifo.cpp
  sockets.cpp sockets-receiver.cpp sockets-sender.cpp sockets-support.cpp
  )

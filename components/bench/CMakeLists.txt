if(STANDALONE_INSTALL)
  set(BENCH_COMP_PATH SystemML/Namespace/client/brahms/bench)
else()
  set(BENCH_COMP_PATH var/SystemML/Namespace/client/brahms/bench)
endif()

add_subdirectory(elements)
add_subdirectory(operation)
add_subdirectory(overhead)

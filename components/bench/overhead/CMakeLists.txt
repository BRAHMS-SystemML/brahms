add_library(bench_overhead SHARED overhead.cpp)
set_target_properties(bench_overhead PROPERTIES OUTPUT_NAME "component")
set_target_properties(bench_overhead PROPERTIES PREFIX "")
set_target_properties(bench_overhead PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

add_library(bench_overhead SHARED overhead.cpp)
set_target_properties(bench_overhead PROPERTIES OUTPUT_NAME "component" PREFIX "")

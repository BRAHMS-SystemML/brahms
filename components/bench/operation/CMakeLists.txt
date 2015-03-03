add_library(bench_operation SHARED operation.cpp)
set_target_properties(bench_operation PROPERTIES OUTPUT_NAME "component" PREFIX "")

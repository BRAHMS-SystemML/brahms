add_library(bench_operation SHARED operation.cpp)
set_target_properties(bench_operation PROPERTIES OUTPUT_NAME "component")
set_target_properties(bench_operation PROPERTIES PREFIX "")
set_target_properties(bench_operation PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

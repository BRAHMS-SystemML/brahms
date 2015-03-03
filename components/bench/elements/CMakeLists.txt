add_library(bench_elements SHARED elements.cpp scalable.cpp elements_monolithic.cpp)
set_target_properties(bench_elements PROPERTIES OUTPUT_NAME "component" PREFIX "")

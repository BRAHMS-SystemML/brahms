add_library(util_rng SHARED util_rng.cpp)
set_target_properties(util_rng PROPERTIES OUTPUT_NAME "component")
set_target_properties(util_rng PROPERTIES PREFIX "")
set_target_properties(util_rng PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

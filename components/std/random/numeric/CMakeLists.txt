add_library(random_numeric SHARED random_numeric.cpp)
set_target_properties(random_numeric PROPERTIES OUTPUT_NAME "component")
set_target_properties(random_numeric PROPERTIES PREFIX "")
set_target_properties(random_numeric PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

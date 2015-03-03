add_library(data_numeric SHARED data_numeric.cpp)
set_target_properties(data_numeric PROPERTIES OUTPUT_NAME "component")
set_target_properties(data_numeric PROPERTIES PREFIX "")
set_target_properties(data_numeric PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

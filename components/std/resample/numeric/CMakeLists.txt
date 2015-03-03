add_library(resample_numeric SHARED resample_numeric.cpp)
set_target_properties(resample_numeric PROPERTIES OUTPUT_NAME "component")
set_target_properties(resample_numeric PROPERTIES PREFIX "")
set_target_properties(resample_numeric PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

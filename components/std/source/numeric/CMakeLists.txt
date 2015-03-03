add_library(source_numeric SHARED source_numeric.cpp)
set_target_properties(source_numeric PROPERTIES OUTPUT_NAME "component")
set_target_properties(source_numeric PROPERTIES PREFIX "")
set_target_properties(source_numeric PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

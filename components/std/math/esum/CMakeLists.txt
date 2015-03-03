add_library(math_esum SHARED math_esum.cpp)
set_target_properties(math_esum PROPERTIES OUTPUT_NAME "component")
set_target_properties(math_esum PROPERTIES PREFIX "")
set_target_properties(math_esum PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

add_library(math_eproduct SHARED math_eproduct.cpp)
set_target_properties(math_eproduct PROPERTIES OUTPUT_NAME "component")
set_target_properties(math_eproduct PROPERTIES PREFIX "")
set_target_properties(math_eproduct PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

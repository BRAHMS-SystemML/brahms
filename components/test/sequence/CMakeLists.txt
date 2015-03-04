add_library(sequence SHARED sequence.cpp)
set_target_properties(sequence PROPERTIES OUTPUT_NAME "component" PREFIX "")

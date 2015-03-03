add_library(cat SHARED cat.cpp)
set_target_properties(cat PROPERTIES OUTPUT_NAME "component" PREFIX "")

add_library(abort SHARED abort.cpp)
set_target_properties(abort PROPERTIES OUTPUT_NAME "component" PREFIX "")

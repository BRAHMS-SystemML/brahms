add_library(modlin_leaky SHARED modlin_activation_leaky.cpp)
set_target_properties(modlin_leaky PROPERTIES OUTPUT_NAME "component" PREFIX "")
# No install; this component is "only for testing" (according to the old makefile)

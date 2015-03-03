add_library(resample_spikes SHARED resample_spikes.cpp)
set_target_properties(resample_spikes PROPERTIES OUTPUT_NAME "component" PREFIX "")

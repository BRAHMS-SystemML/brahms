add_library(resample_spikes SHARED resample_spikes.cpp)
set_target_properties(resample_spikes PROPERTIES OUTPUT_NAME "component")
set_target_properties(resample_spikes PROPERTIES PREFIX "")
set_target_properties(resample_spikes PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

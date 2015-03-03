add_library(data_spikes SHARED data_spikes.cpp)
set_target_properties(data_spikes PROPERTIES OUTPUT_NAME "component")
set_target_properties(data_spikes PROPERTIES PREFIX "")
set_target_properties(data_spikes PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

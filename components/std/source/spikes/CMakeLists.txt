add_library(source_spikes SHARED source_spikes.cpp)
set_target_properties(source_spikes PROPERTIES OUTPUT_NAME "component")
set_target_properties(source_spikes PROPERTIES PREFIX "")
set_target_properties(source_spikes PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

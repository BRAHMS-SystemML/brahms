add_library(random_spikes SHARED random_spikes.cpp)
set_target_properties(random_spikes PROPERTIES OUTPUT_NAME "component")
set_target_properties(random_spikes PROPERTIES PREFIX "")
set_target_properties(random_spikes PROPERTIES COMPILE_FLAGS "-fPIC -Werror -pthread -O3")

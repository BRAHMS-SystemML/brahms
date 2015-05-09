add_subdirectory(1258) # Matlab binding
add_subdirectory(1262) # Python binding
# I'm currently ignoring the "2310" binding - that's octave.
# The 995 binding is fully matlab, so ignoring that too, though
# it will need some installation code.
install(DIRECTORY 995 DESTINATION ${SHARE_BRAHMS_INSTALL_PATH}/bindings)

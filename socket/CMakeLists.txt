# keep all cpp files in varibale ${socket_src}
aux_source_directory(./src socket_src)

add_library(muxsocket STATIC ${socket_src})

add_dependencies(muxsocket mbase)
target_link_libraries(muxsocket mbase pthread)

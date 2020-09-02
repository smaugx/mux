# keep all cpp files in varibale ${message_handler_src}
aux_source_directory(./src transport_src)

add_library(transport STATIC ${transport_src})

add_dependencies(transport epoll mbase)
target_link_libraries(transport  epoll mbase)

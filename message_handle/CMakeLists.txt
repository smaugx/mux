# keep all cpp files in varibale ${message_handler_src}
aux_source_directory(./src message_handle_src)

add_library(msghandler STATIC ${message_handle_src})

add_dependencies(msghandler mbase)
target_link_libraries(msghandler mbase)

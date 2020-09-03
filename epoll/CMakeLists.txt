# keep all cpp files in varibale ${epoll_src}
aux_source_directory(./src epoll_src)

add_library(epoll STATIC ${epoll_src})

add_dependencies(epoll mbase )
target_link_libraries(epoll mbase pthread)

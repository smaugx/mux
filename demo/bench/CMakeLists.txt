# build target echo_server
add_executable(bench_server server.cc bench_socket.cc bench_tcp_acceptor.cc)
add_dependencies(bench_server msghandler mbase epoll)
target_link_libraries(bench_server epoll msghandler mbase)


# build target echo_client
add_executable(bench_client client.cc bench_client.cc bench_socket.cc)
add_dependencies(bench_client epoll msghandler mbase)
target_link_libraries(bench_client epoll msghandler mbase)

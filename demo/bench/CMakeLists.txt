# build target echo_server
add_executable(bench_server server.cc bench_socket.cc bench_tcp_acceptor.cc)
add_dependencies(bench_server muxsocket msghandler mbase)
target_link_libraries(bench_server muxsocket msghandler mbase)


# build target echo_client
add_executable(bench_client client.cc bench_client.cc bench_socket.cc)
add_dependencies(bench_client muxsocket msghandler mbase)
target_link_libraries(bench_client muxsocket msghandler mbase)

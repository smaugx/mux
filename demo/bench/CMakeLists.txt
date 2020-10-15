# build target echo_server
add_executable(bench_server server.cc bench_socket.cc bench_tcp_acceptor.cc)
add_dependencies(bench_server muxsocket msghandler mbase)
target_link_libraries(bench_server muxsocket msghandler mbase ${LIB_PROTOBUF})


# build target echo_client
add_executable(bench_client client.cc bench_client.cc bench_socket.cc)
add_dependencies(bench_client muxsocket msghandler mbase)
target_link_libraries(bench_client muxsocket msghandler mbase ${LIB_PROTOBUF})

# build target bench_client_accept
add_executable(bench_client_accept client2.cc bench_client.cc bench_socket.cc)
add_dependencies(bench_client_accept muxsocket msghandler mbase)
target_link_libraries(bench_client_accept muxsocket msghandler mbase ${LIB_PROTOBUF})

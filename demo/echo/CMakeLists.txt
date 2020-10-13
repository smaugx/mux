# build target echo_server
add_executable(echo_server server.cc echo_socket.cc echo_tcp_acceptor.cc)
add_dependencies(echo_server muxsocket msghandler mbase)
target_link_libraries(echo_server muxsocket msghandler mbase ${LIB_PROTOBUF})


# build target echo_client
add_executable(echo_client client.cc echo_client.cc echo_socket.cc)
add_dependencies(echo_client muxsocket msghandler mbase)
target_link_libraries(echo_client muxsocket msghandler mbase ${LIB_PROTOBUF})

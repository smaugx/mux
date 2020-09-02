# build target echo_server
add_executable(echo_server echo_server.cc)
add_dependencies(echo_server transport msghandler mbase)
target_link_libraries(echo_server transport msghandler mbase)


# build target echo_client
add_executable(echo_client client.cc)
add_dependencies(echo_client transport msghandler mbase)
target_link_libraries(echo_client transport msghandler mbase)

# keep all cpp files in varibale ${message_handler_src}

# build target bench_server
add_executable(bench_server bench_server.cc)
add_dependencies(bench_server transport msghandler mbase)
target_link_libraries(bench_server transport msghandler mbase )


# build target bench_client
add_executable(bench_client client.cc)
add_dependencies(bench_client transport msghandler mbase)
target_link_libraries(bench_client transport msghandler mbase )

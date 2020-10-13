# keep all cpp files in variable ${mbase_src}
aux_source_directory(./src mbase_src)
aux_source_directory(./protobuf mbase_protobuf_src)

add_library(mbase ${mbase_src} ${mbase_protobuf_src})

target_link_libraries(mbase ${LIB_PROTOBUF})

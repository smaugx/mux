# keep all cpp files in variable ${mbase_src}
aux_source_directory(./src mbase_src)

add_library(mbase ${mbase_src})

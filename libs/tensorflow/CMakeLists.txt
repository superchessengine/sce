cmake_minimum_required(VERSION 3.12)
message("--linking tensorflow")

add_library(TENSORFLOW_LIB lib/tensorflow.dll)
set_target_properties(TENSORFLOW_LIB PROPERTIES LINKER_LANGUAGE C)

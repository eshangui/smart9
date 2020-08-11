
cmake_minimum_required(VERSION 3.13)

project (d9main)
#set(CMAKE_BUILD_TYPE "Release")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -g")

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -ggdb")

#set(RUNTIME_OUTPUT_DIRECTORY "./${CMAKE_BUILD_TYPE}")

include_directories("../../src/crypto/bdp")

aux_source_directory(. DIR_SRCS)
aux_source_directory(../../src/crypto/bdp DIR_SRCS)

link_directories("../../src/crypto/bdp")

add_executable(bdptest ${DIR_SRCS})
target_link_libraries(bdptest libpahafs.so)
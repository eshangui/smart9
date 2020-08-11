
cmake_minimum_required(VERSION 3.13)

project (d9main)
#set(CMAKE_BUILD_TYPE "Release")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -g")

set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g -ggdb")

#set(RUNTIME_OUTPUT_DIRECTORY "./${CMAKE_BUILD_TYPE}")

include_directories("./inc")
include_directories("./util")
include_directories("./src")
include_directories("./src/prt")
include_directories("./src/net")
include_directories("./src/crypto")
include_directories("./src/db")
include_directories("./src/db/zip")

aux_source_directory(./inc DIR_SRCS)
aux_source_directory(./util DIR_SRCS)
aux_source_directory(./src DIR_SRCS)
aux_source_directory(./src/prt DIR_SRCS)
aux_source_directory(./src/net DIR_SRCS)
aux_source_directory(./src/crytpo DIR_SRCS)
aux_source_directory(./src/db DIR_SRCS)
aux_source_directory(./src/db/zip DIR_SRCS)

add_executable(d9main ${DIR_SRCS})
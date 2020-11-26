
cmake_minimum_required(VERSION 3.13)

project (d9main)
#set(CMAKE_BUILD_TYPE "Release")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall -g")



set(CMAKE_BUILD_NAME linux)
#set(CMAKE_C_COMPILER "aarch64-linux-gnu-gcc")
set(CMAKE_C_COMPILER "/home/chester/wujf/code/test/Firefly-RK3308_Linux_SDK_git_20190924/prebuilts/gcc/linux-x86/aarch64/gcc-linaro-6.3.1-2017.05-x86_64_aarch64-linux-gnu/bin/aarch64-linux-gnu-gcc")
set(CMAKE_BUILD_TYPE "Debug")
set(CMAKE_C_FLAGS_DEBUG "-O0 -g -ggdb -lpthread")

#set(RUNTIME_OUTPUT_DIRECTORY "./${CMAKE_BUILD_TYPE}")

include_directories("./src")
include_directories("./src/prt")
include_directories("./src/net")
include_directories("./src/common")
include_directories("./src/tools")

aux_source_directory(./src DIR_SRCS)
aux_source_directory(./src/prt DIR_SRCS)
aux_source_directory(./src/net DIR_SRCS)
aux_source_directory(./src/common DIR_SRCS)
aux_source_directory(./src/tools DIR_SRCS)

add_executable(d9main ${DIR_SRCS})
target_link_libraries(${PROJECT_NAME} pthread)
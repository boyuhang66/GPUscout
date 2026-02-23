cmake_policy(SET CMP0048 NEW)
cmake_minimum_required(VERSION 3.26)

set (CMAKE_CXX_STANDARD 20)
set (CXX_STANDARD_REQUIRED ON)
#set (CMAKE_CUDA_STANDARD 17)

#gcc11+ (GLIBCXX_3.4.29)

project(GPUscout VERSION 0.2.2 LANGUAGES CXX)

# Define build target options for different gpu vendors: "nvidia", "amd", or "all"
set(VENDOR "all" CACHE STRING "Build target: amd, nvidia, or all")
set_property(CACHE VENDOR PROPERTY STRINGS amd nvidia all)

if(VENDOR STREQUAL "nvidia" OR VENDOR STREQUAL "all")
    add_subdirectory(src/nvidia)
endif()

if(VENDOR STREQUAL "amd" OR VENDOR STREQUAL "all")
    add_subdirectory(src/amd)
endif()
include_directories(src/utilities)

configure_file(GPUscout.sh.in ${CMAKE_CURRENT_BINARY_DIR}/GPUscout.sh @ONLY)


#find_package(CUDAToolkit 11.8 REQUIRED)
#include_directories(CUDA::cupti)
#link_libraries(CUDA::toolkit)

install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/GPUscout.sh DESTINATION . RENAME GPUscout)
set(CMAKE_CXX_COMPILER "dpcpp")
set(CMAKE_CXX_STANDARD 17)

cmake_minimum_required(VERSION 3.13)
project(barrel)

add_executable(fpga_emu src.cpp)
target_compile_options(fpga_emu PRIVATE -fintelfpga)
target_link_options(fpga_emu PRIVATE -fintelfpga)

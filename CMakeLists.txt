set(CMAKE_CXX_COMPILER "dpcpp")
set(CMAKE_CXX_STANDARD 17)

cmake_minimum_required(VERSION 3.10)
project(barrel)

add_executable(fpga_emu src.cpp)
set_target_properties(fpga_emu PROPERTIES COMPILE_FLAGS "-fintelfpga")
set_target_properties(fpga_emu PROPERTIES LINK_FLAGS "-fintelfpga")

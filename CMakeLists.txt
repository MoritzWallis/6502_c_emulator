cmake_minimum_required(VERSION 3.26)
project(6502 CXX)

set(CMAKE_CXX_STANDARD 14)

add_executable(6502
        6502v2.cpp
        6502v2.h
        addr_modes.cpp
        instructions.cpp
        impl_inst.cpp
)
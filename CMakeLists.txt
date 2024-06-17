cmake_minimum_required(VERSION 3.10)

# set the project name
project(Dodgy6502)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# add the executable
add_executable(Dodgy6502 6502v2.cpp impl_inst.cpp
        instructions.cpp
        addr_modes.cpp)

# if there are any libraries you need to link, use the target_link_libraries command
# target_link_libraries(Dodgy6502 PRIVATE some_library)
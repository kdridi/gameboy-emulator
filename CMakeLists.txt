cmake_minimum_required(VERSION 3.28)
project(gaboem)

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find FMT
find_package(fmt CONFIG REQUIRED)

# Find SFML
find_package(SFML COMPONENTS window graphics CONFIG REQUIRED)

add_executable(gaboem main.cpp)

# Link FMT to your project
target_link_libraries(gaboem PRIVATE fmt::fmt)

# Link SFML to your project
target_link_libraries(gaboem PRIVATE sfml-graphics sfml-window)
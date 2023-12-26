cmake_minimum_required(VERSION 3.28)
project(gaboem)

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option pour activer le Sanitizer Address en mode Debug
set(USE_ASAN OFF CACHE BOOL "Enable Address Sanitizer")

if (USE_ASAN)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
    set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")
endif()

# Enable testing
enable_testing()

find_package(GTest CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

# Add core library
add_library(gaboem_core STATIC
    lib/bus.c
    lib/cart.c
    lib/cpu.c
    lib/cpu_fetch.c
    lib/cpu_proc.c
    lib/emu.c
    lib/instruction.c
    lib/ppu.c
    lib/timer.c
)
# Inclure les répertoires d'en-têtes de SDL2
target_include_directories(gaboem_core PRIVATE ${SDL2_INCLUDE_DIRS})
target_include_directories(gaboem_core PRIVATE ${SDL2_TTF_INCLUDE_DIRS})
target_include_directories(gaboem_core PUBLIC include)

add_executable(gaboem main.cpp)

# Link FMT to your project
target_link_libraries(gaboem
    PRIVATE
    gaboem_core
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
)

add_executable(gaboem_test
    tests/cart_test.cpp
)

target_link_libraries(gaboem_test
    PRIVATE
    gaboem_core
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME gaboem_test COMMAND gaboem_test)

# run_tests command using CTest
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS gaboem_test)

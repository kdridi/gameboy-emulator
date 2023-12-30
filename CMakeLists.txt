cmake_minimum_required(VERSION 3.28)
project(gaboem)

# Set the C++ standard to C++23
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Option pour activer le Sanitizer Address en mode Debug
set(USE_ASAN OFF CACHE BOOL "Enable Address Sanitizer")

if (USE_ASAN)
    add_compile_options(-fsanitize=address -W -Wall -Wextra -Wpedantic -Werror)
    add_link_options(-fsanitize=address)
endif()

# Enable testing
enable_testing()

find_package(GTest CONFIG REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(SDL2_ttf CONFIG REQUIRED)

# Add core library
add_library(gaboem_core STATIC
    include/bus.h
    lib/bus.c
    include/cart.h
    lib/cart.c
    include/cpu.h
    lib/cpu.c
    include/dbg.h
    lib/dbg.c
    include/dma.h
    lib/dma.c
    include/emu.h
    lib/emu.c
    include/gamepad.h
    lib/gamepad.c
    include/instruction.h
    lib/instruction.c
    include/interrupts.h
    lib/interrupts.c
    include/io.h
    lib/io.c
    include/lcd.h
    lib/lcd.c
    include/ppu_pipeline.h
    lib/ppu_pipeline.c
    include/ppu_sm.h
    lib/ppu_sm.c
    include/ppu.h
    lib/ppu.c
    include/ram.h
    lib/ram.c
    include/stack.h
    lib/stack.c
    include/timer.h
    lib/timer.c
    include/ui.h
    lib/ui.c
    
    lib/cpu_fetch.c
    lib/cpu_proc.c
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
    # tests/cart_test.cpp
    tests/cpu_tests.cpp
    tests/stack_tests.cpp
)

target_link_libraries(gaboem_test
    PRIVATE
    gaboem_core
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME gaboem_test COMMAND gaboem_test)

# run_tests command using CTest
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --verbose
    DEPENDS gaboem_test)

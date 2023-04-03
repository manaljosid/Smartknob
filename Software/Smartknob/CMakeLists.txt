cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(Smartknob C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(main main.cpp)

pico_enable_stdio_usb(main 1)
pico_enable_stdio_uart(main 0)

pico_add_extra_outputs(main)

add_subdirectory(lib)

target_link_libraries(main pico_stdlib hardware_i2c hardware_spi MT6701 MCP3564R FOC) # Insert libraries used in here
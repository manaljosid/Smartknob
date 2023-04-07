add_library(TMC6300 INTERFACE)

target_sources(TMC6300 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/TMC6300.cpp
)

target_include_directories(TMC6300 INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(TMC6300 INTERFACE hardware_clocks hardware_pwm hardware_gpio)
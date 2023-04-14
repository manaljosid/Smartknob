add_library(FOC INTERFACE)

target_sources(FOC INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/FOC.cpp
)

target_include_directories(FOC INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(FOC INTERFACE TMC6300 MT6701 hardware_pwm hardware_gpio)
add_library(MCP3564R INTERFACE)

target_sources(MCP3564R INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/MCP3564R.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MCP3564R_regs.h
)

target_include_directories(MCP3564R INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(MCP3564R INTERFACE hardware_spi hardware_gpio)
add_library(SCD30 INTERFACE)

target_sources(SCD30 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/SCD30.cpp
)

target_include_directories(SCD30 INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(SCD30 INTERFACE hardware_i2c)
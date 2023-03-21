add_library(MT6701 INTERFACE)

target_sources(MT6701 INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/MT6701.cpp
)

target_include_directories(MT6701 INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(MT6701 INTERFACE hardware_spi)
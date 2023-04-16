add_library(FIR INTERFACE)

target_sources(FIR INTERFACE)

target_include_directories(FIR INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(FIR INTERFACE)
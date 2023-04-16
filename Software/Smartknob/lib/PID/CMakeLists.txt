add_library(PID INTERFACE)

target_sources(PID INTERFACE
    ${CMAKE_CURRENT_LIST_DIR}/PID.cpp
)

target_include_directories(PID INTERFACE ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(PID INTERFACE)
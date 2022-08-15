set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(network_demo main.cpp)

target_link_libraries(network_demo 
PRIVATE
    network
)
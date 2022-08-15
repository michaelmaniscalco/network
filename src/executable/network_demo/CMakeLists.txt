add_executable(network_demo main.cpp)

target_link_libraries(network_demo 
PRIVATE
    network
)
add_library(network
    ./network.cpp
    ./socket/socket_id.cpp
    ./ip/ip_address.cpp
    ./socket/active_socket.cpp
    ./socket/passive_socket.cpp
    ./polling/poller.cpp
    ./polling/poller_registration.cpp
    ./network_interface/network_interface.cpp
    ./socket/private/socket_base_impl.cpp
    ./socket/private/passive_socket_impl.cpp
    ./socket/private/active_socket_impl.cpp
)

target_link_libraries(network 
PUBLIC
    fmt::fmt
    work_contract
    )

target_include_directories(network
    PUBLIC
        $<BUILD_INTERFACE:${_network_include_dir}>
        $<INSTALL_INTERFACE:include/network>
        ${_common_src_path}/src
        ${_work_contract_src_path}/src
        ${_fmt_src_path}/include
)

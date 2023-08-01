#include "./network_interface.h"


//=============================================================================
maniscalco::network::network_interface::network_interface
(
):
    poller_(std::make_shared<poller>(poller::configuration{})),
    workContractGroup_({default_capacity})
{
}


//=============================================================================
maniscalco::network::network_interface::network_interface
(
    configuration const & config
):
    poller_(std::make_shared<poller>(config.poller_)),
    workContractGroup_({config.capacity_})
{
}


//=============================================================================
template <maniscalco::network::socket_concept S, typename T>
auto maniscalco::network::network_interface::open_socket
(
    T handle,
    typename S::configuration config,
    typename S::event_handlers eventHandlers
) -> S
{
    return S{std::move(handle), config, eventHandlers, workContractGroup_, *poller_};
}


//=============================================================================
auto maniscalco::network::network_interface::tcp_listen
(
    socket_address socketAddress,
    tcp_listener_socket::configuration config,
    tcp_listener_socket::event_handlers eventHandlers
) -> tcp_listener_socket
{
    return open_socket<tcp_listener_socket>(socketAddress, config, eventHandlers);
}


//=============================================================================
auto maniscalco::network::network_interface::tcp_accept
(
    system::file_descriptor fileDescriptor,
    tcp_socket::configuration config,
    tcp_socket::event_handlers eventHandlers
) -> tcp_socket
{
    return open_socket<tcp_socket>(std::move(fileDescriptor), config, eventHandlers);
}


//=============================================================================
auto maniscalco::network::network_interface::tcp_connect
(
    ip_address localIpAddress,
    socket_address remoteSocketAddress,
    tcp_socket::configuration config,
    tcp_socket::event_handlers eventHandlers
) -> tcp_socket
{
    auto tcpSocket = open_socket<tcp_socket>(localIpAddress, config, eventHandlers);
    tcpSocket.connect_to(remoteSocketAddress);
    return tcpSocket;
}


//=============================================================================
auto maniscalco::network::network_interface::udp_connect
(
    socket_address localSocketAddress,
    socket_address remoteSocketAddress,
    udp_socket::configuration config,
    udp_socket::event_handlers eventHandlers
) -> udp_socket
{
    auto udpSocket = udp_connectionless(localSocketAddress, config, eventHandlers);
    if (remoteSocketAddress.is_valid())
        udpSocket.connect_to(remoteSocketAddress);
    return udpSocket;
}


//=============================================================================
auto maniscalco::network::network_interface::udp_connectionless
(
    socket_address localSocketAddress,
    udp_socket::configuration config,
    udp_socket::event_handlers eventHandlers
) -> udp_socket
{
    return open_socket<udp_socket>(localSocketAddress, config, eventHandlers);
}


//=============================================================================
auto maniscalco::network::network_interface::multicast_join
(
    socket_address socketAddress,
    udp_socket::configuration config,
    udp_socket::event_handlers eventHandlers
) -> udp_socket
{
    auto udpSocket = open_socket<udp_socket>(socket_address{in_addr_any, socketAddress.get_port_id()}, config, eventHandlers);
    udpSocket.join(socketAddress.get_network_id());
    return udpSocket;
}


//=============================================================================
void maniscalco::network::network_interface::poll
(
)
{
    poller_->poll();
}


//=============================================================================
void maniscalco::network::network_interface::service_sockets
(
)
{
    workContractGroup_.service_contracts();
}


//=============================================================================
namespace maniscalco::network
{
    template tcp_socket network_interface::open_socket(system::file_descriptor, tcp_socket::configuration, tcp_socket::event_handlers);
    template tcp_socket network_interface::open_socket(ip_address, tcp_socket::configuration, tcp_socket::event_handlers);
    template tcp_listener_socket network_interface::open_socket(socket_address, tcp_listener_socket::configuration, tcp_listener_socket::event_handlers);
    template udp_socket network_interface::open_socket(socket_address, udp_socket::configuration, udp_socket::event_handlers);
}
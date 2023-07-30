#include "./network_interface.h"


//=============================================================================
maniscalco::network::network_interface::network_interface
(
    configuration const & config,
    std::shared_ptr<system::work_contract_group> workContractGroup
):
    poller_(std::make_shared<poller>(config.poller_)),
    workContractGroup_(workContractGroup)
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
    return S{std::move(handle), config, eventHandlers, *workContractGroup_, *poller_};
}


//=============================================================================
auto maniscalco::network::network_interface::tcp_listen
(
    ip_address ipAddress,
    tcp_listener_socket::configuration config,
    tcp_listener_socket::event_handlers eventHandlers
) -> tcp_listener_socket
{
    return open_socket<tcp_listener_socket>(ipAddress, config, eventHandlers);
}


//=============================================================================
auto maniscalco::network::network_interface::tcp_connect
(
    network_id localNetworkId,
    ip_address remoteIpAddress,
    tcp_socket::configuration config,
    tcp_socket::event_handlers eventHandlers
) -> tcp_socket
{
    auto tcpSocket = open_socket<tcp_socket>(localNetworkId, config, eventHandlers);
    tcpSocket.connect_to(remoteIpAddress);
    return tcpSocket;
}


//=============================================================================
auto maniscalco::network::network_interface::udp_connect
(
    ip_address localIpAddress,
    ip_address remoteIpAddress,
    udp_socket::configuration config,
    udp_socket::event_handlers eventHandlers
) -> udp_socket
{
    auto udpSocket = open_socket<udp_socket>(localIpAddress, config, eventHandlers);
    udpSocket.connect_to(remoteIpAddress);
    return udpSocket;
}


//=============================================================================
auto maniscalco::network::network_interface::multicast_join
(
    ip_address ipAddress,
    udp_socket::configuration config,
    udp_socket::event_handlers eventHandlers
) -> udp_socket
{
    auto udpSocket = open_socket<udp_socket>(ip_address{in_addr_any, ipAddress.get_port_id()}, config, eventHandlers);
    udpSocket.join(ipAddress.get_network_id());
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
namespace maniscalco::network
{
    template tcp_socket network_interface::open_socket(system::file_descriptor, tcp_socket::configuration, tcp_socket::event_handlers);
    template tcp_socket network_interface::open_socket(network_id, tcp_socket::configuration, tcp_socket::event_handlers);
    template tcp_listener_socket network_interface::open_socket(ip_address, tcp_listener_socket::configuration, tcp_listener_socket::event_handlers);
    template udp_socket network_interface::open_socket(ip_address, udp_socket::configuration, udp_socket::event_handlers);
}
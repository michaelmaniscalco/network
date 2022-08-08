#include "./network_interface.h"

#include <library/network/socket/private/udp_socket_impl.h>

#include <iostream>


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
auto maniscalco::network::network_interface::open_socket
(
    ip_address ipAddress,
    tcp_socket::configuration config,
    tcp_socket::event_handlers eventHandlers
) -> tcp_socket
{
    return tcp_socket{ipAddress, config, eventHandlers, *workContractGroup_, *poller_};
}


//=============================================================================
auto maniscalco::network::network_interface::open_socket
(
    file_descriptor fileDescriptor,
    tcp_socket::configuration config,
    tcp_socket::event_handlers eventHandlers
) -> tcp_socket
{
    return tcp_socket{std::move(fileDescriptor), config, eventHandlers, *workContractGroup_, *poller_};
}


//=============================================================================
auto maniscalco::network::network_interface::open_socket
(
    ip_address ipAddress,
    udp_socket::configuration config,
    udp_socket::event_handlers eventHandlers
) -> udp_socket
{
    return udp_socket{ipAddress, config, eventHandlers, *workContractGroup_, *poller_};
}


//=============================================================================
auto maniscalco::network::network_interface::open_socket
(
    ip_address ipAddress,
    tcp_listener_socket::configuration config,
    tcp_listener_socket::event_handlers eventHandlers
) -> tcp_listener_socket
{
    return tcp_listener_socket{ipAddress, config, eventHandlers, *workContractGroup_, *poller_};
}


//=============================================================================
void maniscalco::network::network_interface::poll
(
)
{
    poller_->poll();
}


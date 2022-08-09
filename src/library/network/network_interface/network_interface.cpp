#include "./network_interface.h"

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
void maniscalco::network::network_interface::poll
(
)
{
    poller_->poll();
}


//=============================================================================
namespace maniscalco::network
{

    template tcp_socket network_interface::open_socket(file_descriptor, tcp_socket::configuration, tcp_socket::event_handlers);

    template tcp_socket network_interface::open_socket(ip_address,tcp_socket::configuration, tcp_socket::event_handlers);

    template tcp_listener_socket network_interface::open_socket(ip_address, tcp_listener_socket::configuration, tcp_listener_socket::event_handlers);

    template udp_socket network_interface::open_socket(ip_address, udp_socket::configuration, udp_socket::event_handlers);
    
}
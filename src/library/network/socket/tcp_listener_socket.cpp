#include "./tcp_listener_socket.h"

#include "./private/tcp_listener_socket_impl.h"

#include <iostream>


//=============================================================================
maniscalco::network::tcp_listener_socket::socket
(
    ip_address ipAddress,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
)
{
    impl_ = std::move(decltype(impl_)(new socket_impl<traits>(
            ipAddress, 
            {
                .backlog_ = 128
            }, 
            socket_impl<traits>::event_handlers
            {
                eventHandlers.closeHandler_,
                nullptr,
                eventHandlers.acceptHandler_
            },
            workContractGroup, p), 
            [](auto * impl){impl->destroy();}));
}


//=============================================================================
maniscalco::network::tcp_listener_socket::~socket
(
)
{
}


//=============================================================================
bool maniscalco::network::tcp_listener_socket::close
(
)
{
    if (impl_)
        return impl_->close();
    return false;
}


//=============================================================================
bool maniscalco::network::tcp_listener_socket::is_valid
(
) const noexcept
{
    if (impl_)
        return impl_->is_valid();
    return false;
}


//=============================================================================
auto maniscalco::network::tcp_listener_socket::get_ip_address
(
) const noexcept -> ip_address
{
    if (impl_)
        return impl_->get_ip_address();
    return {};
}


//=============================================================================
auto maniscalco::network::tcp_listener_socket::get_id
(
) const -> socket_id
{
    if (impl_)
        return impl_->get_id();
    return {};
}

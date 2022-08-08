#include "./udp_socket.h"

#include "./private/udp_socket_impl.h"

#include <iostream>


//=============================================================================
maniscalco::network::udp_socket::socket
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
            {}, // TODO: copy fields from config when available 
            {
                .closeHandler_ = eventHandlers.closeHandler_,
                .receiveHandler_ = eventHandlers.receiveHandler_
            },
            workContractGroup, p), 
            [](auto * impl){impl->destroy();}));
}


//=============================================================================
maniscalco::network::udp_socket::~socket
(
)
{
}


//=============================================================================
auto maniscalco::network::udp_socket::connect_to
(
    ip_address const & destination
) noexcept -> connect_result
{
    if (impl_)
        return impl_->connect_to(destination);
    return connect_result::connect_error;
}


//=============================================================================
std::span<char const> maniscalco::network::udp_socket::send
(
    std::span<char const> data
)
{
    if (impl_)
        return impl_->send(data);
    return data;
}


//=============================================================================
bool maniscalco::network::udp_socket::close
(
)
{
    if (impl_)
        return impl_->close();
    return false;
}


//=============================================================================
bool maniscalco::network::udp_socket::is_valid
(
) const noexcept
{
    if (impl_)
        return impl_->is_valid();
    return false;
}


//=============================================================================
auto maniscalco::network::udp_socket::get_ip_address
(
) const noexcept -> ip_address
{
    if (impl_)
        return impl_->get_ip_address();
    return {};
}


//=============================================================================
bool maniscalco::network::udp_socket::is_connected
(
) const noexcept
{
    if (impl_)
        return impl_->is_connected();
    return false;
}


//=============================================================================
auto maniscalco::network::udp_socket::get_connected_ip_address
(
) const noexcept -> ip_address
{
    if (impl_)
        return impl_->get_connected_ip_address();
    return {};
}


//=============================================================================
std::vector<std::uint8_t> maniscalco::network::udp_socket::receive
(
)
{
    if (impl_)
        return impl_->receive();
    return {};
}


//=============================================================================
auto maniscalco::network::udp_socket::get_id
(
) const -> socket_id
{
    if (impl_)
        return impl_->get_id();
    return {};
}

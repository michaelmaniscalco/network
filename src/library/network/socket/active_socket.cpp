#include "./active_socket.h"

#include "./private/active_socket_impl.h"


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
maniscalco::network::active_socket<P>::socket
(
    ip_address ipAddress,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
)
{
    impl_ = std::move(decltype(impl_)(new impl_type(
            ipAddress, 
            {
                .receiveBufferSize_ = config.receiveBufferSize_,
                .sendBufferSize_ = config.sendBufferSize_,
                .ioMode_ = config.ioMode_
            },
            {
                eventHandlers.closeHandler_,
                eventHandlers.receiveHandler_,
                eventHandlers.receiveErrorHandler_,
                eventHandlers.packetAllocationHandler_
            },
            workContractGroup, p), 
            [](auto * impl){impl->destroy();}));
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
maniscalco::network::active_socket<P>::socket
(
    system::file_descriptor fileDescriptor,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
) requires (tcp_protocol_concept<P>)
{
    impl_ = std::move(decltype(impl_)(new impl_type(
            std::move(fileDescriptor), 
            {
                .receiveBufferSize_ = config.receiveBufferSize_,
                .sendBufferSize_ = config.sendBufferSize_,
                .ioMode_ = config.ioMode_
            },
            {
                eventHandlers.closeHandler_,
                eventHandlers.receiveHandler_,
                eventHandlers.receiveErrorHandler_,
                eventHandlers.packetAllocationHandler_
            },
            workContractGroup, p), 
            [](auto * impl){impl->destroy();}));
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket<P>::connect_to
(
    ip_address const & destination
) noexcept -> connect_result
{
    if (impl_)
        return impl_->connect_to(destination);
    return connect_result::connect_error;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket<P>::join
(
    network_id networkId
) -> connect_result 
requires (P == network_transport_protocol::udp)
{
    if (impl_)
        return impl_->join(networkId);
    return connect_result::connect_error;
}

//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket<P>::send
(
    std::span<char const> buffer
) -> send_result
{
    if (impl_)
        return impl_->send(std::move(buffer));
    return {ENOTCONN, 0};
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket<P>::close
(
)
{
    if (impl_)
        return impl_->close();
    return false;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket<P>::is_valid
(
) const noexcept
{
    if (impl_)
        return impl_->is_valid();
    return false;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket<P>::get_ip_address
(
) const noexcept -> ip_address
{
    if (impl_)
        return impl_->get_ip_address();
    return {};
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket<P>::is_connected
(
) const noexcept
{
    if (impl_)
        return impl_->is_connected();
    return false;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket<P>::get_connected_ip_address
(
) const noexcept -> ip_address
{
    if (impl_)
        return impl_->get_connected_ip_address();
    return {};
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket<P>::get_id
(
) const -> socket_id
{
    if (impl_)
        return impl_->get_id();
    return {};
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket<P>::shutdown
(
) noexcept
{
    if (impl_)
        return impl_->shutdown();
    return false;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket<P>::set_read_only
(
) noexcept
{
    return set_io_mode(system::io_mode::read);
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket<P>::set_write_only
(
) noexcept
{
    return set_io_mode(system::io_mode::write);
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket<P>::set_read_write
(
) noexcept
{
    return set_io_mode(system::io_mode::read_write);
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket<P>::set_io_mode
(
    system::io_mode ioMode
) noexcept
{
    if (impl_)
        return impl_->set_io_mode(ioMode);
    return false;
}


//=============================================================================
namespace maniscalco::network
{
    template class socket<tcp_socket_traits>;
    template class socket<udp_socket_traits>;
}

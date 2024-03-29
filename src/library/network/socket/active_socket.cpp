#include "./active_socket.h"
#include "./private/active_socket_impl.h"


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
maniscalco::network::active_socket<P>::socket
(
    socket_address socketAddress,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
) requires (udp_protocol_concept<P>)
{
    impl_ = std::move(decltype(impl_)(new impl_type(
            socketAddress, 
            {
                .socketReceiveBufferSize_ = config.socketReceiveBufferSize_,
                .socketSendBufferSize_ = config.socketSendBufferSize_,
                .readBufferSize_ = config.readBufferSize_,
                .ioMode_ = config.ioMode_
            },
            {
                eventHandlers.closeHandler_,
                eventHandlers.pollErrorHandler_,
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
    ip_address ipAddress,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
) requires (tcp_protocol_concept<P>)
{
    impl_ = std::move(decltype(impl_)(new impl_type(
            {ipAddress}, 
            {
                .socketReceiveBufferSize_ = config.socketReceiveBufferSize_,
                .socketSendBufferSize_ = config.socketSendBufferSize_,
                .readBufferSize_ = config.readBufferSize_,
                .ioMode_ = config.ioMode_
            },
            {
                eventHandlers.closeHandler_,
                eventHandlers.pollErrorHandler_,
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
                .socketReceiveBufferSize_ = config.socketReceiveBufferSize_,
                .socketSendBufferSize_ = config.socketSendBufferSize_,
                .readBufferSize_ = config.readBufferSize_,
                .ioMode_ = config.ioMode_
            },
            {
                eventHandlers.closeHandler_,
                eventHandlers.pollErrorHandler_,
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
    socket_address destination
) noexcept -> connect_result
{
    return (impl_) ? impl_->connect_to(destination) : connect_result::connect_error;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket<P>::join
(
    ip_address ipAddress
) -> connect_result 
requires (P == network_transport_protocol::udp)
{
    return (impl_) ? impl_->join(ipAddress) : connect_result::connect_error;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket<P>::send
(
    std::span<char const> source
) -> std::tuple<std::span<char const>, std::int32_t>
{
    return (impl_) ? impl_->send(source) : std::make_tuple(source, ENOTCONN);
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket<P>::send_to
(
    socket_address destinationSocketAddress,
    std::span<char const> source
) -> std::tuple<std::span<char const>, std::int32_t> 
requires (P == network_transport_protocol::udp)
{
    return (impl_) ? impl_->send_to(destinationSocketAddress, source) : std::make_tuple(source, ENOTCONN);
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket<P>::close
(
)
{
    return (impl_) ? impl_->close() : false;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket<P>::is_valid
(
) const noexcept
{
    return (impl_) ? impl_->is_valid() : false;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket<P>::get_ip_address
(
) const noexcept -> socket_address
{
    return (impl_) ? impl_->get_ip_address() : socket_address{};
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket<P>::is_connected
(
) const noexcept
{
    return (impl_) ? impl_->is_connected() : false;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket<P>::get_peer_ip_address
(
) const noexcept -> socket_address
{
    return (impl_) ? impl_->get_peer_ip_address() : socket_address{};
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket<P>::get_id
(
) const -> socket_id
{
    return (impl_) ? impl_->get_id() : socket_id{};
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket<P>::shutdown
(
) noexcept
{
    return (impl_) ? impl_->shutdown() : false;
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
    return (impl_) ? impl_->set_io_mode(ioMode) : false;
}


//=============================================================================
namespace maniscalco::network
{
    template class socket<tcp_socket_traits>;
    template class socket<udp_socket_traits>;
}

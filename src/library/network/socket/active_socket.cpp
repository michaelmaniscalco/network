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
            {}, // TODO: copy fields from config when available 
            {
                eventHandlers.closeHandler_,
                eventHandlers.receiveHandler_
            },
            workContractGroup, p), 
            [](auto * impl){impl->destroy();}));
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
maniscalco::network::active_socket<P>::socket
(
    file_descriptor fileDescriptor,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
)
{
    impl_ = std::move(decltype(impl_)(new impl_type(
            std::move(fileDescriptor), 
            {}, // TODO: copy fields from config when available 
            {
                eventHandlers.closeHandler_,
                eventHandlers.receiveHandler_
            },
            workContractGroup, p), 
            [](auto * impl){impl->destroy();}));
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
maniscalco::network::active_socket<P>::~socket
(
)
{
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
std::span<char const> maniscalco::network::active_socket<P>::send
(
    std::span<char const> data
)
{
    if (impl_)
        return impl_->send(data);
    return data;
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
std::vector<std::uint8_t> maniscalco::network::active_socket<P>::receive
(
)
{
    if (impl_)
        return impl_->receive();
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
namespace maniscalco::network
{
    template class socket<tcp_socket_traits>;
    template class socket<udp_socket_traits>;
}

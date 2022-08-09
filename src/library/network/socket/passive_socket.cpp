#include "./passive_socket.h"

#include "./private/passive_socket_impl.h"


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
maniscalco::network::passive_socket<P>::socket
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
                .backlog_ = 128
            }, 
            {
                eventHandlers.closeHandler_,
                eventHandlers.acceptHandler_
            },
            workContractGroup, p), 
            [](auto * impl){impl->destroy();}));
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
maniscalco::network::passive_socket<P>::~socket
(
)
{
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::passive_socket<P>::close
(
)
{
    if (impl_)
        return impl_->close();
    return false;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::passive_socket<P>::is_valid
(
) const noexcept
{
    if (impl_)
        return impl_->is_valid();
    return false;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::passive_socket<P>::get_ip_address
(
) const noexcept -> ip_address
{
    if (impl_)
        return impl_->get_ip_address();
    return {};
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::passive_socket<P>::get_id
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
    template class socket<tcp_listener_socket_traits>;
}

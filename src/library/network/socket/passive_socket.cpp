#include "./passive_socket.h"

#include "./private/passive_socket_impl.h"


//=============================================================================
maniscalco::network::passive_socket::socket
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
bool maniscalco::network::passive_socket::close
(
)
{
    if (impl_)
        return impl_->close();
    return false;
}


//=============================================================================
bool maniscalco::network::passive_socket::is_valid
(
) const noexcept
{
    if (impl_)
        return impl_->is_valid();
    return false;
}


//=============================================================================
auto maniscalco::network::passive_socket::get_ip_address
(
) const noexcept -> ip_address
{
    if (impl_)
        return impl_->get_ip_address();
    return {};
}


//=============================================================================
auto maniscalco::network::passive_socket::get_id
(
) const -> socket_id
{
    if (impl_)
        return impl_->get_id();
    return {};
}

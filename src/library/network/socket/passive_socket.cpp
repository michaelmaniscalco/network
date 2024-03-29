#include "./passive_socket.h"
#include "./private/passive_socket_impl.h"


//=============================================================================
maniscalco::network::passive_socket::socket
(
    socket_address socketAddress,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
)
{
    impl_ = std::move(decltype(impl_)(new impl_type(
            socketAddress,
            {
                .backlog_ = config.backlog_
            }, 
            {
                eventHandlers.closeHandler_,
                eventHandlers.pollErrorHandler_,
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
    return (impl_) ? impl_->close() : false;
}


//=============================================================================
bool maniscalco::network::passive_socket::is_valid
(
) const noexcept
{
    return (impl_) ? impl_->is_valid() : false;
}


//=============================================================================
auto maniscalco::network::passive_socket::get_ip_address
(
) const noexcept -> socket_address
{
    return (impl_) ? impl_->get_ip_address() : socket_address{};
}


//=============================================================================
auto maniscalco::network::passive_socket::get_id
(
) const -> socket_id
{
    return (impl_) ? impl_->get_id() : socket_id{};
}

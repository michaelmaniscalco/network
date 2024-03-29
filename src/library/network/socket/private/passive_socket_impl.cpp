#include "./passive_socket_impl.h"


//=============================================================================
maniscalco::network::passive_socket_impl::socket_impl
(
    socket_address socketAddress,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
) noexcept :    
    socket_base_impl(socketAddress, {.ioMode_ = config.ioMode_}, eventHandlers, ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP),
            workContractGroup.create_contract([this](){this->accept();}, [this](){this->destroy();})),
    pollerRegistration_(p.register_socket(*this)),
    acceptHandler_(eventHandlers.acceptHandler_)   
{
    ::listen(fileDescriptor_.get(), config.backlog_);
}


//=============================================================================
void maniscalco::network::passive_socket_impl::accept
(
)
{
    ::sockaddr address;
    socklen_t addressLength = sizeof(address);
    system::file_descriptor fileDescriptor(::accept(fileDescriptor_.get(), &address, &addressLength));
    if (acceptHandler_)
        acceptHandler_(id_, std::move(fileDescriptor));
}


//=============================================================================
void maniscalco::network::passive_socket_impl::destroy
(
    // use the work contract to asynchronously delete 'this'.
    // doing it this way ensures that the work contract's primary
    // work can not be executed any longer just prior to deleting
    // this.  This allows the primary work contract function to 
    // use a raw 'this' 
)
{
    if (workContract_.is_valid())
    {
        workContract_.surrender();
    }
    else
    {
        // remove this socket from the poller before deleting 
        // 'this' as the poller has a raw pointer to 'this'.
        pollerRegistration_.release();
        delete this;
    }
}

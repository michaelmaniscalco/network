#include "./udp_socket_impl.h"

#include <iostream>


//=============================================================================
maniscalco::network::udp_socket_impl::socket_impl
(
    ip_address ipAddress,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
):    
    socket_base_impl(ipAddress, config, eventHandlers, ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP),
            workContractGroup.create_contract(
            {
                .contractHandler_ = [this](){this->receive();},
                .endContractHandler_ = [this](){this->destroy();}
            })),
    pollerRegistration_(p.register_socket(*this))    
{
}


//=============================================================================
auto maniscalco::network::udp_socket_impl::connect_to
(
    ip_address const & destination
) noexcept -> connect_result
{
    if (!destination.is_valid())
        return connect_result::invalid_destination;

    if (!fileDescriptor_.is_valid())
        return connect_result::invalid_file_descriptor;

    if (is_connected())
        return connect_result::already_connected;

    ::sockaddr_in socketAddress = destination;
    socketAddress.sin_family = AF_INET;
    auto result = ::connect(fileDescriptor_.get(), (sockaddr const *)&socketAddress, sizeof(socketAddress));
    if ((result != 0) && (errno != EINPROGRESS))
        return connect_result::connect_error;
    connectedIpAddress_ = destination;
    return connect_result::success;
}


//=============================================================================
std::span<char const> maniscalco::network::udp_socket_impl::send
(
    std::span<char const> data
)
{
    auto result = ::send(fileDescriptor_.get(), data.data(), data.size(), MSG_NOSIGNAL);
    if ((result < 0) && (errno != EAGAIN))
    {
        ;//on_send_error(result);
        return data;
    }
    return data.subspan(result);
}


//=============================================================================
void maniscalco::network::udp_socket_impl::destroy
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

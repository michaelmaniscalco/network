#include "./active_socket_impl.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
maniscalco::network::active_socket_impl<P>::socket_impl
(
    ip_address ipAddress,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
):    
    socket_base_impl(ipAddress, {.ioMode_ = config.ioMode_}, eventHandlers, 
            (P == network_transport_protocol::udp) ? ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP) : ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP),
            workContractGroup.create_contract(
            {
                .contractHandler_ = [this](){this->receive();},
                .endContractHandler_ = [this](){this->destroy();}
            })),
    pollerRegistration_(p.register_socket(*this)),
    receiveHandler_(eventHandlers.receiveHandler_)    
{
    if (config.receiveBufferSize_ > 0)
        set_socket_option(SOL_SOCKET, SO_RCVBUF, config.receiveBufferSize_);
    if (config.sendBufferSize_ > 0)
        set_socket_option(SOL_SOCKET, SO_SNDBUF, config.sendBufferSize_);
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
maniscalco::network::active_socket_impl<P>::socket_impl
(
    system::file_descriptor fileDescriptor,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
):
    socket_base_impl({.ioMode_ = config.ioMode_}, eventHandlers, std::move(fileDescriptor),
            workContractGroup.create_contract(
            {
                .contractHandler_ = [this](){this->receive();},
                .endContractHandler_ = [this](){this->destroy();}
            })),
    pollerRegistration_(p.register_socket(*this)),
    receiveHandler_(eventHandlers.receiveHandler_)    
{
    connectedIpAddress_ = get_peer_name();
    if (config.receiveBufferSize_ > 0)
        set_socket_option(SOL_SOCKET, SO_RCVBUF, config.receiveBufferSize_);
    if (config.sendBufferSize_ > 0)
        set_socket_option(SOL_SOCKET, SO_SNDBUF, config.sendBufferSize_);
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket_impl<P>::connect_to
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

    set_socket_option(IPPROTO_IP, IP_MULTICAST_TTL, 8);
    return connect_result::success;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket_impl<P>::join
(
    network_id networkId
) -> connect_result
requires (P == network_transport_protocol::udp)
{
    if (!networkId.is_valid())
        return connect_result::invalid_destination;

    if (!fileDescriptor_.is_valid())
        return connect_result::invalid_file_descriptor;

    if (is_connected())
        return connect_result::already_connected;

    // join multicast
    ::ip_mreq mreq;
    ::memset(&mreq, 0x00, sizeof(mreq));
    mreq.imr_multiaddr = networkId;
    mreq.imr_interface = in_addr_any;
    if (!set_socket_option(IPPROTO_IP, IP_ADD_MEMBERSHIP, mreq))
    {
        // TODO: log failure
        return connect_result::connect_error;
    }
    connectedIpAddress_ = {networkId, port_id{0}};
    set_io_mode(system::io_mode::read);
    return connect_result::success;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket_impl<P>::disconnect
(
)
{
    if (!is_connected())
        return false;
    if (!fileDescriptor_.is_valid())
        return false;

    if (connectedIpAddress_.is_multicast())
    {
        if constexpr (P == network_transport_protocol::udp)
        {
            // drop multicast
            ::ip_mreq mreq;
            ::memset(&mreq, 0x00, sizeof(mreq));
            mreq.imr_multiaddr = connectedIpAddress_.get_network_id();
            mreq.imr_interface = in_addr_any;
            if (!set_socket_option(IPPROTO_IP, IP_DROP_MEMBERSHIP, mreq))
            {
                // TODO: log failure
                return false;
            }
            connectedIpAddress_ = {};
            return true;
        }
    }
    return true;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
std::span<char const> maniscalco::network::active_socket_impl<P>::send
(
    std::span<char const> data
)
{
    auto result = ::send(fileDescriptor_.get(), data.data(), data.size(), MSG_NOSIGNAL);
    if ((result < 0) && (errno != EAGAIN))
    {
        // TODO: log/maybe forward error
        return data;
    }
    return data.subspan(result);
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
std::vector<std::uint8_t> maniscalco::network::active_socket_impl<P>::receive
(
)
{
    // TODO: get from allocator
    std::vector<std::uint8_t> buffer(2048);

    auto result = ::recv(fileDescriptor_.get(), buffer.data(), buffer.capacity(), 0);
    if (result > 0)
    {
        buffer.resize(result);
        receiveHandler_(id_, std::move(buffer));
        on_polled(); // there could be more ...
        return buffer;
    }
    if (result == EAGAIN)
        on_polled();
    // TODO: deal with actual errors
    return {};
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
void maniscalco::network::active_socket_impl<P>::destroy
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
        disconnect();
        pollerRegistration_.release();
        delete this;
    }
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket_impl<P>::is_connected
(
) const noexcept
{
    return (connectedIpAddress_.is_valid());
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket_impl<P>::get_connected_ip_address
(
) const noexcept -> ip_address
{
    return connectedIpAddress_;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket_impl<P>::get_peer_name
(
) const noexcept -> ip_address
{
    ::sockaddr_in socketAddress;
    socketAddress.sin_family = AF_INET;
    ::socklen_t sizeofSocketAddress(sizeof(socketAddress));
    if (::getpeername(fileDescriptor_.get(), (struct sockaddr *)&socketAddress, &sizeofSocketAddress) == 0)
        return {socketAddress};
    return {};
}


//=============================================================================
namespace maniscalco::network
{
    template class socket_impl<tcp_socket_traits>;
    template class socket_impl<udp_socket_traits>;
}

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
) try :
    socket_base_impl(ipAddress, {.ioMode_ = config.ioMode_}, eventHandlers, 
            (P == network_transport_protocol::udp) ? ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP) : ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP),
            workContractGroup.create_contract([this](){this->receive();}, [this](){this->destroy();})),
    pollerRegistration_(p.register_socket(*this)),
    receiveHandler_(eventHandlers.receiveHandler_),
    receiveErrorHandler_(eventHandlers.receiveErrorHandler_),
    packetAllocationHandler_(eventHandlers.packetAllocationHandler_ ? eventHandlers.packetAllocationHandler_ : [](auto, auto desiredSize){return packet(desiredSize);})  
{
    if (config.receiveBufferSize_ > 0)
        set_socket_option(SOL_SOCKET, SO_RCVBUF, config.receiveBufferSize_);
    if (config.sendBufferSize_ > 0)
        set_socket_option(SOL_SOCKET, SO_SNDBUF, config.sendBufferSize_);
}
catch (std::exception const & exception)
{
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
maniscalco::network::active_socket_impl<P>::socket_impl
(
    // this ctor is for 'accepted' tcp sockets where the socket file
    // descriptor is created prior to the socket_impl
    system::file_descriptor fileDescriptor,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
) requires (tcp_protocol_concept<P>) 
try :
    socket_base_impl({.ioMode_ = config.ioMode_}, eventHandlers, std::move(fileDescriptor),
            workContractGroup.create_contract([this](){this->receive();}, [this](){this->destroy();})),
    pollerRegistration_(p.register_socket(*this)),
    receiveHandler_(eventHandlers.receiveHandler_),
    receiveErrorHandler_(eventHandlers.receiveErrorHandler_),
    packetAllocationHandler_(eventHandlers.packetAllocationHandler_ ? eventHandlers.packetAllocationHandler_ : [](auto, auto desiredSize){return packet(desiredSize);})    
{
    peerIpAddress_ = get_peer_name();
    if (config.receiveBufferSize_ > 0)
        set_socket_option(SOL_SOCKET, SO_RCVBUF, config.receiveBufferSize_);
    if (config.sendBufferSize_ > 0)
        set_socket_option(SOL_SOCKET, SO_SNDBUF, config.sendBufferSize_);
}
catch (std::exception const & exception)
{
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
    peerIpAddress_ = destination;

    set_socket_option(IPPROTO_IP, IP_MULTICAST_TTL, 8);
    return connect_result::success;
}


//=============================================================================
template <>
auto maniscalco::network::tcp_socket_impl::join
(
    network_id
) -> connect_result = delete;


//=============================================================================
template <>
auto maniscalco::network::udp_socket_impl::join
(
    network_id networkId
) -> connect_result
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
    peerIpAddress_ = {networkId, port_id{0}};
    set_io_mode(system::io_mode::read);
    return connect_result::success;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
bool maniscalco::network::active_socket_impl<P>::disconnect
(
)
{
    if ((!is_connected()) ||(!fileDescriptor_.is_valid()))
        return false;

    if constexpr (P == network_transport_protocol::udp)
    {
        if (peerIpAddress_.is_multicast())
        {
            // drop multicast membership
            ::ip_mreq mreq;
            ::memset(&mreq, 0x00, sizeof(mreq));
            mreq.imr_multiaddr = peerIpAddress_.get_network_id();
            mreq.imr_interface = in_addr_any;
            if (!set_socket_option(IPPROTO_IP, IP_DROP_MEMBERSHIP, mreq))
            {
                // TODO: log failure
                return false;
            }
            peerIpAddress_ = {};
            return true;
        }
    }
    return true;
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket_impl<P>::send
(
    std::span<char const> buffer
) -> send_result 
requires (tcp_protocol_concept<P>) 
{
    auto bytesToSend = buffer.size();
    while (!buffer.empty())
    {
        auto result = ::send(fileDescriptor_.get(), buffer.data(), buffer.size(), MSG_NOSIGNAL);
        if (result < 0)
        {
            if (result != EAGAIN)
                return {result, bytesToSend - buffer.size()};
        }
        else
        {
            buffer = buffer.subspan(result);
        }
    }
    return {bytesToSend};
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket_impl<P>::send
(
    std::span<char const> buffer
) -> send_result
requires (udp_protocol_concept<P>) 
{
    return send_to({}, buffer);
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket_impl<P>::send_to
(
    ip_address destinationIpAddress,
    std::span<char const> buffer
) -> send_result
requires (udp_protocol_concept<P>) 
{
    ::sockaddr_in sockAddr = destinationIpAddress;
    sockAddr.sin_family = AF_INET;
    auto p = destinationIpAddress.is_valid() ? reinterpret_cast<sockaddr const *>(&sockAddr) : nullptr;

    while (true)
    {
        auto result = ::sendto(fileDescriptor_.get(), buffer.data(), buffer.size(), MSG_NOSIGNAL, p, (p == nullptr) ? 0 : sizeof(sockAddr));
        if (result < 0)
        {
            if (result != EAGAIN)
                return {result, 0};
        }
        else
        {
            return {buffer.size()};
        }
    }
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
void maniscalco::network::active_socket_impl<P>::receive
(
)
{
    ::sockaddr_in sockAddrIn;
    ::socklen_t addressLength = sizeof(sockAddrIn);

    packet buffer = packetAllocationHandler_(id_, 2048);
    if (auto bytesReceived = ::recvfrom(fileDescriptor_.get(), buffer.data(), buffer.size(), 0, 
            reinterpret_cast<::sockaddr *>(&sockAddrIn), &addressLength); bytesReceived >= 0)
    {
        if constexpr (tcp_protocol_concept<P>)
        {
            if (bytesReceived == 0)        
            {
                // graceful shutdown
                close();
                return;
            }
        }
        buffer.resize(bytesReceived);
        buffer.from_ = sockAddrIn;
        receiveHandler_(id_, std::move(buffer));
        on_polled(); // there could be more ...
    }
    else
    {
        if ((errno != EAGAIN) && (errno != EWOULDBLOCK) && (receiveErrorHandler_))
            receiveErrorHandler_(id_, receive_error{errno});
    }
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
    return (peerIpAddress_.is_valid());
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
auto maniscalco::network::active_socket_impl<P>::get_peer_ip_address
(
) const noexcept -> ip_address
{
    return peerIpAddress_;
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

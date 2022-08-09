#include "./active_socket_impl.h"


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
    socket_base_impl(ipAddress, config, eventHandlers, 
            (P == network_transport_protocol::udp) ? ::socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP) : ::socket(PF_INET, SOCK_STREAM, IPPROTO_TCP),
            workContractGroup.create_contract(
            {
                .contractHandler_ = [this](){this->receive();},
                .endContractHandler_ = [this](){this->destroy();}
            })),
    pollerRegistration_(p.register_socket(*this)),
    receiveHandler_(eventHandlers.receiveHandler_)    
{
}


//=============================================================================
template <maniscalco::network::network_transport_protocol P>
maniscalco::network::active_socket_impl<P>::socket_impl
(
    file_descriptor fileDescriptor,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::work_contract_group & workContractGroup,
    poller & p
):    
    socket_base_impl(config, eventHandlers, std::move(fileDescriptor),
            workContractGroup.create_contract(
            {
                .contractHandler_ = [this](){this->receive();},
                .endContractHandler_ = [this](){this->destroy();}
            })),
    pollerRegistration_(p.register_socket(*this)),
    receiveHandler_(eventHandlers.receiveHandler_)    
{
    connectedIpAddress_ = get_peer_name();
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

    if (destination.is_multicast())
    {
        if constexpr (P != network_transport_protocol::udp)
        {
            // only for udp
            return connect_result::connect_error;
        }
        else
        {
            // join multicast
            ::ip_mreq mreq;
            ::memset(&mreq, 0x00, sizeof(mreq));
            mreq.imr_multiaddr = destination.get_network_id();
            mreq.imr_interface = in_addr_any;
            if (!set_socket_option(IPPROTO_IP, IP_ADD_MEMBERSHIP, mreq))
            {
                ;// TODO : LOG - throw std::runtime_error(fmt::format("udp_socket::join_multicast_group: socket [{}] Failed to join multicast group {}", id_.get(), to_string(destination)));
                return connect_result::connect_error;
            }
            connectedIpAddress_ = destination;
            return connect_result::success;
        }
/*
        ::ip_mreq mreq;
        ::memset(&mreq, 0x00, sizeof(mreq));
        mreq.imr_multiaddr.s_addr = previousConnectedIpAddress.get_network_id();
        mreq.imr_interface.s_addr = endian_swap<std::endian::native, std::endian::big>(INADDR_ANY);
        auto success = set_socket_option(IPPROTO_IP, IP_DROP_MEMBERSHIP, mreq);
*/
    }
    else
    {
        ::sockaddr_in socketAddress = destination;
        socketAddress.sin_family = AF_INET;
        auto result = ::connect(fileDescriptor_.get(), (sockaddr const *)&socketAddress, sizeof(socketAddress));
        if ((result != 0) && (errno != EINPROGRESS))
            return connect_result::connect_error;
        connectedIpAddress_ = destination;
        return connect_result::success;
    }
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
                ;// TODO : LOG - throw std::runtime_error(fmt::format("udp_socket::join_multicast_group: socket [{}] Failed to join multicast group {}", id_.get(), to_string(destination)));
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
        ;//on_send_error(result);
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
    //    std::cout << "received " << buffer.size() << " bytes\n";
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

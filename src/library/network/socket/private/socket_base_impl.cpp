#include "./socket_base_impl.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include <cerrno>


//=============================================================================
maniscalco::network::socket_base_impl::socket_base_impl
(
    ip_address ipAddress,
    configuration const & config,
    event_handlers const & eventHandlers,
    file_descriptor fileDescriptor,
    system::work_contract workContract
) noexcept :
    fileDescriptor_(std::move(fileDescriptor)),
    eventHandlers_(eventHandlers),
    workContract_(std::move(workContract))
{
    if (!set_socket_option(SOL_SOCKET, SO_REUSEADDR, 1))
        ;//glimpse_warning << "socket::socket: failed to set sock option SO_REUSEADDR";
    ipAddress_ = get_socket_name();
    auto bindResult = bind(ipAddress);
    switch (bindResult)
    {
        case bind_result::success:
        {
            break;
        }
        case bind_result::undefined:
        case bind_result::bind_error:
        case bind_result::invalid_file_descriptor:
        {
            // log here
            fileDescriptor_ = {};
            break;
        }
    }
    set_synchronicity(config.synchronicityMode_);
}


//=============================================================================
maniscalco::network::socket_base_impl::socket_base_impl
(
    configuration const & config,
    event_handlers const & eventHandlers,
    file_descriptor fileDescriptor,
    system::work_contract workContract
) noexcept :
    fileDescriptor_(std::move(fileDescriptor)),
    eventHandlers_(eventHandlers),
    workContract_(std::move(workContract))
{
    if (!set_socket_option(SOL_SOCKET, SO_REUSEADDR, 1))
        ;//glimpse_warning << "socket::socket: failed to set sock option SO_REUSEADDR";
    ipAddress_ = get_socket_name();
    set_synchronicity(config.synchronicityMode_);
}


//=============================================================================
maniscalco::network::socket_base_impl::~socket_base_impl
(
)
{
    close();
}


//=============================================================================
void maniscalco::network::socket_base_impl::on_polled
(
)
{
    workContract_.invoke();
}


//=============================================================================
template <typename T>
bool maniscalco::network::socket_base_impl::set_socket_option
(
    std::int32_t level,
    std::int32_t optionName,
    T optionValue
) noexcept
{
    return (::setsockopt(fileDescriptor_.get(), level, optionName, &optionValue, sizeof(optionValue)) == 0);
}


//=============================================================================
auto maniscalco::network::socket_base_impl::get_socket_name
(
) const noexcept -> ip_address
{
    ::sockaddr_in socketAddress;
    ::socklen_t sizeofSocketAddress(sizeof(socketAddress));
    if (::getsockname(fileDescriptor_.get(), (struct sockaddr *)&socketAddress, &sizeofSocketAddress) == 0)
        return socketAddress;
    return {};
}


//=============================================================================
auto maniscalco::network::socket_base_impl::bind
(
    ip_address const & ipAddress
) noexcept -> bind_result
{
    if (!fileDescriptor_.is_valid())
        return bind_result::invalid_file_descriptor;

    ::sockaddr_in socketAddress = ipAddress;
    socketAddress.sin_family = AF_INET;
    auto bindResult = ::bind(fileDescriptor_.get(), (sockaddr const *)&socketAddress, sizeof(socketAddress));
    if (bindResult == -1)
        return bind_result::bind_error;
    ipAddress_ = get_socket_name();
    return bind_result::success;
}


//=============================================================================
bool maniscalco::network::socket_base_impl::close
(
)
{
    if (fileDescriptor_.close())
    {
        if (eventHandlers_.closeHandler_)
            eventHandlers_.closeHandler_(id_);
        ipAddress_ = {};
        return true;
    }
    return false;
}


//=============================================================================
bool maniscalco::network::socket_base_impl::is_connected
(
) const noexcept
{
    return (connectedIpAddress_.is_valid());
}


//=============================================================================
bool maniscalco::network::socket_base_impl::is_valid
(
) const noexcept
{
    return (fileDescriptor_.is_valid());
}


//=============================================================================
auto maniscalco::network::socket_base_impl::get_file_descriptor
(
) const noexcept -> file_descriptor const & 
{
    return fileDescriptor_;
}


//=============================================================================
auto maniscalco::network::socket_base_impl::get_ip_address
(
) const noexcept -> ip_address
{
    return ipAddress_;
}


//=============================================================================
auto maniscalco::network::socket_base_impl::get_connected_ip_address
(
) const noexcept -> ip_address
{
    return connectedIpAddress_;
}


//=============================================================================
auto maniscalco::network::socket_base_impl::get_peer_name
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
std::vector<std::uint8_t> maniscalco::network::socket_base_impl::receive
(
)
{
    // TODO: get from allocator
    std::vector<std::uint8_t> buffer(2048);

    auto result = ::recv(fileDescriptor_.get(), buffer.data(), buffer.capacity(), 0);
    if (result > 0)
    {
        buffer.resize(result);
        eventHandlers_.receiveHandler_(id_, std::move(buffer));
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
auto maniscalco::network::socket_base_impl::get_id
(
) const noexcept -> socket_id
{
    return id_;
}


//=============================================================================
bool maniscalco::network::socket_base_impl::set_synchronicity
(
    synchronicity_mode mode
)
{
    auto flags = ::fcntl(fileDescriptor_.get(), F_GETFL, 0);
    if (flags == -1)
        return false;
    if (mode == synchronicity_mode::blocking)
    {
        // synchronous/blocking mode
        flags &= ~O_NONBLOCK;
    }
    else
    {
        // asynchronous/non-blocking
        flags |= O_NONBLOCK;
    }
    auto fcntlResult = ::fcntl(fileDescriptor_.get(), F_SETFL, flags);
    if (fcntlResult != 0)
        return false;
    return true; 
}

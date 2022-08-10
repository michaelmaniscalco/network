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
    system::file_descriptor fileDescriptor,
    system::work_contract workContract
) noexcept :
    fileDescriptor_(std::move(fileDescriptor)),
    closeHandler_(eventHandlers.closeHandler_),
    workContract_(std::move(workContract))
{
    if (!set_socket_option(SOL_SOCKET, SO_REUSEADDR, 1))
        ;// TODO: log failure
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
    system::file_descriptor fileDescriptor,
    system::work_contract workContract
) noexcept :
    fileDescriptor_(std::move(fileDescriptor)),
    closeHandler_(eventHandlers.closeHandler_),
    workContract_(std::move(workContract))
{
    if (!set_socket_option(SOL_SOCKET, SO_REUSEADDR, 1))
        ;// TODO: log failure
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
        if (closeHandler_)
            closeHandler_(id_);
        ipAddress_ = {};
        return true;
    }
    return false;
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
) const noexcept -> system::file_descriptor const & 
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
auto maniscalco::network::socket_base_impl::get_id
(
) const noexcept -> socket_id
{
    return id_;
}


//=============================================================================
bool maniscalco::network::socket_base_impl::set_synchronicity
(
    system::synchronicity_mode mode
)
{
    auto flags = ::fcntl(fileDescriptor_.get(), F_GETFL, 0);
    if (flags == -1)
        return false;
    if (mode == system::synchronicity_mode::blocking)
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

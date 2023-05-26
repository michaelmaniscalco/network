#include "./socket_base.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include <cerrno>


//=============================================================================
maniscalco::network::socket_base::socket_base
(
    ip_address ipAddress,
    configuration const & config,
    system::file_descriptor fileDescriptor,
    system::work_contract workContract
) noexcept :
    fileDescriptor_(std::move(fileDescriptor)),
    workContract_(std::move(workContract))
{
    if (!set_socket_option(SOL_SOCKET, SO_REUSEADDR, 1))
        ;// TODO: log failure
    if (!ipAddress.is_multicast())
    {
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
        ipAddress_ = get_socket_name();
    }
    set_synchronicity(system::synchronization_mode::non_blocking);
    set_io_mode(config.ioMode_);
}


//=============================================================================
maniscalco::network::socket_base::socket_base
(
    configuration const & config,
    system::file_descriptor fileDescriptor,
    system::work_contract workContract
) noexcept :
    fileDescriptor_(std::move(fileDescriptor)),
    workContract_(std::move(workContract))
{
    if (!set_socket_option(SOL_SOCKET, SO_REUSEADDR, 1))
        ;// TODO: log failure
    ipAddress_ = get_socket_name();
    set_synchronicity(system::synchronization_mode::non_blocking);
    set_io_mode(config.ioMode_);
}


//=============================================================================
void maniscalco::network::socket_base::on_polled
(
)
{
    workContract_.invoke();
}


//=============================================================================
auto maniscalco::network::socket_base::get_socket_name
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
auto maniscalco::network::socket_base::bind
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
bool maniscalco::network::socket_base::is_valid
(
) const noexcept
{
    return (fileDescriptor_.is_valid());
}


//=============================================================================
auto maniscalco::network::socket_base::get_file_descriptor
(
) const noexcept -> system::file_descriptor const & 
{
    return fileDescriptor_;
}


//=============================================================================
auto maniscalco::network::socket_base::get_ip_address
(
) const noexcept -> ip_address
{
    return ipAddress_;
}


//=============================================================================
auto maniscalco::network::socket_base::get_id
(
) const noexcept -> socket_id
{
    return id_;
}


//=============================================================================
bool maniscalco::network::socket_base::set_synchronicity
(
    system::synchronization_mode mode
) noexcept
{
    return false;
    auto flags = ::fcntl(fileDescriptor_.get(), F_GETFL, 0);
    if (flags == -1)
        return false;
    if (mode == system::synchronization_mode::blocking)
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


//=============================================================================
bool maniscalco::network::socket_base::shutdown
(
) noexcept
{
    return set_io_mode(system::io_mode::none);
}


//=============================================================================
bool maniscalco::network::socket_base::set_io_mode
(
    system::io_mode ioMode
) noexcept
{
    switch (ioMode)
    {
        case system::io_mode::read:
        {
            return shutdown(system::io_mode::write);
        }
        case system::io_mode::write:
        {
            return shutdown(system::io_mode::read);
        }
        case system::io_mode::read_write:
        {
            return shutdown(system::io_mode::none);
        }
        case system::io_mode::none:
        {
            return shutdown(system::io_mode::read_write);
        }
        default:
        {
            return true;
        }
    }
}


//=============================================================================
bool maniscalco::network::socket_base::shutdown
(
    system::io_mode ioMode
) noexcept
{
    switch (ioMode)
    {
        case system::io_mode::read:
        {
            return (::shutdown(fileDescriptor_.get(), SHUT_RD) == 0);
        }
        case system::io_mode::write:
        {
            return (::shutdown(fileDescriptor_.get(), SHUT_WR) == 0);
        }
        case system::io_mode::read_write:
        {
            return (::shutdown(fileDescriptor_.get(), SHUT_RDWR) == 0);
        }
        case system::io_mode::none:
        default:
        {
            return true;
        }
    }
}

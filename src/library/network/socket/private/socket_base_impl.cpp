#include "./socket_base_impl.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include <cerrno>

#include <cstdint>
#include <string_view>


namespace maniscalco::network
{

    enum class bind_result : std::uint32_t
    {
        undefined               = 0,
        success                 = 1,
        bind_error              = 2,
        invalid_file_descriptor = 3
    };


    //=========================================================================
    [[maybe_unused]] static std::string_view const to_string
    (
        bind_result connectResult
    )
    {
        using namespace std::string_literals;
        
        static auto constexpr undefined = "undefined";
        static auto constexpr success = "success";
        static auto constexpr invalid_file_descriptor = "invalid_file_descriptor";
        static auto constexpr bind_error = "bind_error";

        switch (connectResult)
        {
            case bind_result::success: return success;
            case bind_result::invalid_file_descriptor: return invalid_file_descriptor;
            case bind_result::bind_error: return bind_error;
            case bind_result::undefined: 
            default:
            {
                return undefined;
            }
        }
    }

}


//=============================================================================
maniscalco::network::socket_base_impl::socket_base_impl
(
    ip_address ipAddress,
    configuration const & config,
    event_handlers const & eventHandlers,
    system::file_descriptor fileDescriptor,
    system::work_contract workContract
) try :
    fileDescriptor_(std::move(fileDescriptor)),
    closeHandler_(eventHandlers.closeHandler_),
    pollErrorHandler_(eventHandlers.pollErrorHandler_),
    workContract_(std::move(workContract))
{
    if (auto success = set_socket_option(SOL_SOCKET, SO_REUSEADDR, 1); !success)
        throw std::runtime_error("socket_base_impl::set reuse address failure");
    if (!ipAddress.is_multicast())
    {
        auto bindResult = bind(ipAddress);
        switch (bindResult)
        {
            case bind_result::success:
            {
                break;
            }
            default:
            case bind_result::undefined:
            case bind_result::bind_error:
            case bind_result::invalid_file_descriptor:
            {
                throw std::runtime_error("socket_base_impl::bind error");
            }
        }
        ipAddress_ = get_socket_name();
    }
    if (auto success = set_synchronicity(system::synchronization_mode::non_blocking); !success)
        throw std::runtime_error("socket_base_impl::set_synchronicity: failure");
    if (auto success = set_io_mode(config.ioMode_); !success)
        throw std::runtime_error("socket_base_impl::set_io_mode: failure");
}
catch (std::exception const & exception)
{
    fileDescriptor_ = {};
    ipAddress_ = {};
}


//=============================================================================
maniscalco::network::socket_base_impl::socket_base_impl
(
    configuration const & config,
    event_handlers const & eventHandlers,
    system::file_descriptor fileDescriptor,
    system::work_contract workContract
) try :
    fileDescriptor_(std::move(fileDescriptor)),
    closeHandler_(eventHandlers.closeHandler_),
    workContract_(std::move(workContract))
{
    if (auto success = set_socket_option(SOL_SOCKET, SO_REUSEADDR, 1); !success)
        throw std::runtime_error("socket_base_impl::set reuse address failure");
    if (auto success = set_synchronicity(system::synchronization_mode::non_blocking); !success)
        throw std::runtime_error("socket_base_impl::set_synchronicity: failure");
    if (auto success = set_io_mode(config.ioMode_); !success)
        throw std::runtime_error("socket_base_impl::set_io_mode: failure");
    ipAddress_ = get_socket_name();
}
catch (std::exception const &)
{
    auto exception = std::current_exception();
    fileDescriptor_ = {};
    ipAddress_ = {};
    std::rethrow_exception(exception);
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
void maniscalco::network::socket_base_impl::on_poll_error
(
)
{
    if (pollErrorHandler_)
        pollErrorHandler_(id_);
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
    system::synchronization_mode mode
) noexcept
{
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
bool maniscalco::network::socket_base_impl::shutdown
(
) noexcept
{
    return set_io_mode(system::io_mode::none);
}


//=============================================================================
bool maniscalco::network::socket_base_impl::set_io_mode
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
bool maniscalco::network::socket_base_impl::shutdown
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

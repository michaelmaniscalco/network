#pragma once

#include <library/network/socket/socket_id.h>

#include <library/network/socket/return_code/connect_result.h>
#include <library/network/socket/return_code/bind_result.h>

#include <library/network/ip/ip_address.h>
#include <include/file_descriptor.h>
#include <include/io_mode.h>
#include <include/synchronicity_mode.h>

#include <library/system.h>

#include <functional>


namespace maniscalco::network
{

    //=========================================================================
    class socket_base_impl
    {
    public:

        struct event_handlers
        {
            using close_handler = std::function<void(socket_id)>;

            close_handler   closeHandler_;
        };

        struct configuration
        {
            system::io_mode ioMode_{system::io_mode::read_write};
        };

        socket_base_impl
        (
            configuration const &,
            event_handlers const &,
            system::file_descriptor,
            system::work_contract
        ) noexcept;

        socket_base_impl
        (
            ip_address,
            configuration const &,
            event_handlers const &,
            system::file_descriptor,
            system::work_contract
        ) noexcept;

        virtual ~socket_base_impl();

        bool close();

        bool is_valid() const noexcept;

        system::file_descriptor const & get_file_descriptor() const noexcept;
        
        ip_address get_ip_address() const noexcept;

        socket_id get_id() const noexcept;

        bool set_io_mode
        (
            system::io_mode
        ) noexcept;

    protected:

        // unfortunate
        friend class poller;

        bool set_synchronicity
        (
            system::synchronicity_mode
        ) noexcept;

        bool shutdown
        (
            system::io_mode
        ) noexcept;

        void on_polled();

        template <typename T>
        bool set_socket_option
        (
            std::int32_t,
            std::int32_t,
            T
        ) noexcept;

        bind_result bind
        (
            ip_address const &
        ) noexcept;

        ip_address get_socket_name() const noexcept;

        system::file_descriptor                 fileDescriptor_;

        ip_address                      ipAddress_;

        socket_id                       id_;

        event_handlers::close_handler   closeHandler_;

        system::work_contract           workContract_;

    }; // class socket_base_impl

} // namespace maniscalco::network


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

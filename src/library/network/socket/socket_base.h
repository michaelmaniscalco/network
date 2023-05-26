#pragma once

#include <library/network/socket/socket_id.h>

#include <library/network/socket/return_code/connect_result.h>
#include <library/network/socket/return_code/bind_result.h>

#include <library/network/ip/ip_address.h>
#include <include/file_descriptor.h>
#include <include/io_mode.h>
#include <include/synchronization_mode.h>

#include <library/system.h>

#include <include/non_copyable.h>
#include <include/non_movable.h>

#include <functional>


namespace maniscalco::network
{

    //=========================================================================
    class socket_base :
        public non_copyable,
        public non_movable
    {
    public:

        struct configuration
        {
            system::io_mode ioMode_{system::io_mode::read_write};
        };

        socket_base
        (
            configuration const &,
            system::file_descriptor,
            system::work_contract
        ) noexcept;

        socket_base
        (
            ip_address,
            configuration const &,
            system::file_descriptor,
            system::work_contract
        ) noexcept;

        virtual ~socket_base() = default;

        bool is_valid() const noexcept;

        system::file_descriptor const & get_file_descriptor() const noexcept;
        
        ip_address get_ip_address() const noexcept;

        socket_id get_id() const noexcept;

        bool shutdown() noexcept;

        bool set_io_mode
        (
            system::io_mode
        ) noexcept;

    protected:

        // unfortunate
        friend class poller;

        bool set_synchronicity
        (
            system::synchronization_mode
        ) noexcept;

        bool shutdown
        (
            system::io_mode
        ) noexcept;

        void on_polled();

        virtual void on_poll_error() = 0;

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

        system::file_descriptor             fileDescriptor_;
        ip_address                          ipAddress_;
        socket_id                           id_;
        system::work_contract               workContract_;
    }; // class socket_base

} // namespace maniscalco::network


//=============================================================================
template <typename T>
bool maniscalco::network::socket_base::set_socket_option
(
    std::int32_t level,
    std::int32_t optionName,
    T optionValue
) noexcept
{
    return (::setsockopt(fileDescriptor_.get(), level, optionName, &optionValue, sizeof(optionValue)) == 0);
}

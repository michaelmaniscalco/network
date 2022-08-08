#pragma once

#include <library/network/socket/socket_id.h>

#include <library/network/socket/return_code/connect_result.h>
#include <library/network/socket/return_code/bind_result.h>

#include <library/network/ip/ip_address.h>
#include <include/file_descriptor.h>
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
            using receive_handler = std::function<void(socket_id, std::vector<std::uint8_t>)>;

            close_handler   closeHandler_;
            receive_handler receiveHandler_;
        };

        struct configuration
        {
            synchronicity_mode synchronicityMode_{synchronicity_mode::non_blocking};
        };

        socket_base_impl
        (
            configuration const &,
            event_handlers const &,
            file_descriptor,
            system::work_contract
        ) noexcept;

        socket_base_impl
        (
            ip_address,
            configuration const &,
            event_handlers const &,
            file_descriptor,
            system::work_contract
        ) noexcept;

        virtual ~socket_base_impl();

        bool close();

        bool is_valid() const noexcept;

        file_descriptor const & get_file_descriptor() const noexcept;
        
        ip_address get_ip_address() const noexcept;

        bool is_connected() const noexcept;

        ip_address get_connected_ip_address() const noexcept;

        std::vector<std::uint8_t> receive();

        socket_id get_id() const noexcept;

 //   protected:
        bool set_synchronicity
        (
            synchronicity_mode
        );

        void on_polled();

        template <typename T>
        bool set_socket_option
        (
            std::int32_t,
            std::int32_t,
            T
        ) noexcept;

        [[nodiscard]] bind_result bind
        (
            ip_address const &
        ) noexcept;

        ip_address get_socket_name() const noexcept;

        ip_address get_peer_name() const noexcept;

        file_descriptor                 fileDescriptor_;

        ip_address                      ipAddress_;

        ip_address                      connectedIpAddress_;

        socket_id                       id_;

  //  private:

        event_handlers                  eventHandlers_;

        system::work_contract           workContract_;

    }; // class socket_base_impl

} // namespace maniscalco::network

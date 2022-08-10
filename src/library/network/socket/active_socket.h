#pragma once

#include "./socket.h"
#include "./traits/traits.h"
#include "./return_code/connect_result.h"
#include "./return_code/bind_result.h"
#include <include/file_descriptor.h>
#include <library/network/ip/ip_address.h>

#include <library/system.h>

#include <functional>
#include <type_traits>
#include <span>


namespace maniscalco::network
{

    class poller;


    //=========================================================================
    template <network_transport_protocol P>
    class socket<active_socket_traits<P>>
    {
    public:

        using traits = active_socket_traits<P>; 

        struct event_handlers
        {
            using close_handler = std::function<void(socket_id)>;
            using receive_handler = std::function<void(socket_id, std::vector<std::uint8_t>)>;

            close_handler   closeHandler_;
            receive_handler receiveHandler_;
        };

        struct configuration
        {
        };


        socket(socket const &) = delete;
        socket & operator = (socket const &) = delete;
        
        socket() = default;
        socket(socket &&) = default;
        socket & operator = (socket &&) = default;

        socket
        (
            ip_address,
            configuration const &,
            event_handlers const &,
            system::work_contract_group &,
            poller &
        );

        socket
        (
            system::file_descriptor,
            configuration const &,
            event_handlers const &,
            system::work_contract_group &,
            poller &
        );

        virtual ~socket();

        std::span<char const> send
        (
            std::span<char const>
        );

        connect_result connect_to
        (
            ip_address const &
        ) noexcept;

        bool close();

        bool is_valid() const noexcept;

        ip_address get_ip_address() const noexcept;

        bool is_connected() const noexcept;

        ip_address get_connected_ip_address() const noexcept;

        std::vector<std::uint8_t> receive();

        socket_id get_id() const;
        
    private:

        using impl_type = socket_impl<traits>;

        std::unique_ptr<impl_type, std::function<void(impl_type *)>>   impl_;

    }; // class socket<active_socket_traits<P>>


    template <network_transport_protocol T>
    using active_socket = socket<active_socket_traits<T>>;

    using udp_socket = active_socket<network_transport_protocol::udp>;
    using tcp_socket = active_socket<network_transport_protocol::tcp>;

} // namespace maniscalco::network

#pragma once

#include "./socket.h"
#include "./traits/traits.h"
#include "./return_code/connect_result.h"
#include "./return_code/bind_result.h"
#include "./return_code/receive_result.h"
#include "./return_code/send_result.h"

#include <library/network/ip/ip_address.h>
#include <library/network/packet/packet.h>

#include <include/file_descriptor.h>
#include <include/io_mode.h>
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
            using poll_error_handler = std::function<void(socket_id)>;
            using receive_handler = std::function<void(socket_id)>;
            using receive_error_handler = std::function<void(socket_id, receive_error)>;

            close_handler               closeHandler_;
            poll_error_handler          pollErrorHandler_;
            receive_handler             receiveHandler_;
            receive_error_handler       receiveErrorHandler_;
        };

        struct configuration
        {
            std::size_t receiveBufferSize_{0};
            std::size_t sendBufferSize_{0};
            system::io_mode ioMode_{system::io_mode::read_write};
        };

        socket() = default;
        socket(socket &&) = default;
        socket & operator = (socket &&) = default;

        socket(socket const &) = delete;
        socket & operator = (socket const &) = delete;

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
        ) requires (tcp_protocol_concept<P>);

        ~socket() = default;

        send_result send
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

        ip_address get_peer_ip_address() const noexcept;

        socket_id get_id() const;
        
        connect_result join
        (
            network_id
        ) requires (P == network_transport_protocol::udp);

        bool set_read_only() noexcept;

        bool set_write_only() noexcept;

        bool set_read_write() noexcept;

        bool set_io_mode
        (
            system::io_mode
        ) noexcept;

    private:

        using impl_type = socket_impl<traits>;

        std::unique_ptr<impl_type, std::function<void(impl_type *)>>   impl_;

    }; // class socket<active_socket_traits<P>>


    template <network_transport_protocol T>
    using active_socket = socket<active_socket_traits<T>>;

    using udp_socket = active_socket<network_transport_protocol::udp>;
    using tcp_socket = active_socket<network_transport_protocol::tcp>;

} // namespace maniscalco::network

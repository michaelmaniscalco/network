#pragma once

#include <library/network/socket/socket.h>
#include <library/network/polling/poller.h>

#include "./socket_base_impl.h"

#include <include/io_mode.h>
#include <library/system.h>

#include <functional>
#include <type_traits>
#include <span>


namespace maniscalco::network
{

    template <network_transport_protocol P>
    class socket_impl<socket_traits<P, socket_type::active>> :
        public socket_base_impl
    {
    public:

        using traits = socket_traits<P, socket_type::active>;

        struct event_handlers : socket_base_impl::event_handlers
        {
            using receive_handler = std::function<void(socket_id, std::vector<std::uint8_t>)>;
            receive_handler receiveHandler_;
        };

        struct configuration
        {
            std::size_t     receiveBufferSize_{0};
            std::size_t     sendBufferSize_{0};
            system::io_mode ioMode_{system::io_mode::read_write};
        };

        socket_impl
        (
            ip_address,
            configuration const &,
            event_handlers const &,
            system::work_contract_group &,
            poller &
        );

        socket_impl
        (
            system::file_descriptor,
            configuration const &,
            event_handlers const &,
            system::work_contract_group &,
            poller &
        );

        std::span<char const> send
        (
            std::span<char const>
        );

        connect_result connect_to
        (
            ip_address const &
        ) noexcept;

        std::vector<std::uint8_t> receive();

        void destroy();

        bool is_connected() const noexcept;

        ip_address get_connected_ip_address() const noexcept;

        connect_result join
        (
            network_id
        );

    private:

        ip_address get_peer_name() const noexcept;

        bool disconnect();

        ip_address                      connectedIpAddress_;

        poller_registration             pollerRegistration_;

        typename event_handlers::receive_handler receiveHandler_;

    }; // class socket_impl<socket_traits<P, socket_type::active>>


    template <network_transport_protocol P>
    using active_socket_impl = socket_impl<socket_traits<P, socket_type::active>>;


    using tcp_socket_impl = active_socket_impl<network_transport_protocol::tcp>;
    using udp_socket_impl = active_socket_impl<network_transport_protocol::udp>;

} // namespace maniscalco::network

#pragma once

#include <library/network/polling/poller.h>
#include <library/network/socket/tcp_socket.h>
#include <library/network/socket/udp_socket.h>
#include <library/network/socket/tcp_listener_socket.h>

#include <library/system.h>


namespace maniscalco::network
{

    class network_interface
    {
    public:

        struct configuration
        {
            poller::configuration poller_;
        };

        network_interface
        (
            configuration const &,
            std::shared_ptr<system::work_contract_group>
        );

        ~network_interface() = default;

        tcp_listener_socket open_socket
        (
            ip_address,
            tcp_listener_socket::configuration,
            tcp_listener_socket::event_handlers
        );

        udp_socket open_socket
        (
            ip_address,
            udp_socket::configuration,
            udp_socket::event_handlers
        );

        tcp_socket open_socket
        (
            ip_address,
            tcp_socket::configuration,
            tcp_socket::event_handlers
        );

        tcp_socket open_socket
        (
            file_descriptor,
            tcp_socket::configuration,
            tcp_socket::event_handlers
        );

        void poll();

    private:

        std::shared_ptr<poller> poller_;

        std::shared_ptr<system::work_contract_group>     workContractGroup_;

    }; // class network_interface

} // namespace maniscalco::network

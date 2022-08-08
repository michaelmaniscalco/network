#pragma once

#include <library/network/socket/socket.h>
#include <library/network/polling/poller.h>

#include "./socket_base_impl.h"

#include <library/system.h>

#include <functional>
#include <type_traits>
#include <span>


namespace maniscalco::network
{

    template <>
    class socket_impl<tcp_listener_socket_traits> :
        public socket_base_impl
    {
    public:

        using traits = tcp_listener_socket_traits;

        struct event_handlers : socket_base_impl::event_handlers
        {
            using accept_handler = std::function<void(socket_id, file_descriptor)>;

            accept_handler acceptHandler_;
        };

        struct configuration : socket_base_impl::configuration
        {
            std::uint32_t backlog_;
        };

        void destroy();

        void on_selected();

//    private:

//        friend class network_interface;

        socket_impl
        (
            ip_address,
            configuration const &,
            event_handlers const &,
            system::work_contract_group &,
            poller &
        );

        void accept();

        poller_registration    pollerRegistration_;

        event_handlers::accept_handler acceptHandler_;
    };

    using tcp_listener_socket_impl = socket_impl<tcp_listener_socket_traits>;

}

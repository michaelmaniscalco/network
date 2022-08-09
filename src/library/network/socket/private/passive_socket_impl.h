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

    template <network_transport_protocol P>
    class socket_impl<socket_traits<P, socket_type::passive>> :
        public socket_base_impl
    {
    public:

        using traits = socket_traits<P, socket_type::passive>;

        struct event_handlers : socket_base_impl::event_handlers
        {
            using accept_handler = std::function<void(socket_id, file_descriptor)>;

            accept_handler acceptHandler_;
        };

        struct configuration : socket_base_impl::configuration
        {
            std::uint32_t backlog_;
        };

        socket_impl
        (
            ip_address,
            configuration const &,
            event_handlers const &,
            system::work_contract_group &,
            poller &
        );

        void destroy();

    private:

        void accept();

        poller_registration    pollerRegistration_;

        event_handlers::accept_handler acceptHandler_;

    }; // namespace socket_impl<passive_socket_traits_concept> 


    template <network_transport_protocol P>
    using passive_socket_impl = socket_impl<socket_traits<P, socket_type::passive>>;


    using tcp_listener_socket_impl = passive_socket_impl<network_transport_protocol::tcp>;

} // namespace maniscalco::network

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
    class socket_impl<udp_socket_traits> :
        public socket_base_impl
    {
    public:

        using traits = udp_socket_traits;

        std::span<char const> send
        (
            std::span<char const>
        );

        // TODO
        // send_to
        // recv
        // recv_from

        [[nodiscard]] connect_result connect_to
        (
            ip_address const &
        ) noexcept;

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

        poller_registration    pollerRegistration_;
    };

    using udp_socket_impl = socket_impl<udp_socket_traits>;

}

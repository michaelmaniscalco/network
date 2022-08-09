#pragma once

#include <library/network/polling/poller.h>
#include <library/network/socket/active_socket.h>
#include <library/network/socket/passive_socket.h>

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

        template <socket_concept P, typename T>
        P open_socket
        (
            T,
            typename P::configuration,
            typename P::event_handlers
        );

        void poll();

    private:

        std::shared_ptr<poller> poller_;

        std::shared_ptr<system::work_contract_group>     workContractGroup_;

    }; // class network_interface

} // namespace maniscalco::network

#pragma once

#include <library/network/polling/poller.h>
#include <library/network/socket/active_socket.h>
#include <library/network/socket/passive_socket.h>
#include <library/network/stream/stream.h>

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

        template <socket_concept P, typename T, typename B = default_buffer_type>
        stream<P> open_stream
        (
            T && socketHandle,
            typename P::configuration config,
            typename P::event_handlers eventHandlers
        )
        {
            return stream<P, B>(open_socket<P>(socketHandle, config, eventHandlers), *workContractGroup_);
        }

        void poll();

    private:

        std::shared_ptr<poller>                         poller_;
        std::shared_ptr<system::work_contract_group>    workContractGroup_;

    }; // class network_interface

} // namespace maniscalco::network

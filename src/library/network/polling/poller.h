#pragma once

#include "./poller_registration.h"
#include <include/file_descriptor.h>

#include <library/network/socket/socket.h>

#include <mutex>
#include <vector>
#include <memory>


namespace maniscalco::network
{

    class poller :
        public std::enable_shared_from_this<poller>
    {
    public:

        enum class trigger_type : std::uint32_t
        {
            edge_triggered,
            level_triggered
        };

        struct configuration
        {
            trigger_type trigger_{trigger_type::edge_triggered};
        };

        poller
        (
            configuration const &
        );

        ~poller();

        template <socket_impl_concept S>
        poller_registration register_socket
        (
            S &
        );

        bool unregister_socket
        (
            system::file_descriptor const &
        );

        void poll();
        
        void close();

    private:

        system::file_descriptor fileDescriptor_;

        trigger_type            trigger_;

    }; // class poller

} // namespace maniscalco::network

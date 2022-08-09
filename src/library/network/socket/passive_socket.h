#pragma once

#include "./socket.h"
#include "./traits/traits.h"
#include "./return_code/connect_result.h"
#include "./return_code/bind_result.h"
#include <include/file_descriptor.h>
#include <library/network/ip/ip_address.h>
#include <library/network/packet/packet.h>

#include <library/system.h>

#include <functional>
#include <type_traits>
#include <span>


namespace maniscalco::network
{

    class poller;


    //=========================================================================
    template <network_transport_protocol P>
    class socket<passive_socket_traits<P>>
    {
    public:

        using traits = passive_socket_traits<P>; 

        static auto constexpr default_backlog{128};

        struct event_handlers
        {
            using close_handler = std::function<void(socket_id)>;
            using accept_handler = std::function<void(socket_id, file_descriptor)>;

            close_handler   closeHandler_;
            accept_handler  acceptHandler_;
        };

        struct configuration
        {
            std::uint32_t backlog_{default_backlog};
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

        virtual ~socket();

        bool close();

        bool is_valid() const noexcept;

        ip_address get_ip_address() const noexcept;

        socket_id get_id() const;
        
    private:

        using impl_type = socket_impl<traits>;

        std::unique_ptr<impl_type, std::function<void(impl_type *)>>   impl_;

    }; // class socket<passive_socket_traits<P>>


    template <network_transport_protocol T>
    using passive_socket = socket<passive_socket_traits<T>>;


    using tcp_listener_socket = passive_socket<network_transport_protocol::tcp>;

} // namespace maniscalco::network

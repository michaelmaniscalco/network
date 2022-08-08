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
    template <>
    class socket<tcp_listener_socket_traits>
    {
    public:

        using traits = tcp_listener_socket_traits; 

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

        friend class poller;

        std::unique_ptr<socket_impl<traits>, std::function<void(socket_impl<traits> *)>>   impl_;

    }; // class socket<tcp_listener_socket_traits>


    //=========================================================================
    template <socket_concept T> 
    static bool constexpr is_tcp_listener_socket_v = std::is_same_v<typename T::traits, tcp_listener_socket_traits>;

    using tcp_listener_socket = socket<tcp_listener_socket_traits>;

} // namespace maniscalco::network

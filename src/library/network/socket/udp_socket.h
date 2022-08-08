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
    class socket<udp_socket_traits>
    {
    public:

        using traits = udp_socket_traits; 

        struct event_handlers
        {
            using close_handler = std::function<void(socket_id)>;
            using receive_handler = std::function<void(socket_id, std::vector<std::uint8_t>)>;

            close_handler   closeHandler_;
            receive_handler receiveHandler_;
        };

        struct configuration
        {
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

        std::span<char const> send
        (
            std::span<char const>
        );

        // TODO
        // send_to
        // recv
        // recv_from

        connect_result connect_to
        (
            ip_address const &
        ) noexcept;

        bool close();

        bool is_valid() const noexcept;

        ip_address get_ip_address() const noexcept;

        bool is_connected() const noexcept;

        ip_address get_connected_ip_address() const noexcept;

        std::vector<std::uint8_t> receive();

        socket_id get_id() const;
        
    private:

        friend class poller;

        std::unique_ptr<socket_impl<traits>, std::function<void(socket_impl<traits> *)>>   impl_;

    }; // class socket<udp_socket_traits>


    //=========================================================================
    template <socket_concept T> 
    static bool constexpr is_udp_socket_v = std::is_same_v<typename T::traits, udp_socket_traits>;

    using udp_socket = socket<udp_socket_traits>;

} // namespace maniscalco::network

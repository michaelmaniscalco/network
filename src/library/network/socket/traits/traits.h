#pragma once

#include "./network_transport_protocol.h"
#include "./socket_type.h"

#include <concepts>
#include <type_traits>


namespace maniscalco::network
{

    //=========================================================================
    template <network_transport_protocol T0, socket_type T1>
    struct socket_traits
    {
        static auto constexpr protocol = T0;
        static auto constexpr type = T1;
    };


    using tcp_socket_traits = socket_traits<network_transport_protocol::tcp, socket_type::active>;
    using tcp_listener_socket_traits = socket_traits<network_transport_protocol::tcp, socket_type::passive>;
    using udp_socket_traits = socket_traits<network_transport_protocol::udp, socket_type::active>;
    

    //=========================================================================
    template <typename T>
    concept socket_traits_concept = std::is_same_v<T, socket_traits<T::protocol, T::type>>;


    //=========================================================================
    template <typename T>
    concept active_socket_traits_concept = std::is_same_v<T, socket_traits<T::protocol, socket_type::active>>;

    template <network_transport_protocol T>
    using active_socket_traits = socket_traits<T, socket_type::active>;


    //=========================================================================
    template <typename T>
    concept passive_socket_traits_concept = std::is_same_v<T, socket_traits<T::protocol, socket_type::passive>>;


    template <network_transport_protocol T>
    using passive_socket_traits = socket_traits<T, socket_type::passive>;

} // namespace maniscalco::network

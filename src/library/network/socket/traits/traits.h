#pragma once

#include "./socket_type.h"

#include <concepts>
#include <type_traits>


namespace maniscalco::network
{

    //=========================================================================
    template <socket_type T>
    struct socket_traits
    {
        static auto constexpr type = T;
    };


    using tcp_socket_traits = socket_traits<socket_type::tcp>;
    using tcp_listener_socket_traits = socket_traits<socket_type::tcp_listener>;
    using udp_socket_traits = socket_traits<socket_type::udp>;
    

    //=========================================================================
    template <typename T>
    concept socket_traits_concept = std::is_same_v<T, socket_traits<T::type>>;

} // namespace maniscalco::network

#pragma once

#include <cstdint>
#include <concepts>
#include <type_traits>


namespace maniscalco::network
{

    //=========================================================================
    // the protocol type (udp or tcp)
    enum class network_transport_protocol : std::uint32_t
    {
        undefined                       = 0,
        transmission_control_protocol   = 1,
        tcp                             = transmission_control_protocol,
        user_datagram_protocol          = 2,
        udp                             = user_datagram_protocol
    };

    template <network_transport_protocol T>
    using udp_concept = std::bool_constant<T == network_transport_protocol::udp>;

    template <network_transport_protocol T>
    using tcp_concept = std::bool_constant<T == network_transport_protocol::tcp>;

    template <network_transport_protocol T>
    inline constexpr auto is_udp_v = udp_concept<T>::value;

    template <network_transport_protocol T>
    inline constexpr auto is_tcp_v = tcp_concept<T>::value;

} // namespace maniscalco::network

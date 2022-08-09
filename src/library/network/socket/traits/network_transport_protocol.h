#pragma once

#include <cstdint>


namespace maniscalco::network
{

    enum class network_transport_protocol : std::uint32_t
    {
        undefined       = 0,
        tcp             = 1,
        udp             = 2
    };

    //=========================================================================
    template <typename T>
    concept udp_protocol_concept = std::is_same_v<T, network_transport_protocol::udp>;

    //=========================================================================
    template <typename T>
    concept tcp_protocol_concept = std::is_same_v<T, network_transport_protocol::tcp>;
    
} // namespace maniscalco::network

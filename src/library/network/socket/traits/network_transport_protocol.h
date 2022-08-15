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
    template <network_transport_protocol T>
    concept udp_protocol_concept = (T == network_transport_protocol::udp);

    //=========================================================================
    template <network_transport_protocol T>
    concept tcp_protocol_concept = (T == network_transport_protocol::tcp);
    
} // namespace maniscalco::network

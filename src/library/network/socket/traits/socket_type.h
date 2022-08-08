#pragma once

#include <cstdint>


namespace maniscalco::network
{

    enum class socket_type : std::uint32_t
    {
        undefined       = 0,
        tcp             = 1,
        udp             = 2,
        tcp_listener    = 3
    };

} // namespace maniscalco::network

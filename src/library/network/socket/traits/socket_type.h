#pragma once

#include <cstdint>


namespace maniscalco::network
{

    //=========================================================================
    // the type of socket (active or passive)
    enum class socket_type : std::uint32_t
    {
        undefined       = 0,
        passive         = 1,
        active          = 2
    };

} // namespace maniscalco::network

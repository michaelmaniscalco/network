#pragma once

#include "./network_id.h"
#include "./port_id.h"

#include <fmt/format.h>

#include <string>
#include <iostream>


namespace maniscalco::network
{

    // TODO: abstract to include version (4 vs 6)

    class ip_address
    {
    public:

        ip_address() noexcept = default;
        ip_address(ip_address const &) noexcept = default;
        ip_address & operator = (ip_address const &) noexcept = default;
        ip_address(ip_address &&) noexcept = default;
        ip_address & operator = (ip_address &&) noexcept = default;

        ip_address
        (
            network_id,
            port_id
        ) noexcept;

        ip_address
        (
            network_id
        ) noexcept;

        ip_address
        (
            ::sockaddr_in
        ) noexcept;

        network_id get_network_id() const noexcept;

        port_id get_port_id() const noexcept;
        
        operator ::sockaddr_in() const noexcept;

        bool is_valid() const noexcept;

        bool is_multicast() const noexcept;

    private:

        network_id  networkId_{};

        port_id     portId_{};

    }; // class ip_address


    //=========================================================================
    static std::string to_string
    (
        ip_address ipAddress
    )
    {
        return fmt::format("{}:{}", to_string(ipAddress.get_network_id()), to_string(ipAddress.get_port_id()));     
    }

} // namespace maniscalco::network


//=============================================================================
inline maniscalco::network::ip_address::ip_address
(
    network_id networkId,
    port_id portId
) noexcept :
    networkId_(networkId),
    portId_(portId)
{
}


//=============================================================================
inline maniscalco::network::ip_address::ip_address
(
    network_id networkId
) noexcept :
    networkId_(networkId)
{
}


//=============================================================================
inline bool maniscalco::network::ip_address::is_multicast
(
) const noexcept
{
    return networkId_.is_multicast();
}


//=============================================================================
static std::ostream & operator << 
(
    std::ostream & stream,
    maniscalco::network::ip_address const & ipAddress
)
{
    stream << to_string(ipAddress);
    return stream;
}

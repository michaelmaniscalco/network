#include "./ip_address.h"

#include <include/endian.h>

#include <netinet/ip.h>


//=============================================================================
maniscalco::network::ip_address::ip_address
(
    ::sockaddr_in socketAddrIn
) noexcept :
    networkId_(socketAddrIn.sin_addr),
    portId_(endian_swap<std::endian::big, std::endian::native>(socketAddrIn.sin_port))
{
}  


//=============================================================================
bool maniscalco::network::ip_address::is_valid
(
) const noexcept
{
    return (networkId_.is_valid() && portId_.is_valid());
}


//=============================================================================
auto maniscalco::network::ip_address::get_network_id
(
) const noexcept -> network_id
{
    return networkId_;
}


//=============================================================================
auto maniscalco::network::ip_address::get_port_id
(
) const noexcept -> port_id
{
    return portId_;
}


//=============================================================================
maniscalco::network::ip_address::operator ::sockaddr_in
(
) const noexcept
{
    return {.sin_port = endian_swap<std::endian::native, std::endian::big>(portId_).get(), .sin_addr =  networkId_};
}
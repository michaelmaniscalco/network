#include "./packet.h"


//=============================================================================
std::atomic<std::uint64_t> maniscalco::network::packet::nextUniqueId_{0};


//=============================================================================
auto maniscalco::network::packet::capacity
(
) const -> size_type
{
    return ((data_ != nullptr) ? get_header().capacity_ : 0);
}


//=============================================================================
auto maniscalco::network::packet::get_timestamp
(
) const -> timestamp_type
{
    return get_header().timestamp_;
}


//=============================================================================
std::int32_t maniscalco::network::packet::get_error_code
(
) const
{
    return get_header().errorCode_;
}


//=============================================================================
auto maniscalco::network::packet::get_header
(
) -> packet_header &
{
    return *(packet_header *)(data_);
}


//=============================================================================
auto maniscalco::network::packet::get_header
(
) const -> packet_header const & 
{
    return *(packet_header const *)(data_);
}


//=============================================================================
std::chrono::nanoseconds maniscalco::network::packet::get_age
(
) const
{
    timestamp_type now(std::chrono::system_clock::now().time_since_epoch());
    return std::chrono::duration_cast<std::chrono::nanoseconds>(now.get() - get_timestamp().get());
}


//=============================================================================
auto maniscalco::network::packet::get_virtual_network_interface_id
(
) const -> virtual_network_interface_id
{
    return get_header().virtualNetworkInterfaceId_;
}

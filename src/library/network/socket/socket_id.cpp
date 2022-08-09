#include "./socket_id.h"

#include <atomic>


//=============================================================================
maniscalco::network::socket_id::socket_id
(
)
{
    static std::atomic<value_type> next{0};
    value_ = ++next;
}


//=============================================================================
auto maniscalco::network::socket_id::get
(
) const noexcept -> value_type
{
    return value_;
}


//=============================================================================
bool maniscalco::network::socket_id::is_valid
(
) const noexcept
{
    return (value_ > 0);
}


//=============================================================================
bool maniscalco::network::socket_id::operator <
(
    socket_id const & other
) const noexcept
{
    return (value_ < other.value_);
}

#include "./port_id.h"


//=============================================================================
maniscalco::network::port_id::port_id
(
    std::string_view const value
)
{
    // TODO: handle invalid value
    std::from_chars(value.data(), value.data() + value.size(), value_);
}

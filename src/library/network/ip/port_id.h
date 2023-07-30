#pragma once

#include <include/endian.h>

#include <cstdint>
#include <string>
#include <iostream>


namespace maniscalco::network
{

    class port_id
    {
    public:

        using value_type = std::uint16_t;
        static value_type constexpr any{0};

        port_id() noexcept = default;
        port_id(port_id const &) noexcept = default;
        port_id & operator = (port_id const &) noexcept = default;
        port_id(port_id &&) noexcept = default;
        port_id & operator = (port_id &&) noexcept = default;

        explicit constexpr port_id
        (
            value_type
        ) noexcept;

        value_type get() const noexcept;

        bool is_valid() const noexcept;

    private:

        value_type value_{any};
    };


    //=========================================================================
    static port_id byte_swap
    (
        port_id source
    )
    {
        return port_id(maniscalco::byte_swap(source.get()));
    }
    

    //=========================================================================
    static std::string to_string
    (
        port_id portId
    )
    {
        return std::to_string(portId.get());     
    }

} // namespace maniscalco::network


//=============================================================================
static std::ostream & operator << 
(
    std::ostream & stream,
    maniscalco::network::port_id const & portId
)
{
    stream << portId.get();
    return stream;
}


//=============================================================================
constexpr maniscalco::network::port_id::port_id
(
    std::uint16_t value
) noexcept :
    value_(value)
{
}


//=============================================================================
inline std::uint16_t maniscalco::network::port_id::get
(
) const noexcept
{
    return value_;
}


//=============================================================================
inline bool maniscalco::network::port_id::is_valid
(
) const noexcept
{
    return (value_ != 0);
}


//=============================================================================
namespace maniscalco::network
{
    static port_id constexpr port_id_any{port_id::any};
}

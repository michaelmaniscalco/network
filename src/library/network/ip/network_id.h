#pragma once

#include <include/endian.h>

#include <fmt/format.h>

#include <cstdint>
#include <string>
#include <netinet/in.h>

#include <iostream>


namespace maniscalco::network
{

    class network_id
    {
    public:

        using value_type = std::uint32_t;

        network_id() noexcept = default;
        network_id(network_id const &) noexcept = default;
        network_id & operator = (network_id const &) noexcept = default;
        network_id(network_id &&) noexcept = default;
        network_id & operator = (network_id &&) noexcept = default;

        explicit constexpr network_id
        (
            value_type
        ) noexcept;

        explicit constexpr network_id
        (
            ::in_addr
        ) noexcept;
        
        value_type get() const noexcept;

        bool is_valid() const noexcept;

        operator ::in_addr() const noexcept;

    private:

        value_type value_{};
    };


    //=========================================================================
    static network_id byte_swap
    (
        network_id source
    ) 
    {
        return network_id(maniscalco::byte_swap(source.get()));
    }


    //=========================================================================
    static std::string to_string
    (
        network_id networkId
    )
    {
        auto value = networkId.get();
        auto p = reinterpret_cast<std::uint8_t const *>(&value);
        return fmt::format("{}.{}.{}.{}", p[3], p[2], p[1], p[0]);       
    }

} // namespace maniscalco::network


//=============================================================================
static std::ostream & operator << 
(
    std::ostream & stream,
    maniscalco::network::network_id const & networkId
)
{
    stream << to_string(networkId);
    return stream;
}


//=============================================================================
constexpr maniscalco::network::network_id::network_id
(
    std::uint32_t value
) noexcept :
    value_(value)
{
}


//=============================================================================
constexpr maniscalco::network::network_id::network_id
(
    ::in_addr inAddr
) noexcept :
    value_(endian_swap<std::endian::big, std::endian::native>(inAddr.s_addr))
{
}


//=============================================================================
inline std::uint32_t maniscalco::network::network_id::get
(
) const noexcept
{
    return value_;
}


//=============================================================================
inline bool maniscalco::network::network_id::is_valid
(
) const noexcept
{
    return (value_ != 0);
}


//=============================================================================
inline maniscalco::network::network_id::operator ::in_addr
(
) const noexcept
{
    return {.s_addr = endian_swap<std::endian::native, std::endian::big>(value_)};
}


//=============================================================================
namespace maniscalco::network
{
    static network_id constexpr local_host{INADDR_LOOPBACK};
    static network_id constexpr loop_back{INADDR_LOOPBACK};
    static network_id constexpr in_addr_any{INADDR_ANY};
    
}
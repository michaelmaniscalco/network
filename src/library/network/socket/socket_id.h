#pragma once

#include <cstdint>
#include <utility>
#include <unistd.h>

#include <iostream>


namespace maniscalco
{

    class socket_id
    {
    public:

        using value_type = std::uint64_t;

        socket_id();

        socket_id(socket_id const &) = default;

        socket_id & operator = (socket_id const &) = default;

        bool is_valid() const noexcept;

        value_type get() const noexcept;

        bool operator <
        (
            socket_id const &
        ) const noexcept;

    private:

        value_type   value_{0};
    };

} // namespace maniscalco


//=============================================================================
static std::ostream & operator << 
(
    std::ostream & stream,
    maniscalco::socket_id const & socketId
)
{
    stream << socketId.get();
    return stream;
}

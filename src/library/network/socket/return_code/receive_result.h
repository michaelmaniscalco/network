#pragma once

#include <type_safe/strong_typedef.hpp>

#include <cstdint>
#include <span>


namespace maniscalco::network
{


    struct receive_error : type_safe::strong_typedef<receive_error, std::int32_t>
    {
        using strong_typedef::strong_typedef;
        auto get_error_code() const noexcept
        {
            return static_cast<std::int32_t>(*this);
        }
    };

} // namespace maniscalco::network

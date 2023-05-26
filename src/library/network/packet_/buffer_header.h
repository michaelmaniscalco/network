#pragma once

#include <cstdint>
#include <chrono>


namespace maniscalco::network
{

    static auto constexpr buffer_header_size = 64;

    #pragma pack(push, 1)
    struct alignas(buffer_header_size) buffer_header
    {
        enum class type : std::uint8_t
        {
            heap_allocation,
            pooled_allocation
        };

        type                                    type_{type::heap_allocation};
        std::uint16_t                           headerSize_{sizeof(buffer_header)};
        std::uint16_t                           beginOffset_{0};
        std::uint16_t                           size_{0};
        std::uint16_t                           capacity_{0};
        std::size_t                             id_{0};
    };
    #pragma pack(pop)

    static_assert(sizeof(buffer_header) == buffer_header_size);

}
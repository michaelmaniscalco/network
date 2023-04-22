#pragma once

#include "./buffer_id.h"

#include <include/non_copyable.h>
#include <include/non_movable.h>

#include <atomic>
#include <memory>
#include <cstdint>
#include <functional>


namespace maniscalco::network
{

    class buffer;


    class buffer_allocator
    {
    public:

        using element_type = buffer;
        using iterator = element_type *;
        using const_iterator = element_type const *;
        using size_type = std::size_t;

        static auto constexpr default_buffer_capacity = ((1ull << 10) * 32);
        struct configuration
        {
            std::size_t     capacity_{default_buffer_capacity};
            std::size_t     bufferPoolAlignment_{(1 << 20) * 2};
        };

        buffer_allocator
        (
            configuration const &
        );

        ~buffer_allocator() = default;

        element_type * get_buffer
        (
            buffer_id
        );

        element_type const * get_buffer
        (
            buffer_id
        ) const;

        size_type capacity() const;

        iterator begin();

        const_iterator begin() const;

        iterator end();

        const_iterator end() const;

        void recycle
        (
            element_type *
        );

    private:

        bool initialize
        (
            configuration const &
        );

        size_type           capacity_;

        std::unique_ptr<element_type [], std::function<void(element_type *)>>      bufferPool_;

        std::unique_ptr<element_type * []>      availableQueue_;

        std::size_t                             pushBitMask_;

        std::atomic<std::size_t>                pushIndex_{0};

        std::atomic<std::size_t>                popIndex_{0};
    };

} // maniscalco::network

#include "./buffer.h"


//==============================================================================
inline auto maniscalco::network::buffer_allocator::get_buffer
(
    buffer_id id
) -> element_type *
{
    return (bufferPool_.get() + id);
}


//==============================================================================
inline auto maniscalco::network::buffer_allocator::get_buffer
(
    buffer_id id
) const -> element_type const * 
{
    return (bufferPool_.get() + id);
}


//=============================================================================
inline void maniscalco::network::buffer_allocator::recycle
(
    element_type * buffer
)
{
    availableQueue_[pushIndex_++ & pushBitMask_] = buffer;
}
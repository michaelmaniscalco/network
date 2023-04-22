#pragma once

#include "./buffer_id.h"

#include <array>
#include <cstdint>
#include <memory>
#include <limits>


namespace maniscalco::network
{

    class buffer_allocator;

    class buffer
    {
    public:

        using element_type = std::int8_t;
        using iterator = element_type *;
        using const_iterator = element_type const *;
        using size_type = std::size_t;
        using id_type = buffer_id;

        static constexpr auto physical_size = ((1 << 10) * 2);
        static auto constexpr header_size = 128;

        struct alignas(header_size) header_type
        {
            std::array<std::byte, 128 - 16>     payload_;
            buffer_id const                     id_{0};
            buffer_allocator *                  bufferAllocator_;
        };

        static_assert(sizeof(header_type) == header_size);

        buffer() = default;

        buffer
        (
            buffer_allocator *,
            id_type
        );

        ~buffer() = default;

        size_type capacity() const;

        iterator begin();

        const_iterator begin() const;

        iterator end();

        const_iterator end() const;

        id_type get_id() const;

        bool valid() const;

        element_type const * data() const;

        header_type const & get_header() const;

        void recycle();

        static constexpr size_type get_data_offset();

    private:

        buffer(buffer const &) = delete;
        buffer(buffer &&) = delete;
        buffer & operator = (buffer const &) = delete;
        buffer & operator = (buffer &&) = delete;

        static constexpr auto alignment_size = ((sizeof(header_type) % 64) > 0) ? (64 - (sizeof(header_type) % 64)) : 0;
        static constexpr auto data_size = (physical_size - (sizeof(header_type) + alignment_size));

        header_type                             header_;

        char                                    padding_[alignment_size];

        std::array<element_type, data_size>     data_;
    };

    static_assert((sizeof(buffer) == buffer::physical_size), "buffer struct is the wrong size");

} // maniscalco::network

#include "./buffer_allocator.h"


//==============================================================================
inline constexpr auto maniscalco::network::buffer::get_data_offset
(
    // return the offset from the beginning of this object to its 'data_' member.
) -> size_type
{
    return (sizeof(header_) + sizeof(padding_));
}


//==============================================================================
inline auto maniscalco::network::buffer::data
(
    // returns address of actual message data excluding the header
) const -> element_type const *
{
    return data_.data();
}


//==============================================================================
inline auto maniscalco::network::buffer::get_header
(
) const -> header_type const &
{
    return header_;
}


//==============================================================================
inline bool maniscalco::network::buffer::valid
(
) const
{
    return (header_.id_ != -1);
}


//==============================================================================
inline auto maniscalco::network::buffer::capacity
(
) const -> size_type
{
    return data_.size();
}


//==============================================================================
inline auto maniscalco::network::buffer::begin
(
) -> iterator 
{
    return data_.begin();
}


//==============================================================================
inline auto maniscalco::network::buffer::begin
(
) const -> const_iterator
{
    return data_.begin();
}


//==============================================================================    
inline auto maniscalco::network::buffer::end
(
) -> iterator
{
    return data_.end();
}


//==============================================================================
inline auto maniscalco::network::buffer::end
(
) const -> const_iterator
{
    return data_.end();
}


//==============================================================================
inline auto maniscalco::network::buffer::get_id
(
) const -> buffer_id
{
    return header_.id_;
}


//==============================================================================
inline void maniscalco::network::buffer::recycle
(
)
{
    if (header_.bufferAllocator_)
        header_.bufferAllocator_->recycle(this);
}

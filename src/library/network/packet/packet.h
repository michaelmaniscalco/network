#pragma once

#include <include/non_copyable.h>

#include <span>
#include <cstdint>
#include <utility>


namespace maniscalco::network
{

    class packet :
        non_copyable
    {
    public:

        using element_type = char;
        using delete_handler = void(*)(packet const &);

        struct event_handlers
        {
            delete_handler deleteHandler_;
        };

        packet
        (
            event_handlers const &, 
            std::span<element_type>
        );

        packet
        (
            packet &&
        );

        packet & operator = 
        (
            packet &&
        );

        ~packet();

        auto capacity() const;

        auto begin();

        auto begin() const;

        auto end();

        auto end() const;

        auto data() const;

        auto data();

        auto size() const;

        bool empty() const;

        bool resize
        (
            std::size_t
        );

    private:

        void release();

        std::span<element_type>  buffer_;

        delete_handler  deleteHandler_;

        std::size_t     size_{0};

    };

} // namespace maniscalco::network


//=============================================================================
inline maniscalco::network::packet::packet
(
    event_handlers const & eventHandler, 
    std::span<element_type> buffer
):
    buffer_(buffer), 
    deleteHandler_(eventHandler.deleteHandler_)
{
}


//=============================================================================
inline maniscalco::network::packet::packet
(
    packet && other
):
    buffer_(other.buffer_),
    deleteHandler_(other.deleteHandler_),
    size_(other.size_)
{
    other.deleteHandler_ = {};
    other.size_ = {};
    other.buffer_ = {};
}


//=============================================================================
inline auto maniscalco::network::packet::operator = 
(
    packet && other
) -> packet & 
{
    if (&other != this)
    {
        release();
        buffer_ = other.buffer_;
        deleteHandler_ = other.deleteHandler_;
        size_ = other.size_;

        other.deleteHandler_ = {};
        other.size_ = {};
        other.buffer_ = {};
    }
    return *this;
}


//=============================================================================
inline maniscalco::network::packet::~packet
(
)
{
    release();
}


//=============================================================================
inline auto maniscalco::network::packet::capacity
(
) const
{
    return buffer_.size();
}


//=============================================================================
inline auto maniscalco::network::packet::begin
(
)
{
    return buffer_.begin();
}


//=============================================================================        
inline auto maniscalco::network::packet::begin
(
) const
{
    return buffer_.begin();
}


//=============================================================================
inline auto maniscalco::network::packet::end
(
)
{
    return begin() + size_;
}


//=============================================================================
inline auto maniscalco::network::packet::end
(
) const
{
    return begin() + size_;
}


//=============================================================================
inline auto maniscalco::network::packet::data
(
) const
{
    return buffer_.data();
}


//=============================================================================
inline auto maniscalco::network::packet::data
(
)
{
    return buffer_.data();
}


//=============================================================================
inline auto maniscalco::network::packet::size
(
) const
{
    return size_;
}


//=============================================================================
inline bool maniscalco::network::packet::empty
(
) const
{
    return (size_ == 0);
}


//=============================================================================
inline bool maniscalco::network::packet::resize
(
    std::size_t size
)
{
    if (size > capacity()) 
        return false; 
    size_ = size; 
    return true;
}


//=============================================================================
inline void maniscalco::network::packet::release
(
)
{
    if (deleteHandler_)
        std::exchange(deleteHandler_, nullptr)(*this);
    size_ = {};
    buffer_ = {};
}

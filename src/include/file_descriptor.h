#pragma once

#include <cstdint>
#include <utility>
#include <unistd.h>

#include <iostream>


namespace maniscalco
{

    class file_descriptor
    {
    public:

        using value_type = std::int32_t;

        file_descriptor() = default;

        file_descriptor
        (
            value_type
        );

        file_descriptor
        (
            file_descriptor &&
        );

        file_descriptor & operator =
        (
            file_descriptor &&
        );

        ~file_descriptor();

        bool close();

        value_type get() const;

        bool is_valid() const;

    private:

        file_descriptor(file_descriptor const &) = delete;
        file_descriptor & operator = (file_descriptor const &) = delete;

        value_type   value_{0};
    };

} // namespace maniscalco


//=============================================================================
static std::ostream & operator << 
(
    std::ostream & stream,
    maniscalco::file_descriptor const & fileDescriptor
)
{
    stream << fileDescriptor.get();
    return stream;
}


//=============================================================================
inline maniscalco::file_descriptor::file_descriptor
(
    value_type value
):
    value_(value)
{
}


//=============================================================================
inline maniscalco::file_descriptor::file_descriptor
(
    file_descriptor && other
)
{
    value_ = other.value_;
    other.value_ = {};
}



//=============================================================================
inline maniscalco::file_descriptor::~file_descriptor
(
)
{
    close();
}


//=============================================================================
inline auto maniscalco::file_descriptor::operator =
(
    file_descriptor && other
) -> file_descriptor & 
{
    if (this != & other)
    {
        close();
        value_ = other.value_;
        other.value_ = {};
    }
    return *this;
}


//=============================================================================
inline bool maniscalco::file_descriptor::close
(
)
{
    auto value = std::exchange(value_, 0);
    if (value != 0)
        ::close(value);
    return (value != 0);
}


//=============================================================================
inline auto maniscalco::file_descriptor::get
(
) const -> value_type
{
    return value_;
}


//=============================================================================
inline bool maniscalco::file_descriptor::is_valid
(
) const
{
    return (value_ > 0);
}

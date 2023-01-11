#pragma once

#include <cstdint>


namespace maniscalco::network
{

    class send_result
    {
    public:

        send_result
        (
            std::size_t bytesSent
        ):
            bytesSent_(bytesSent)
        {
        }

        send_result
        (
            std::int64_t errorCode,
            std::size_t bytesSent
        ):
            errorCode_(errorCode),
            bytesSent_(bytesSent)
        {
        }

        auto is_error() const noexcept
        {
            return (errorCode_ != 0);
        }

        auto get_error_code() const noexcept
        {
            return errorCode_;
        }

        auto get_bytes_sent() const noexcept
        {
            return bytesSent_;
        }
    
    private:

        std::int64_t errorCode_{0};
        std::size_t bytesSent_{0};
    };


} // namespace maniscalco::network

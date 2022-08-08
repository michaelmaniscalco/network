#pragma once

#include <cstdint>
#include <string_view>
#include <string>


namespace maniscalco::network
{

    enum class bind_result : std::uint32_t
    {
        undefined               = 0,
        success                 = 1,
        bind_error              = 2,
        invalid_file_descriptor = 3
    };


    //=========================================================================
    static std::string_view const to_string
    (
        bind_result connectResult
    )
    {
        using namespace std::string_literals;
        
        static auto constexpr undefined = "undefined";
        static auto constexpr success = "success";
        static auto constexpr invalid_file_descriptor = "invalid_file_descriptor";
        static auto constexpr bind_error = "bind_error";

        switch (connectResult)
        {
            case bind_result::success: return success;
            case bind_result::invalid_file_descriptor: return invalid_file_descriptor;
            case bind_result::bind_error: return bind_error;
            case bind_result::undefined: 
            default:
            {
                return undefined;
            }
        }
    }
}
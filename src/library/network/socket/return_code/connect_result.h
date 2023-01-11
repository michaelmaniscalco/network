#pragma once

#include <cstdint>
#include <string_view>
#include <string>


namespace maniscalco::network
{

    enum class connect_result : std::uint32_t
    {
        undefined               = 0,
        success                 = 1,
        invalid_file_descriptor = 2,
        already_connected       = 3,
        connect_error           = 4,
        in_progress             = 5,
        invalid_destination     = 6
    };


    //=========================================================================
    [[maybe_unused]] static std::string_view const to_string
    (
        connect_result connectResult
    )
    {
        using namespace std::string_literals;
        
        static auto constexpr undefined = "undefined";
        static auto constexpr success = "success";
        static auto constexpr invalid_file_descriptor = "invalid_file_descriptor";
        static auto constexpr already_connected = "already_connected";
        static auto constexpr connect_error = "connect_error";
        static auto constexpr in_progress = "in_progress";
        static auto constexpr invalid_destination = "invalid_destination";

        switch (connectResult)
        {
            case connect_result::success: return success;
            case connect_result::invalid_file_descriptor: return invalid_file_descriptor;
            case connect_result::already_connected: return already_connected;
            case connect_result::connect_error: return connect_error;
            case connect_result::in_progress: return in_progress;
            case connect_result::invalid_destination: return invalid_destination;

            case connect_result::undefined: 
            default:
            {
                return undefined;
            }
        }
    }
}
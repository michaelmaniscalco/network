#pragma once

#include <cstdint>


namespace maniscalco::network
{

    //=========================================================================
    // the type of network interface (kernel or ef_vi)
    enum class network_interface_type : std::uint32_t
    {
        undefined       = 0,
        kernel          = 1,
        ef_vi           = 2
    };

    template <network_interface_type T>
    using kernel_network_interface_concept = std::bool_constant<T == network_interface_type::kernel>;

    template <network_interface_type T>
    using ef_vi_network_interface_concept = std::bool_constant<T == network_interface_type::ef_vi>;

    template <network_interface_type T>
    inline constexpr auto is_kernel_network_interface_v = kernel_network_interface_concept<T>::value;

    template <network_interface_type T>
    inline constexpr auto is_ef_vi_network_interface_v = ef_vi_network_interface_concept<T>::value;

} // namespace maniscalco::network

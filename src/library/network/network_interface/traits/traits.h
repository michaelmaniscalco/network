#pragma once

#include "./network_interface_type.h"

#include <concepts>
#include <type_traits>


namespace maniscalco::network
{

    //=========================================================================
    // network_interface traits
    // 1: kernel
    // 2: ef_vi
    template <network_interface_type T>
    struct network_interface_traits
    {
        static auto constexpr type = T;
    };


    //=========================================================================
    // aliases for the various valid network_interface traits
    using kernel_network_interface_traits = network_interface_traits<network_interface_type::kernel>;
    using ef_vi_network_interface_traits = network_interface_traits<network_interface_type::ef_vi>;
    

    //=========================================================================
    template <typename T>
    concept network_interface_traits_concept = std::is_same_v<T, network_interface_traits<T::type>>;

} // namespace maniscalco::network

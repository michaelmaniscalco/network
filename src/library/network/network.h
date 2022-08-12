#pragma once

#include "./network_interface/network_interface.h"

#include <string>


namespace maniscalco::network
{

    network_id get_network_id_from_hostname
    (
        std::string
    );

}
#include "./network.h"

#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>


//=============================================================================
auto maniscalco::network::get_network_id_from_hostname
(
    // experimental 
    // placeholder for now
    std::string hostname
) -> network_id
{
    struct ::addrinfo * result = nullptr;
    ::getaddrinfo(hostname.c_str(), 0, 0, &result);
    for (struct addrinfo * cur = result; cur != nullptr; cur = cur->ai_next)
        if (cur->ai_addr->sa_family == AF_INET) 
            return network_id(((struct sockaddr_in *)(cur->ai_addr))->sin_addr);
    return {};
}

#include <library/network.h>

#include <array>
#include <iostream>
#include <thread>


//=============================================================================
int main
(
    int,
    char **
)
{
    using namespace maniscalco::network;
    using namespace std::string_literals;

    ip_address multicastIpAddress{"239.0.0.1"};
    port_id multicastPortId{3000};

    socket_address anyLookbackSocketAddress{loop_back, port_id_any};

    // set up network interface and threads to poll and receive
    network_interface networkInterface;
    std::jthread pollerThread([&](std::stop_token const & stopToken){while (!stopToken.stop_requested()) networkInterface.poll();});
    std::jthread workerThread([&](std::stop_token const & stopToken){while (!stopToken.stop_requested()) networkInterface.service_sockets();});

    // set up sender
    auto sender = networkInterface.udp_connect(anyLookbackSocketAddress, {multicastIpAddress, multicastPortId}, {}, {});

    // set up receivers
    static auto constexpr number_of_receivers = 10;
    std::array<udp_socket, number_of_receivers> receivers;
    for (auto & receiver : receivers)
        receiver = networkInterface.multicast_join({multicastIpAddress, multicastPortId}, {}, 
                {.receiveHandler_ = [](auto, auto packet, auto){std::cout << "got multicast packet. data = " << std::string_view(packet.data(), packet.size()) << std::endl;}});

    // send messages
    for (auto i = 0; i < 100; ++i)
        sender.send("this is a multicast message");

    // demo is async so give it a moment to complete
    std::this_thread::sleep_for(std::chrono::seconds(1)); 

    pollerThread.request_stop();
    workerThread.request_stop();

    pollerThread.join();
    workerThread.join();

    return 0;
}

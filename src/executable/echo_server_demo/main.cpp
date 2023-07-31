#include <library/network.h>
#include <library/system.h>

#include <include/non_movable.h>
#include <include/non_copyable.h>

#include <iostream>
#include <map>

using namespace maniscalco;
using namespace maniscalco::network;
using namespace maniscalco::system;


//=============================================================================
struct echo_server : non_movable, non_copyable
{
    echo_server():
        socket_(networkInterface_.tcp_listen({loop_back, port_id(3000)}, {},
            {.acceptHandler_ = [this](auto, auto fileDescriptor){sessions.push_back(std::make_unique<session>(networkInterface_, std::move(fileDescriptor)));}})),
        pollerThread_([this](std::stop_token const & stopToken){while (!stopToken.stop_requested()) networkInterface_.poll();}),
        workerThread_([this](std::stop_token const & stopToken){while (!stopToken.stop_requested()) networkInterface_.service_sockets();}){}

    struct session : non_movable, non_copyable
    {
        session(network_interface & networkInterface, file_descriptor fileDescriptor):
            tcpSocket_(networkInterface.tcp_accept(std::move(fileDescriptor), {}, {.receiveHandler_ = [this](auto, auto packet){tcpSocket_.send(packet);}})){}
        tcp_socket tcpSocket_;
    };

    network_interface                       networkInterface_;
    tcp_listener_socket                     socket_;
    std::vector<std::unique_ptr<session>>   sessions;
    std::jthread                            pollerThread_;
    std::jthread                            workerThread_;
};


//=============================================================================
int main
(
    int,
    char **
)
{
    echo_server echoServer;

    network_interface networkInterface;
    std::jthread pollerThread([&](auto const & stopToken){while (!stopToken.stop_requested()) networkInterface.poll();});
    std::jthread workerThread([&](auto const & stopToken){while (!stopToken.stop_requested()) networkInterface.service_sockets();});

    auto tcpSocket = networkInterface.tcp_connect(loop_back, {loop_back, port_id(3000)}, {}, 
            {.receiveHandler_ = [&](auto, auto packet){std::cout << std::string(packet.data(), packet.size()) << std::flush;}});

    for (auto i = 0; i < 10; ++i)
        tcpSocket.send(fmt::format("message {}\n", i));

    std::this_thread::sleep_for(std::chrono::seconds(1)); // demo is async so give it a moment to complete
    return 0;
}

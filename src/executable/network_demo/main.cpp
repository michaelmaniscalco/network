#include <library/network.h>
#include <library/system.h>

#include <iostream>
#include <map>


namespace
{

    // For the purposes of more clearly demonstrating the use of sockets
    // this demo sets up globals for thread pools, network interface etc.
    //
    // Obviously, in a real world application we would not do this.

    using namespace maniscalco::network;
    using namespace maniscalco::system;
    using namespace std::string_literals;
    std::mutex mutex;

    std::map<socket_id, tcp_socket> acceptedTcpSockets;

    static ip_address const any_loopback_ip_address{"127.0.0.1", port_id_any};
    static ip_address const tcp_listener_ip_address{"127.0.0.1"s, port_id(3000)};

    // set up work contract group
    auto workContractGroup = work_contract_group::create({});
    // set up a network interface
    network_interface networkInterface({}, workContractGroup);

    auto closeHandler = []
            (
                auto socketId
            )
            {
                std::lock_guard lg(mutex);
                std::cout << "socket " << socketId << ": closed\n";
            };

    auto receiveHandler = []
            (
                auto socketId, 
                auto buffer
            )
            {
                std::lock_guard lg(mutex);
                std::cout << "socket " << socketId << ": received message: \"" << 
                        std::string((char const *)buffer.data(), buffer.size()) << "\"\n";
            };

    auto acceptHandler = []
            (
                auto socketId, 
                auto fileDescriptor
            )
            {
                auto acceptedTcpSocket = networkInterface.open_socket<tcp_socket>(
                        std::move(fileDescriptor),
                        {},
                        {
                            .closeHandler_ = closeHandler,
                            .receiveHandler_ = [&](auto socketId, auto buffer)
                            {
                                std::lock_guard lg(mutex);
                                std::cout << "socket " << socketId << ": received message: \"" << 
                                        std::string((char const *)buffer.data(), buffer.size()) << "\"\n";
                                if (auto iter = acceptedTcpSockets.find(socketId); iter != acceptedTcpSockets.end())
                                    iter->second.send("right back at you good buddy!");
                            }
                        }); 
                auto acceptedTcpSocketId = acceptedTcpSocket.get_id();
                acceptedTcpSockets[acceptedTcpSocketId] = std::move(acceptedTcpSocket);
            };

} // anonymous namespace


//=============================================================================
void demonstrate_udp_sockets
(
    // create two udp sockets
    // connect sockets
    // send datagrams back and forth between the pair of udp sockets
)
{
    std::cout << "*** UDP demonstration ***\n";
    auto udpSocket1 = networkInterface.open_socket<udp_socket>(any_loopback_ip_address, {}, {.closeHandler_ = closeHandler, .receiveHandler_ = receiveHandler});
    auto udpSocket2 = networkInterface.open_socket<udp_socket>(any_loopback_ip_address, {}, {.closeHandler_ = closeHandler, .receiveHandler_ = receiveHandler});

    udpSocket1.connect_to(udpSocket2.get_ip_address());
    udpSocket2.connect_to(udpSocket1.get_ip_address());

    udpSocket1.send("guess what");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    udpSocket2.send("chicken butt!!!");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    udpSocket1.send("guess why");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    udpSocket2.send("chicken thigh!!!");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


//=============================================================================
void demonstrate_udp_multicast_sockets
(
    // NOTE: multicast demo will require that the "239.0.0.1" ip address exists.
    // see README for more details
)
{
    network_id multicastNetworkId{"239.0.0.1"};
    port_id multicastPortId{3000};

    std::cout << "*** Multicast UDP demonstration ***\n";
    auto sender = networkInterface.open_socket<udp_socket>(any_loopback_ip_address, {}, {.closeHandler_ = closeHandler, .receiveHandler_ = receiveHandler});
    sender.connect_to(ip_address{multicastNetworkId, multicastPortId});

    static auto constexpr number_of_receivers = 10;
    std::vector<udp_socket> receivers(number_of_receivers);
    for (auto & receiver : receivers)
    {
        // open udp socket and then join the multicast.
        receiver = networkInterface.open_socket<udp_socket>(ip_address{in_addr_any, multicastPortId}, {}, {.closeHandler_ = closeHandler, .receiveHandler_ = receiveHandler});
        receiver.join(multicastNetworkId);
        // this can also be done in one step with:
        // receiver = networkInterface.join_multicast(ip_address{in_addr_any, multicastPortId}, {}, {.closeHandler_ = closeHandler, .receiveHandler_ = receiveHandler}, multicastNetworkId);
    }
    

    for (auto i = 0; i < 10; ++i)
    {
        sender.send("this is a multicast message");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
}


//=============================================================================
void demonstrate_tcp_sockets
(
    // create tcp listener socket
    // create tcp socket
    // connect tcp socket to tcp listener socket
    // accept connection to create a partner tcp socket
    // send messages back and forth across partnered tcp sockets
)
{
    using namespace maniscalco::system;

    std::cout << "*** TCP demonstration ***\n";
    auto tcpListenerSocket = networkInterface.open_socket<tcp_listener_socket>(tcp_listener_ip_address, {}, {.closeHandler_ = closeHandler, .acceptHandler_ = acceptHandler});
    auto tcpSocket = networkInterface.open_socket<tcp_socket>(any_loopback_ip_address, 
            {
                .receiveBufferSize_ = (1 << 20), // demonstrate setting send/receive buffer sizes (works for udp as well)
                .sendBufferSize_ = (1 << 20)
            }, 
            {
                .closeHandler_ = closeHandler, 
                .receiveHandler_ = receiveHandler
            });

    tcpSocket.connect_to(tcp_listener_ip_address);
    tcpSocket.send("greetings partner!");
    // since the socket system is async (and for simplicity sake) simply sleep for a tiny bit to let the partner respond
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    acceptedTcpSockets.clear(); // close any accepted sockets 
}


//=============================================================================
int main
(
    int,
    char **
)
{
    // set up thread pool to process work contracts
    static auto constexpr num_worker_threads = 4;
    std::vector<thread_pool::thread_configuration> threads;
    for (auto i = 0; i < num_worker_threads; ++i)
        threads.push_back({.function_ = [&](std::stop_token const &)
                {
                    workContractGroup->service_contracts();
                }});
    // add one additional thread for polling
    threads.push_back({.function_ = [&](std::stop_token const &){networkInterface.poll();}});
    thread_pool workerThreadPool({.threads_ = threads});

    demonstrate_tcp_sockets();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    demonstrate_udp_sockets();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    demonstrate_udp_multicast_sockets();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    return 0;
}

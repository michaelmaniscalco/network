#include <library/network.h>
#include <library/system.h>

#include <iostream>
#include <map>


//=============================================================================
void demonstrate_udp_sockets
(
)
{
    using namespace maniscalco::network;
    using namespace maniscalco::system;

    // set up work contract group
    auto workContractGroup = work_contract_group::create({});
    // set up thread pool to process work contracts
    thread_pool threadPool({.threadCount_ = 4, .workerThreadFunction_ = [&](){workContractGroup->service_contracts();}});
    // set up a network interface
    network_interface networkInterface({}, workContractGroup);
    // set up a second thread pool just to poll the network interface
    thread_pool threadPool2({.threadCount_ = 1, .workerThreadFunction_ = [&](){networkInterface.poll();}});

    auto closeHandler = [](auto socketId){std::cout << "socket " << socketId << " closed\n";};
    auto receiveHandler = [](auto socketId, auto buffer)
            {
                static std::mutex m;
                std::lock_guard lockGuard(m);

                std::cout << "socket " << socketId << ": " << std::string((char const *)buffer.data(), buffer.size()) << "\n";
            };
    auto acceptHandler = [&](auto socketId, auto fd)
            {
                // hack for demo purposes.  make new socket static so we don't loose scope of it
                std::cout << "accepted socket fd = " << fd << "\n";
                static tcp_socket tcpSocket = networkInterface.open_socket(
                        std::move(fd),
                        {},
                        tcp_socket::event_handlers
                                {
                                    .closeHandler_ = closeHandler,
                                    .receiveHandler_ = [&](auto socketId, auto buffer)
                                    {
                                        std::cout << "socket " << socketId << ": " << 
                                                std::string((char const *)buffer.data(), buffer.size()) << "\n";
                                        tcpSocket.send("right back at you good buddy!");
                                    }
                                }); 
                std::cout << "partner socket is " << tcpSocket.get_connected_ip_address() << "\n"; 
            };

    static ip_address constexpr any_local_ip_address{local_host, port_id_any};
    // create two udp sockets
    udp_socket udpSocket1 = networkInterface.open_socket(
            any_local_ip_address, 
            {}, 
            udp_socket::event_handlers
                    {
                        .closeHandler_ = closeHandler,
                        .receiveHandler_ = receiveHandler
                    });

    udp_socket udpSocket2 = networkInterface.open_socket(
            any_local_ip_address, 
            {}, 
            udp_socket::event_handlers
                    {
                        .closeHandler_ = closeHandler,
                        .receiveHandler_ = receiveHandler
                    });
            
    // "connect" the sockets
    udpSocket1.connect_to(udpSocket2.get_ip_address());
    udpSocket2.connect_to(udpSocket1.get_ip_address());

    udpSocket1.send("guess what");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    udpSocket2.send("chicken butt!!!");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    udpSocket2.send("guess why");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    udpSocket1.send("chicken thigh!!!");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}



//=============================================================================
void demonstrate_tcp_sockets
(
)
{
    using namespace maniscalco;
    using namespace maniscalco::network;
    using namespace maniscalco::system;

    // set up work contract group
    auto workContractGroup = work_contract_group::create({});
    // set up thread pool to process work contracts
    thread_pool threadPool({.threadCount_ = 4, .workerThreadFunction_ = [&](){workContractGroup->service_contracts();}});
    // set up a network interface
    network_interface networkInterface({}, workContractGroup);
    // set up a second thread pool just to poll the network interface
    thread_pool threadPool2({.threadCount_ = 1, .workerThreadFunction_ = [&](){networkInterface.poll();}});

    std::map<socket_id, tcp_socket> acceptedTcpSockets;

    auto closeHandler = [](auto socketId){std::cout << "socket " << socketId << " closed\n";};
    auto receiveHandler = [](auto socketId, auto buffer)
            {
                std::cout << "socket " << socketId << ": " << std::string((char const *)buffer.data(), buffer.size()) << "\n";
            };
    auto acceptHandler = [&](auto socketId, auto fd)
            {
                auto acceptedTcpSocket = networkInterface.open_socket(
                        std::move(fd),
                        {},
                        tcp_socket::event_handlers
                                {
                                    .closeHandler_ = closeHandler,
                                    .receiveHandler_ = [&](auto socketId, auto buffer)
                                    {
                                        std::cout << "socket " << socketId << ": " << 
                                                std::string((char const *)buffer.data(), buffer.size()) << "\n";
                                        if (auto iter = acceptedTcpSockets.find(socketId); iter != acceptedTcpSockets.end())
                                            iter->second.send("right back at you good buddy!");
                                    }
                                }); 
                        auto acceptedTcpSocketId = acceptedTcpSocket.get_id();
                        acceptedTcpSockets[acceptedTcpSocketId] = std::move(acceptedTcpSocket);
                    };

    static ip_address constexpr any_local_ip_address{local_host, port_id_any};

    // create a tcp listener socket
    tcp_listener_socket tcpListenerSocket = networkInterface.open_socket(
            {local_host, port_id(3000)},
            {},
            tcp_listener_socket::event_handlers
                    {
                        .closeHandler_ = closeHandler,
                        .acceptHandler_ = acceptHandler
                    });

    // create a tcp socket
    tcp_socket tcpSocket = networkInterface.open_socket(
            any_local_ip_address,
            {},
            tcp_socket::event_handlers
                    {
                        .closeHandler_ = closeHandler,
                        .receiveHandler_ = receiveHandler
                    });

    // connect to listener socket (making a partner socket for the current tcp socket)
    tcpSocket.connect_to(tcpListenerSocket.get_ip_address());
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    tcpSocket.send("greetings partner!");

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}


//=============================================================================
int main
(
    int,
    char **
)
{
    demonstrate_tcp_sockets();
    demonstrate_udp_sockets();
    return 0;
}

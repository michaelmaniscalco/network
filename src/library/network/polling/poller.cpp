#include "./poller.h"

#include <library/network/socket/private/active_socket_impl.h>
#include <library/network/socket/private/passive_socket_impl.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/epoll.h>

#include <iostream>


//=============================================================================
maniscalco::network::poller::poller
(
    configuration const & config
):
    fileDescriptor_(::epoll_create1(0)),
    trigger_(config.trigger_)
{
}


//=============================================================================
maniscalco::network::poller::~poller
(
)
{
    close();
}


//=============================================================================
void maniscalco::network::poller::close
(
)
{
    fileDescriptor_ = {};
}


//=============================================================================
void maniscalco::network::poller::poll
(
)
{
    std::array<::epoll_event, 1024> epollEvents;
    auto epollWaitResult = ::epoll_wait(fileDescriptor_.get(), epollEvents.data(), epollEvents.size(), 0);
    if (epollWaitResult > 0)
    {
        for (auto i = 0; i < epollWaitResult; ++i)
        {
            auto const & event = epollEvents[i];
            auto impl = reinterpret_cast<socket_base_impl *>(event.data.ptr);
/*
            if (event.events & EPOLLERR)
            {
                if (socketInfo.socketErrorHandler_)
                    socketInfo.socketErrorHandler_();
            }
*/
            if (event.events & EPOLLIN)
                impl->on_polled();
        }   
    }
}


//=============================================================================
template <maniscalco::network::socket_impl_concept S>
auto maniscalco::network::poller::register_socket
(
    S & s
) -> poller_registration
{
    socket_base_impl * socketImpl = &s;
    // add socket to epoller
    ::epoll_event epollEvent;
    epollEvent.data.ptr = socketImpl;
    epollEvent.events = (EPOLLIN | ((trigger_ == trigger_type::edge_triggered) ? EPOLLET : 0));
    auto epollCtlResult = ::epoll_ctl(fileDescriptor_.get(), EPOLL_CTL_ADD, socketImpl->get_file_descriptor().get(), &epollEvent);
    if (epollCtlResult != 0)
    {
        // TODO: log failure
        return {weak_from_this(), {}};
    }
    return {weak_from_this(), socketImpl->get_file_descriptor()};
}


//=============================================================================
bool maniscalco::network::poller::unregister_socket
(
    system::file_descriptor const & fileDescriptor
)
{
    ::epoll_event epollEvent;
    auto epollCtlResult = ::epoll_ctl(fileDescriptor_.get(), EPOLL_CTL_DEL, fileDescriptor.get(), &epollEvent);
    if (epollCtlResult != 0)
    {
        // TODO: log failure
        return false;
    }
    return true;
}


//=============================================================================
namespace maniscalco::network
{

    template poller_registration poller::register_socket(tcp_socket_impl &);
    template poller_registration poller::register_socket(udp_socket_impl &);
    template poller_registration poller::register_socket(tcp_listener_socket_impl &);

}

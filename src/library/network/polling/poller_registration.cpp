#include "./poller_registration.h"
#include "./poller.h"


//=============================================================================
maniscalco::network::poller_registration::poller_registration
(
    std::weak_ptr<poller> p,
    system::file_descriptor const & fileDescriptor
):
    poller_(p),
    fileDescriptor_(fileDescriptor)
{
    if (!fileDescriptor_.is_valid())
        poller_.reset();
}


//=============================================================================
maniscalco::network::poller_registration::~poller_registration
(
)
{
    release();
}


//=============================================================================
void maniscalco::network::poller_registration::release
(
)
{
    if (!poller_.expired())
    {
        if (auto poller = poller_.lock(); poller)
            poller->unregister_socket(fileDescriptor_);
        poller_.reset();
    }
}

#pragma once

#include <include/file_descriptor.h>

#include <memory>


namespace maniscalco::network
{

    class poller;


    class poller_registration
    {
    public:

        poller_registration
        (
            std::weak_ptr<poller>,
            system::file_descriptor const &
        );

        ~poller_registration();

        void release();

    private:

        std::weak_ptr<poller>   poller_;

        system::file_descriptor const & fileDescriptor_;

    }; // class poller_registration

} // namespace maniscalco::network

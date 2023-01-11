#include <cstdint>
#include <memory>
#include <map>

enum class protocol : std::uint32_t
{
    cta,
    siac
};

class market_feed_impl_base
{
public:
    // just so that the pImpl can point to a common impl
    virtual ~market_feed_impl_base(){}
    virtual void destroy() = 0;
};

template <protocol T>
class market_feed_impl_common :
    public market_feed_impl_base
{
public:
    // common to all specializations 
    // handle sockets, parsing, gap detection,
    // forwarding translated messages as citrius messages
    // arbitration, book building etc ...
    virtual ~market_feed_impl_common(){}
};


template <protocol T>
class market_feed_impl :
    public market_feed_impl_common<T>
{
public:
    // specialized functions to handle messages of the specified protocol type
    // registers callbacks for specific messages to 'this' but using protocol_parser
    // in the common ancestor class.

    ~market_feed_impl(){}

    void destroy
    (
    ) override
    {
        // surrender work contract, which causes sockets to unregister from poller.
        // then, in the unregister handler of the work contract we actually delete 'this'
    }
};


template <protocol T>
struct type_wrapper{};


class market_feed
{
public:
    market_feed():impl_(nullptr, nullptr){}

    template <protocol T>
    market_feed
    (
        type_wrapper<T>
    ):
        impl_(new market_feed_impl<T>(), [](market_feed_impl_base * p){p->destroy();})
    {
    }


    market_feed & operator =(market_feed&& other){impl_ = std::move(other.impl_);return *this;}
    market_feed(market_feed && other):impl_(std::move(other.impl_)){}

private:

    std::unique_ptr<market_feed_impl_base, void(*)(market_feed_impl_base *)> impl_;
};



int main
(
    int,
    char **
)
{
    auto siacMarketFeed = market_feed(type_wrapper<protocol::siac>());
    auto ctaMarketFeed = market_feed(type_wrapper<protocol::cta>());


    std::map<int, market_feed> map;
    map[0] = std::move(siacMarketFeed);
    map[1] = std::move(ctaMarketFeed);

}
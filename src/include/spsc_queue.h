#pragma once

#include <functional>
#include <cstdint>
#include <memory>


namespace maniscalco
{

    template <typename T>
    class spsc_queue
    {
    public:

        struct event_handlers
        {
            using select_handler = std::function<void()>;
            select_handler selectHandler_;
        };

        struct configuration
        {
            std::size_t capacity_;
        };

        spsc_queue
        (
            event_handlers const &,
            configuration const &
        );

        auto size() const;

        auto empty() const;

        T & front();

        T const & front() const;

        std::tuple<bool, std::size_t> pop
        (
            T &
        );

        template <typename T_>
        bool push
        (
            T_ &&
        );

        auto capacity() const;

    private:

        std::unique_ptr<T []>   queue_;

        std::size_t             capacity_;

        std::size_t volatile    front_{0};

        std::size_t volatile    back_{0};
    
        event_handlers          eventHandlers_;

    }; // class spsc_queue

} // namespace maniscalco


//=============================================================================
template <typename T>
maniscalco::spsc_queue<T>::spsc_queue
(
    event_handlers const & eventHandlers,
    configuration const & config
):
    queue_(std::make_unique<T []>(config.capacity_)),
    capacity_(config.capacity_)
{
}

//=============================================================================
template <typename T>
auto maniscalco::spsc_queue<T>::size
(
) const
{
    std::size_t front = front_;
    std::size_t back = back_;
    return (front <= back) ? (back - front) : (capacity_ - (back - front));
}


//=============================================================================
template <typename T>
auto maniscalco::spsc_queue<T>::empty
(
) const
{
    return (back_ == front_);
}


//=============================================================================
template <typename T>
T & maniscalco::spsc_queue<T>::front
(
)
{
    return queue_[front_];
}


//=============================================================================
template <typename T>
T const & maniscalco::spsc_queue<T>::front
(
) const
{
    return queue_[front_];
}


//=============================================================================
template <typename T>
std::tuple<bool, std::size_t> maniscalco::spsc_queue<T>::pop
(
    T & destination
)
{
    if (auto s = size(); s != 0)
    {
        std::size_t front = front_;
        destination = std::move(queue_[front]);
        if (front == capacity_)
            front_ = 0;
        else
            front_ = (front + 1); 
        return {true, s - 1};
    }
    return {false, 0};
}


//=============================================================================
template <typename T>
template <typename T_>
bool maniscalco::spsc_queue<T>::push
(
    T_ && value
)
{
    if (auto s = size(); s < capacity_)
    {
        std::size_t back = back_;
        queue_[back] = std::forward<T_>(value);
        if (back == capacity_)
            back_ = 0;
        else
            back_ = (back + 1);
        if (s == 0)
            eventHandlers_.selectHandler_();
        return true;
    }
    return false;
}


//=============================================================================
template <typename T>
auto maniscalco::spsc_queue<T>::capacity
(
) const
{
    return capacity_;
}

#include "./buffer_allocator.h"


namespace 
{
   maniscalco::network::buffer invalid_buffer;
}


//==============================================================================
maniscalco::network::buffer_allocator::buffer_allocator
(
    configuration const & config
):
    capacity_(config.capacity_ ? config.capacity_ : default_buffer_capacity),
    bufferPool_()
{
    if (!initialize(config))
        capacity_ = 0;
}


//==============================================================================
auto maniscalco::network::buffer_allocator::capacity
(
) const -> size_type
{
    return capacity_;
}


//==============================================================================
auto maniscalco::network::buffer_allocator::begin
(
) -> iterator
{
    return bufferPool_.get();
}


//==============================================================================
auto maniscalco::network::buffer_allocator::begin
(
) const -> const_iterator
{
    return bufferPool_.get();
}


//==============================================================================
auto maniscalco::network::buffer_allocator::end
(
) -> iterator
{
    return (bufferPool_.get() + capacity_);
}


//==============================================================================
auto maniscalco::network::buffer_allocator::end
(
) const -> const_iterator
{
    return (bufferPool_.get() + capacity_);
}


//==============================================================================
bool maniscalco::network::buffer_allocator::initialize
(
    configuration const & config
) 
{
    buffer * allocation = nullptr;
    auto spaceRequired = (sizeof(element_type) * capacity_);
    auto posixMemAlignResult = posix_memalign((void **)&allocation, config.bufferPoolAlignment_, spaceRequired);
    if (posixMemAlignResult != 0)
        return false;
    bufferPool_ = std::unique_ptr<buffer [], std::function<void(buffer *)>>(allocation, 
            [this](auto * allocation){::free(allocation);});
    // initialize the buffers with placement new
    buffer_id id(0);
    for (auto i = 0; i < capacity_; ++i)    
        new (&bufferPool_[i]) element_type(this, id++);

    pushBitMask_ = ((capacity_ * 2) - 1);
    availableQueue_ = std::move(std::unique_ptr<element_type * []>(new element_type * [capacity_ * 2]));
    for (auto i = 0; i < capacity_; ++i)
        availableQueue_[pushIndex_++ & pushBitMask_] = get_buffer(i);
    return true;
}

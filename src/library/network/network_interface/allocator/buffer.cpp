#include "./buffer.h"


//==============================================================================
maniscalco::network::buffer::buffer
(
    buffer_allocator * bufferAllocator,
    buffer_id id
):
    header_({.id_ = id, .bufferAllocator_ = bufferAllocator})
{
}

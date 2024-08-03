#include <sql-engine/allocator.h>
#include <stdlib.h>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <utility>

ArenaAllocator::ArenaAllocator(const std::size_t max_num_bytes): m_size { max_num_bytes }
    , m_buffer { new std::size_t[max_num_bytes] }
    , m_offset { m_buffer }
{}


ArenaAllocator::ArenaAllocator(ArenaAllocator&& other) noexcept
    : m_size { std::exchange(other.m_size, 0) }
    , m_buffer { std::exchange(other.m_buffer, nullptr) }
    , m_offset { std::exchange(other.m_offset, nullptr) }
{}

ArenaAllocator& ArenaAllocator::operator=(ArenaAllocator&& other) noexcept
{
    std::swap(m_size, other.m_size);
    std::swap(m_buffer, other.m_buffer);
    std::swap(m_offset, other.m_offset);
    return *this;
}





ArenaAllocator::~ArenaAllocator()
{
    // No destructors are called for the stored objects. Thus, memory
    // leaks are possible (e.g. when storing std::vector objects or
    // other non-trivially destructable objects in the allocator).
    // Although this could be changed, it would come with additional
    // runtime overhead and therefore is not implemented.
    delete[] m_buffer;
}
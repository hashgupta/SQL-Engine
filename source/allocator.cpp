#include <sql-engine/allocator.h>
#include <stdlib.h>

ArenaAllocator::ArenaAllocator(unsigned int bytes)
    : m_size(bytes)
{
    m_buffer = static_cast<unsigned int*>(malloc(m_size));
    m_offset = m_buffer;
}

ArenaAllocator::~ArenaAllocator()
{
    free(m_buffer);
}
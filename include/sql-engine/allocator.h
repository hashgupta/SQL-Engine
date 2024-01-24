#pragma once

class ArenaAllocator {
    public:
    
    explicit ArenaAllocator(unsigned int bytes);

    template <typename T>
    T* alloc() {
        void* offset = m_offset;
        m_offset += sizeof(T);
        return static_cast<T*>(offset);
    }

    ArenaAllocator(const ArenaAllocator& other);

    ~ArenaAllocator();
    
    private:
        unsigned int m_size;
        unsigned int* m_buffer;
        unsigned int* m_offset;
};
#pragma once
#include "storage_info.hpp"

// storage allocated on the heap
template <typename T>
struct storage {
    constexpr storage(unsigned size) :
        m_on_host(true),
        m_cpu_ptr(new T[size]),
        m_gpu_ptr(new T[size]) //this should be implemented as cuda ptr, ...
    {}

    ~storage() { 
        delete [] m_cpu_ptr; 
        delete [] m_gpu_ptr;
    }

    void clone_to_device() const { m_on_host = false; }
    void clone_from_device() const { m_on_host = true; }
    bool is_on_host() const { return m_on_host; }

    T* get_cpu_ptr() const { return m_cpu_ptr; }
    T* get_gpu_ptr() const { return m_gpu_ptr; }

    bool m_on_host;
    T* m_cpu_ptr;
    T* m_gpu_ptr;
    
private:
    storage(storage const&);
    storage& operator=(storage const&);
};

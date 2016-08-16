#pragma once
#include "storage_info.hpp"

// storage allocated on the heap
template <typename T>
struct storage {
    constexpr storage(unsigned size) :
        m_valid_on_host(true),
        m_valid_on_device(false),
        m_cpu_ptr(new T[size]),
        m_gpu_ptr(new T[size]) //this should be implemented as cuda ptr, ...
    {}

    ~storage() { 
        delete [] m_cpu_ptr; 
        delete [] m_gpu_ptr;
    }

    void clone_to_device() { 
        if(!m_valid_on_device) {
            // perform a memcpy
        }
        m_valid_on_host = false;
        m_valid_on_device = true; 
    }
    void clone_from_device() { 
        if(!m_valid_on_host) {
            // perform a memcpy
        }
        m_valid_on_host = true;
        m_valid_on_device = false; 
    }
    void view_on_host() { 
        if(!m_valid_on_host) {
            // perform a memcpy
            m_valid_on_host = true; 
        }
    }
    void view_on_device() { 
        if(!m_valid_on_device) {
            // perform a memcpy
            m_valid_on_device = true; 
        }
    }
    
    bool is_valid_on_host() const { return m_valid_on_host; }
    bool is_valid_on_device() const { return m_valid_on_device; }

    T* get_cpu_ptr() const { return m_cpu_ptr; }
    T* get_gpu_ptr() const { return m_gpu_ptr; }

    bool m_valid_on_device, m_valid_on_host;
    T* m_cpu_ptr;
    T* m_gpu_ptr;

private:
    storage(storage const&);
    storage& operator=(storage const&);
};


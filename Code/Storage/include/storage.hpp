#pragma once
#include <iostream>

#include "storage_info.hpp"

struct state_machine {
    bool m_hnu, m_dnu, m_od;
    state_machine() : m_hnu(false), m_dnu(false), m_od(false) {}
};

// storage allocated on the heap
template <typename T>
struct storage {
    constexpr storage(unsigned size) :
        m_cpu_ptr(new T[size]),
        m_gpu_ptr(new T[size]) //this should be implemented as cuda ptr, ...
    {}

    ~storage() { 
        delete [] m_cpu_ptr; 
        delete [] m_gpu_ptr;
    }

    void clone_to_device() { 
        // do a memcpy
        std::cout << "memcopy h->d performed...\n";
        m_state_machine.m_od = true;
        m_state_machine.m_hnu = false;
        m_state_machine.m_dnu = false;        
    }

    void clone_from_device() {
        // do a memcpy
        std::cout << "memcopy d->h performed...\n";
        m_state_machine.m_od = false;
        m_state_machine.m_hnu = false;
        m_state_machine.m_dnu = false;
    }
    
    void sync() {
        if(!m_state_machine.m_hnu && !m_state_machine.m_dnu) return;
        assert((m_state_machine.m_hnu ^ m_state_machine.m_dnu) && "invalid state detected.");
        (m_state_machine.m_hnu) ? clone_from_device() : clone_to_device();
    }

    bool is_valid_on_host() const { return !m_state_machine.m_od; }
    bool is_valid_on_device() const { return m_state_machine.m_od; }
    bool device_needs_update() const { return m_state_machine.m_dnu; }
    bool host_needs_update() const { return m_state_machine.m_hnu; }
    void reactivate_device_write_views() { assert(!m_state_machine.m_dnu && "host views are in write mode"); m_state_machine.m_hnu = 1; m_state_machine.m_od = 1; }
    void reactivate_host_write_views() { assert(!m_state_machine.m_hnu && "device views are in write mode"); m_state_machine.m_dnu = 1; m_state_machine.m_od = 0; }
    state_machine* get_state_machine_ptr() { return &m_state_machine; }

    T* get_cpu_ptr() const { return m_cpu_ptr; }
    T* get_gpu_ptr() const { return m_gpu_ptr; }

    state_machine m_state_machine;
    T* m_cpu_ptr;
    T* m_gpu_ptr;

private:
    storage(storage const&);
    storage& operator=(storage const&);
};


#pragma once
#include <memory>
#include <iostream>
#include <cassert>

#include "storage.hpp"
#include "storage_info.hpp"

template <typename T, typename MetaData>
struct data_handler {
    std::shared_ptr<storage<T> > s;
    std::shared_ptr<MetaData> m;

    // This represents a host view to a storage
    // It contains a pointer to the storage class, performs safety checks,
    // and provides means to modify the contained data
    struct host_view_t {
        storage<T>* s;
        MetaData* m;
        template <typename... N>
        T& operator()(N... n) { 
            assert(s->is_on_host() && "Hey, data is not on host!");  
            return s->m_cpu_ptr[m->index(n...)]; 
        }
    };

    // This represents a device view to a storage
    // It contains a device pointer of the data,
    // and provides means to modify the contained data
    struct device_view_t {
        T* s;
        MetaData* m;
        template <typename... N>
        T& operator()(N... n) {
            return s[m->index(n...)]; 
        }
    };

    // simple getter for host_view
    host_view_t host_view() const {
        return { s.get(), m.get() };
    }
    // simple getter for device_view
    device_view_t device_view() const {
        return { s.get()->m_gpu_ptr, m.get() };
    }

    void clone_to_device() const {
        s->clone_to_device();
    }

    void clone_from_device() const {
        s->clone_from_device();
    }

    constexpr data_handler(MetaData meta) : s(new storage<T>(27)), m(new MetaData(meta)) { }
};


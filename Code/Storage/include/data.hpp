#pragma once
#include <memory>
#include <iostream>
#include <cassert>

#include "storage.hpp"
#include "storage_info.hpp"

template <typename T, typename MetaData>
struct data {
    using data_t = T;
    using meta_data_t = MetaData;

    constexpr data(MetaData meta) : s(new storage<T>(meta.size())), m(new MetaData(meta)) { }

    std::shared_ptr<storage<T> > s;
    std::shared_ptr<MetaData> m;

    void clone_to_device() const {
        s->clone_to_device();
    }

    void clone_from_device() const {
        s->clone_from_device();
    }

    bool is_on_host() const {
        return s->is_on_host();
    }

    template <typename... N> 
    T& operator()(N... n) {
        return s->get_cpu_ptr()[m->index(n...)];
    }

    /*****************************DEVICE SUPPORT***************************************/
    // This represents a device view to a storage.
    // It contains a device pointer of the data,
    // and provides means to modify the contained data
    struct device_view {
        T* s;
        MetaData* m;
        template <typename... N>
        T& operator()(N... n) {
            return s[m->index(n...)]; 
        }
    };

    // simple getter for device_view
    device_view get_device_view() const {
        return { s->get_gpu_ptr(), m.get() };
    }
    /*********************************************************************************/
};


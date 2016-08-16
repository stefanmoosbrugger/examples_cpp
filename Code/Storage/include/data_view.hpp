#pragma once
#include "data.hpp"
#include "storage_info_sol.hpp"

template <typename MetaData, typename DataType>
struct data_view {
    DataType* d;
    MetaData const* md;
    constexpr data_view(DataType* ptr, MetaData const* meta) : d(ptr), md(meta) { }

    template <typename... N>
    DataType& operator()(N... n) {
        return d[md->index(n...)];
    }
};

template <typename T>
data_view<typename T::meta_data_t, typename T::data_t> make_device_view(T const& data) {
    return data_view<typename T::meta_data_t, typename T::data_t>(data.s->get_gpu_ptr(), data.m.get());
}

template <typename T>
data_view<typename T::meta_data_t, typename T::data_t> make_host_view(T const& data) {
    return data_view<typename T::meta_data_t, typename T::data_t>(data.s->get_cpu_ptr(), data.m.get());
}

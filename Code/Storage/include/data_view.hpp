#pragma once
#include "data.hpp"
#include "storage.hpp"
#include "storage_info_sol.hpp"

template <typename MetaData, typename DataType, bool DeviceView = false, bool ReadOnly = false>
struct data_view {
    DataType* d;
    MetaData const* md;
    state_machine const* s;
    constexpr data_view(DataType* ptr, MetaData const* meta, state_machine const* state) : d(ptr), md(meta), s(state) { }

    template <typename T = DataType, typename... N>
    typename std::enable_if<ReadOnly, T const&>::type operator()(N... n) {
        return d[md->index(n...)];
    }

    template <typename T = DataType, typename... N>
    typename std::enable_if<!ReadOnly, T&>::type operator()(N... n) {
        return d[md->index(n...)];
    }

    template <typename T = bool>
    typename std::enable_if<ReadOnly, T>::type valid() const {
        return DeviceView ? (!(s->m_dnu)) : (!(s->m_hnu));
    }

    template <typename T = bool>
    typename std::enable_if<!ReadOnly, T>::type valid() const {
        return DeviceView ? (s->m_od && s->m_hnu) : (!(s->m_od) && s->m_dnu);
    }
    
};

template <typename T, bool ReadOnly = false>
data_view<typename T::meta_data_t, typename T::data_t, true, ReadOnly> make_device_view(T const& data) {
    // set hnu == 1
    data.s->m_state_machine.m_hnu = (data.s->m_state_machine.m_hnu || !ReadOnly);
    //if(!ReadOnly) data.s->m_state_machine.m_hnu = true;
    return data_view<typename T::meta_data_t, typename T::data_t, true, ReadOnly>(data.s->get_gpu_ptr(), data.m.get(), data.s->get_state_machine_ptr());
}

template <typename T, bool ReadOnly = false>
data_view<typename T::meta_data_t, typename T::data_t, false, ReadOnly> make_host_view(T const& data) {
    // set dnu == 1
    data.s->m_state_machine.m_dnu = (data.s->m_state_machine.m_dnu || !ReadOnly);
    //if(!ReadOnly) data.s->m_state_machine.m_dnu = true;
    return data_view<typename T::meta_data_t, typename T::data_t, false, ReadOnly>(data.s->get_cpu_ptr(), data.m.get(), data.s->get_state_machine_ptr());
}

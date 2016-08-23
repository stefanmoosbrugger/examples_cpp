#pragma once

#include <boost/mpl/if.hpp>
#include <boost/utility.hpp>

#include "data.hpp"
#include "storage.hpp"
#include "storage_info_sol.hpp"

template <typename MetaData, typename DataType, bool DeviceView = false, bool ReadOnly = false>
struct data_view {
    const static bool device_view = DeviceView;
    const static bool read_only = ReadOnly;

    DataType* d;
    MetaData const* md;
    constexpr data_view(DataType* ptr, MetaData const* meta) : d(ptr), md(meta) {}

    template <typename... N>
    typename boost::mpl::if_c<ReadOnly, DataType const&, DataType&>::type
    operator()(N... n) {
        return d[md->index(n...)];
    }    
};

template <typename T, bool ReadOnly = false>
data_view<typename T::meta_data_t, typename T::data_t, true, ReadOnly> make_device_view(T const& data) {
    // set host needs update because we generate a device side rw-view 
    if(!ReadOnly) data.s->get_state_machine_ptr()->m_hnu = true;
    return data_view<typename T::meta_data_t, typename T::data_t, true, ReadOnly>(data.s->get_gpu_ptr(), data.m.get());
}

template <typename T, bool ReadOnly = false>
data_view<typename T::meta_data_t, typename T::data_t, false, ReadOnly> make_host_view(T const& data) {
    // set device needs update because we generate a host side rw-view 
    if(!ReadOnly) data.s->get_state_machine_ptr()->m_dnu = true;
    return data_view<typename T::meta_data_t, typename T::data_t, false, ReadOnly>(data.s->get_cpu_ptr(), data.m.get());
}

template <typename T>
struct is_data_view : boost::mpl::false_ {};

template <typename MetaData, typename DataType, bool DeviceView, bool ReadOnly>
struct is_data_view< data_view<MetaData, DataType, DeviceView, ReadOnly> > : boost::mpl::true_ {};

template <typename T, typename DV>
typename boost::enable_if<is_data_view<DV>, bool>::type valid(T const& d, DV const& v) {
    if(DV::read_only) return DV::device_view ? !d.s->get_state_machine_ptr()->m_dnu : !d.s->get_state_machine_ptr()->m_hnu;
    bool hnu = d.s->get_state_machine_ptr()->m_hnu;
    bool dnu = d.s->get_state_machine_ptr()->m_dnu;
    bool od = d.s->get_state_machine_ptr()->m_od;
    return DV::device_view ? (hnu && !dnu && od) : (!hnu && dnu && !od);  
}
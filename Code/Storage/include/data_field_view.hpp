#pragma once

#include <boost/utility.hpp>

#include "data.hpp"
#include "data_field.hpp"
#include "data_view.hpp"
#include "storage_info_sol.hpp"

template <typename MetaData, typename DataType, unsigned N, unsigned Dims, bool DeviceView = false, bool ReadOnly = false>
struct data_field_view {
    using this_t = data_field_view<MetaData, DataType, N, Dims, DeviceView, ReadOnly>;
    DataType* ptrs[N];
    MetaData const* meta;
    unsigned offsets[Dims];

    constexpr data_field_view(DataType* p[N], MetaData* m, unsigned o[Dims]) : meta(m) {
        for(unsigned i=0; i<N; ++i) ptrs[i] = p[i];
        for(unsigned i=0; i<Dims; ++i) offsets[i] = o[i];
    }

    template<unsigned Dim, unsigned Snapshot>
    data_view<MetaData, DataType, DeviceView, ReadOnly> get() {
        return data_view<MetaData, DataType, DeviceView, ReadOnly>(ptrs[offsets[Dim]+Snapshot], meta);
    }

    data_view<MetaData, DataType, DeviceView, ReadOnly> get(unsigned Dim, unsigned Snapshot) {
        return data_view<MetaData, DataType, DeviceView, ReadOnly>(ptrs[offsets[Dim]+Snapshot], meta);
    }

    // returns a ref or const-ref to an element of storage at <Dim,Snapshot>
    template<unsigned Dim, unsigned Snapshot, typename... Coords>
    typename boost::mpl::if_c<ReadOnly, DataType const&, DataType&>::type 
    get_value(Coords... c) {
        return get<Dim,Snapshot>()(c...);
    }

};

template <typename T, bool ReadOnly = false>
data_field_view<typename T::meta_data_t, typename T::data_t, T::size, T::dims, true, ReadOnly> make_device_field_view(T const& data) {
    typename T::data_t* ptrs[T::size];
    unsigned offsets[T::dims];
    unsigned offset = 0;
    unsigned cnt = 0;
    execute_lambda_on_tuple(data.f, [&](auto& tuple_elem) {
        for(unsigned i=0; i<tuple_elem.size(); ++i) {
            ptrs[offset+i] = tuple_elem[i].s->get_gpu_ptr();
            if(!ReadOnly) tuple_elem[i].s->get_state_machine_ptr()->m_hnu = true;
        }
        offsets[cnt++] = offset;
        offset += tuple_elem.size();
    });
    return data_field_view<typename T::meta_data_t, typename T::data_t, T::size, T::dims, true, ReadOnly>(ptrs, std::get<0>(data.f)[0].m.get(), offsets);
}

template <typename T, bool ReadOnly = false>
data_field_view<typename T::meta_data_t, typename T::data_t, T::size, T::dims, false, ReadOnly> make_host_field_view(T const& data) {
    typename T::data_t* ptrs[T::size];
    unsigned offsets[T::dims];
    unsigned offset = 0;
    unsigned cnt = 0;
    execute_lambda_on_tuple(data.f, [&](auto& tuple_elem) {
        for(unsigned i=0; i<tuple_elem.size(); ++i) {
            ptrs[offset+i] = tuple_elem[i].s->get_cpu_ptr();
            if(!ReadOnly) tuple_elem[i].s->get_state_machine_ptr()->m_dnu = true;
        }
        offsets[cnt++] = offset;
        offset += tuple_elem.size();
    });
    return data_field_view<typename T::meta_data_t, typename T::data_t, T::size, T::dims, false, ReadOnly>(ptrs, std::get<0>(data.f)[0].m.get(), offsets);
}

template <typename T>
struct is_data_field_view : boost::mpl::false_ {};

template <typename MetaData, typename DataType, unsigned N, unsigned Dims, bool DeviceView, bool ReadOnly>
struct is_data_field_view< data_field_view<MetaData, DataType, N, Dims, DeviceView, ReadOnly> > : boost::mpl::true_ {};

template <typename T, typename DFV>
typename boost::enable_if<is_data_field_view<DFV>, bool>::type valid(T const& data, DFV const& v) {
    // iterate over all containers and call valid
    bool res = true;
    unsigned Dim = 0;
    execute_lambda_on_tuple(data.f, [&](auto& tuple_elem) {
        for(unsigned i=0; i<tuple_elem.size(); ++i) {
            res &= valid(tuple_elem[i], v.get(Dim, i));
        }
        Dim++;
    });
    return res;
}

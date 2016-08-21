#pragma once
#include "data.hpp"
#include "data_field.hpp"
#include "data_view.hpp"
#include "storage_info_sol.hpp"

template <typename MetaData, typename DataType, unsigned N, unsigned Dims, bool DeviceView = false, bool ReadOnly = false>
struct data_field_view {
    using this_t = data_field_view<MetaData, DataType, N, Dims, DeviceView, ReadOnly>;
    DataType* ptrs[N];
    state_machine* sms[N];
    MetaData const* meta;
    unsigned offsets[Dims];

    constexpr data_field_view(DataType* p[N], state_machine* s[N], MetaData* m, unsigned o[Dims]) : meta(m) {
        for(unsigned i=0; i<N; ++i) ptrs[i] = p[i];
        for(unsigned i=0; i<N; ++i) sms[i] = s[i];
        for(unsigned i=0; i<Dims; ++i) offsets[i] = o[i];
    }

    template<unsigned Dim, unsigned Snapshot>
    data_view<MetaData, DataType, DeviceView, ReadOnly> get() {
        return data_view<MetaData, DataType, DeviceView, ReadOnly>(ptrs[offsets[Dim]+Snapshot], meta, sms[offsets[Dim]+Snapshot]);
    }

    // returns a ref or const-ref to an element of storage at <Dim,Snapshot>
    template<unsigned Dim, unsigned Snapshot, typename... Coords>
    auto get_value(Coords... c) -> decltype(get<Dim,Snapshot>()(c...)) {
        return get<Dim,Snapshot>()(c...);
    }

};


template <typename T, bool ReadOnly = false>
data_field_view<typename T::meta_data_t, typename T::data_t, T::size, T::dims, true, ReadOnly> make_device_field_view(T const& data) {
    typename T::data_t* ptrs[T::size];
    unsigned offsets[T::dims];
    state_machine* states[T::size];
    unsigned offset = 0;
    unsigned cnt = 0;
    execute_lambda_on_tuple(data.f, [&](auto& tuple_elem) {
        for(unsigned i=0; i<tuple_elem.size(); ++i) {
            ptrs[offset+i] = tuple_elem[i].s->get_gpu_ptr();
            states[offset+i] = tuple_elem[i].s->get_state_machine_ptr();
        }
        offsets[cnt++] = offset;
        offset += tuple_elem.size();        
    });
    return data_field_view<typename T::meta_data_t, typename T::data_t, T::size, T::dims, true, ReadOnly>(ptrs, states, std::get<0>(data.f)[0].m.get(), offsets);
}

template <typename T, bool ReadOnly = false>
data_field_view<typename T::meta_data_t, typename T::data_t, T::size, T::dims, false, ReadOnly> make_host_field_view(T const& data) {
    typename T::data_t* ptrs[T::size];
    unsigned offsets[T::dims];
    state_machine* states[T::size];
    unsigned offset = 0;
    unsigned cnt = 0;
    execute_lambda_on_tuple(data.f, [&](auto& tuple_elem) {
        for(unsigned i=0; i<tuple_elem.size(); ++i) {
            ptrs[offset+i] = tuple_elem[i].s->get_cpu_ptr();
            states[offset+i] = tuple_elem[i].s->get_state_machine_ptr();
        }
        offsets[cnt++] = offset;
        offset += tuple_elem.size();        
    });
    return data_field_view<typename T::meta_data_t, typename T::data_t, T::size, T::dims, false, ReadOnly>(ptrs, states, std::get<0>(data.f)[0].m.get(), offsets);
}

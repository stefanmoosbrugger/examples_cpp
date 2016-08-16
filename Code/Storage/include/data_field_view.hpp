#pragma once
#include "data.hpp"
#include "data_field.hpp"
#include "data_view.hpp"
#include "storage_info_sol.hpp"

template <typename MetaData, typename DataType, unsigned N, unsigned Dims>
struct data_field_view {
    DataType* ptrs[N];
    MetaData const* meta;
    unsigned offsets[Dims];

    constexpr data_field_view(DataType* p[N], MetaData* m, unsigned o[Dims]) : meta(m) {
        for(unsigned i=0; i<N; ++i) ptrs[i] = p[i];
        for(unsigned i=0; i<Dims; ++i) offsets[i] = o[i];
    }

    template<unsigned Dim, unsigned Snapshot>
    data_view<MetaData, DataType> get() {
        return data_view<MetaData, DataType>(ptrs[offsets[Dim]+Snapshot], meta);
    }

    template<unsigned Dim, unsigned Snapshot, typename... Coords>
    DataType& get_value(Coords... c) {
        return data_view<MetaData, DataType>(ptrs[offsets[Dim]+Snapshot], meta)(c...);
    }
    
};


template <typename T>
data_field_view<typename T::meta_data_t, typename T::data_t, T::size, T::dims> make_device_field_view(T const& data) {
    typename T::data_t* ptrs[T::size];
    unsigned offsets[T::dims];
    unsigned offset = 0;
    unsigned cnt = 0;
    execute_lambda_on_tuple(data.f, [&](auto& tuple_elem) {
        for(unsigned i=0; i<tuple_elem.size(); ++i) {
            ptrs[offset+i] = tuple_elem[i].s->get_gpu_ptr();
        }
        offsets[cnt++] = offset;
        offset += tuple_elem.size();        
    });
    return data_field_view<typename T::meta_data_t, typename T::data_t, T::size, T::dims>(ptrs, std::get<0>(data.f)[0].m.get(), offsets);
}

template <typename T>
data_field_view<typename T::meta_data_t, typename T::data_t, T::size, T::dims> make_host_field_view(T const& data) {
    typename T::data_t* ptrs[T::size];
    unsigned offsets[T::dims];
    unsigned offset = 0;
    unsigned cnt = 0;
    execute_lambda_on_tuple(data.f, [&](auto& tuple_elem) {
        for(unsigned i=0; i<tuple_elem.size(); ++i) {
            ptrs[offset+i] = tuple_elem[i].s->get_cpu_ptr();
        }
        offsets[cnt++] = offset;
        offset += tuple_elem.size();        
    });
    return data_field_view<typename T::meta_data_t, typename T::data_t, T::size, T::dims>(ptrs, std::get<0>(data.f)[0].m.get(), offsets);
}

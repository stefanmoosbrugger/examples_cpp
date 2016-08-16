#pragma once
#include <array>
#include <memory>
#include <tuple>
#include <iostream>

#include "data.hpp"

namespace {
    /* Helper struct for initializing an std tuple of arrays */    
    template <unsigned N, typename... Seq>
    struct get_array_seq {
        typedef typename get_array_seq<N-1, int, Seq...>::res res;
    };
 
    template <typename... Seq>
    struct get_array_seq<0,Seq...> {
        typedef get_array_seq<0,Seq...> res;
        template <typename T, typename MetaData, typename N>
        static constexpr data<T, MetaData> get_data(MetaData m) {
            return data<T,MetaData>(m);
        }

        template <typename T, typename MetaData>
        static constexpr std::array< data<T,MetaData>, sizeof...(Seq) > get_data_array(MetaData m) {
            return { get_data<T, MetaData, Seq>(m)... };
        }
    };

    /* Given a data_field<T, MetaData, X...> this function will accumulate X... until a given point (N). */
    template <int N, unsigned First, unsigned... Dims>
    struct get_accumulated_data_field_index {
       static const unsigned value = First + get_accumulated_data_field_index<N-1, Dims..., First>::value;
    };

    template <unsigned First, unsigned... Dims>
    struct get_accumulated_data_field_index<0, First, Dims...> {
        static const unsigned value = 0;
    };

    /* Helper struct that is needed to iterate over an std tuple of arrays in order to initialize a stack array. */
    template<typename Lambda, std::size_t I = 0, typename... Tp>
    inline typename std::enable_if<I == sizeof...(Tp), void>::type
    execute_lambda_on_tuple(std::tuple<Tp...> const& t, Lambda const& l) { }

    template<typename Lambda, std::size_t I = 0, typename... Tp>
    inline typename std::enable_if<I < sizeof...(Tp), void>::type
    execute_lambda_on_tuple(std::tuple<Tp...> const& t, Lambda const& l) { 
        l(std::get<I>(t));
        execute_lambda_on_tuple<Lambda, I+1, Tp...>(t, l);
    }
        
}


template <typename T, typename MetaData, unsigned... N>
struct data_field {
    using data_t = T;
    using meta_data_t = MetaData;
    const static unsigned size = get_accumulated_data_field_index<sizeof...(N), N...>::value; 
    const static unsigned dims = sizeof...(N); 

    // tuple of arrays (e.g., { {s00,s01,s02}, {s10, s11}, {s20} }, 3-dimensional field with snapshot sizes 3, 2, and 1. All together we have 6 storages.)
    std::tuple< std::array<data<T, MetaData >, N>... > f;
    constexpr data_field(MetaData m) : f(get_array_seq<N>::res::template get_data_array<T,MetaData>(m)...) { }

    template <unsigned Dim, unsigned Snapshot>
    data<T, MetaData>& get() {
        return std::get<Dim>(f)[Snapshot];
    } 

    template <unsigned Dim, unsigned Snapshot>
    void set(data<T, MetaData>& dh) {
        if(!std::get<Dim>(f)[Snapshot].is_valid_on_host()) std::cout << "WARNING: Data field element that is on device gets overridden.\n";
        std::get<Dim>(f)[Snapshot] = dh;
    }

    // clone all storages in a data field to the device
    void clone_to_device() const {
        execute_lambda_on_tuple(f, [](auto& tuple_elem) {
            for(unsigned i=0; i<tuple_elem.size(); ++i) {
                tuple_elem[i].clone_to_device();
            }
        });
    }

    // clone all storages in a data field from the device
    void clone_from_device() const {
        execute_lambda_on_tuple(f, [](auto& tuple_elem) {
            for(unsigned i=0; i<tuple_elem.size(); ++i) {
                tuple_elem[i].clone_from_device();
            }
        });
    }

    // clone specific storage in a data field from the device
    void clone_from_device(unsigned Dim, unsigned Snapshot) const {
        unsigned cnt = Dim;
        execute_lambda_on_tuple(f, [&](auto& tuple_elem) {
            if(cnt-- == 0) tuple_elem[Snapshot].clone_from_device();
        });
    }

    // clone specific storage in a data field from the device
    void clone_to_device(unsigned Dim, unsigned Snapshot) const {
        unsigned cnt = Dim;
        execute_lambda_on_tuple(f, [&](auto& tuple_elem) {
            if(cnt-- == 0) tuple_elem[Snapshot].clone_to_device();
        });
    }

    // make all storages in a data field valid on the host
    void view_on_host() const {
        execute_lambda_on_tuple(f, [](auto& tuple_elem) {
            for(unsigned i=0; i<tuple_elem.size(); ++i) {
                tuple_elem[i].view_on_host();
            }
        });
    }

    // make all storages in a data field valid on the device
    void view_on_device() const {
        execute_lambda_on_tuple(f, [](auto& tuple_elem) {
            for(unsigned i=0; i<tuple_elem.size(); ++i) {
                tuple_elem[i].view_on_device();
            }
        });
    }

    // make specific storage in a data field valid on the host
    void view_on_host(unsigned Dim, unsigned Snapshot) const {
        unsigned cnt = Dim;
        execute_lambda_on_tuple(f, [&](auto& tuple_elem) {
            if(cnt-- == 0) tuple_elem[Snapshot].view_on_host();
        });
    }

    // make specific storage in a data field valid on the device
    void view_on_device(unsigned Dim, unsigned Snapshot) const {
        unsigned cnt = Dim;
        execute_lambda_on_tuple(f, [&](auto& tuple_elem) {
            if(cnt-- == 0) tuple_elem[Snapshot].view_on_device();
        });
    }

};


/** 
 *  Implementation of a swap function. E.g., swap<0,0>::with<0,1>(field_view) will swap the CPU and GPU pointers of storages 0,0 and 0,1. 
 *  This operation invalidates the previously created device_views. host_views are not affected and will directly see the swap.
 **/
template <unsigned Dim_S, unsigned Snapshot_S>
struct swap {
    template <unsigned Dim_T, unsigned Snapshot_T, typename T, typename MetaData, unsigned... N>
    static void with(data_field<T, MetaData, N...>& data_field) {
        auto& src = data_field.template get<Dim_S,Snapshot_S>();
        auto& trg = data_field.template get<Dim_T,Snapshot_T>();
        assert(src.is_valid_on_host() && trg.is_valid_on_host() && "Either target or source element is not on the device.");
        auto* tmp_cpu = src.s->m_cpu_ptr;
        auto* tmp_gpu = src.s->m_gpu_ptr;
        src.s->m_cpu_ptr = trg.s->m_cpu_ptr;
        src.s->m_gpu_ptr = trg.s->m_gpu_ptr;
        trg.s->m_cpu_ptr = tmp_cpu;
        trg.s->m_gpu_ptr = tmp_gpu;
    }
};


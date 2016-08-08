#pragma once
#include <array>
#include <memory>
#include <tuple>
#include <iostream>

#include "data_handler.hpp"

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
        static constexpr data_handler<T, MetaData> get_data_handler(MetaData m) {
            return data_handler<T,MetaData>(m);
        }

        template <typename T, typename MetaData>
        static constexpr std::array< data_handler<T,MetaData>, sizeof...(Seq) > get_data_handler_array(MetaData m) {
            return { get_data_handler<T, MetaData, Seq>(m)... };
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
    template<typename Lambda, typename DataHandler, unsigned Offset = 0, std::size_t I = 0, typename... Tp>
    inline typename std::enable_if<I == sizeof...(Tp), void>::type
    fill_view(std::tuple<Tp...> const& t, DataHandler& h, Lambda const& l) { }

    template<typename Lambda, typename DataHandler, unsigned Offset = 0, std::size_t I = 0, typename... Tp>
    inline typename std::enable_if<I < sizeof...(Tp), void>::type
    fill_view(std::tuple<Tp...> const& t, DataHandler& h, Lambda const& l) {
        typedef typename std::remove_cv<typename std::remove_reference<decltype(std::get<I>(t))>::type >::type ty;
        const unsigned size = std::tuple_size<ty>::value;
        for(unsigned i=0; i<std::get<I>(t).size(); ++i) {
            h.s[i+Offset] = l(std::get<I>(t)[i]);
        }
        fill_view<Lambda, DataHandler, (Offset+std::tuple_size<ty>::value), I+1, Tp...>(t, h, l);
    }    
}


template <typename T, typename MetaData, unsigned... N>
struct data_field_handler {
    // typedefs to both host and device view for the given T and MetaData
    typedef typename data_handler<T, MetaData>::host_view_t storage_host_view;
    typedef typename data_handler<T, MetaData>::device_view_t storage_device_view;

    // tuple of arrays (e.g., { {s00,s01,s02}, {s10, s11}, {s20} }, 3-dimensional field with snapshot sizes 3, 2, and 1. All in all we have 6 storages.)
    std::tuple< std::array<data_handler<T, MetaData >, N>... > f; 
    constexpr data_field_handler(MetaData m) : f(get_array_seq<N>::res::template get_data_handler_array<T,MetaData>(m)...) { }

    template <typename S>
    struct view_t {
        S s[get_accumulated_data_field_index<sizeof...(N), N...>::value];
        
        template <unsigned Dim, unsigned Snapshot>
        S& get() {
            return s[get_accumulated_data_field_index<Dim, N...>::value + Snapshot];
        }
    };

    // simple getter for a data field device view
    view_t<storage_device_view> device_view() const {
        view_t<storage_device_view> dv;
        fill_view(f, dv, [](auto& view) { return view.device_view(); });
        return dv;
    }

    // simple getter for a data field host view
    view_t<storage_host_view> host_view() const {
        view_t<storage_host_view> hv;
        fill_view(f, hv, [](auto& view) { return view.host_view(); });
        return hv;
    }

};


        template <unsigned Dim_S, unsigned Snapshot_S>
        struct swap {
            template <unsigned Dim_T, unsigned Snapshot_T, typename T>
            static void with(T& view) {
                auto& src = view.template get<Dim_S,Snapshot_S>();
                auto& trg = view.template get<Dim_T,Snapshot_T>();
                auto* tmp_cpu = src.s->m_cpu_ptr;
                auto* tmp_gpu = src.s->m_gpu_ptr;
                src.s->m_cpu_ptr = trg.s->m_cpu_ptr;
                src.s->m_gpu_ptr = trg.s->m_gpu_ptr;
                trg.s->m_cpu_ptr = tmp_cpu;
                trg.s->m_gpu_ptr = tmp_gpu;
            }
        };


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

    void clone_to_device() const { s->clone_to_device(); }
    void clone_from_device() const { s->clone_from_device(); }
    void view_on_host() const { s->view_on_host(); }
    void view_on_device() const { s->view_on_device(); }
    bool is_valid_on_host() const { return s->is_valid_on_host(); }
    bool is_valid_on_device() const { return s->is_valid_on_device(); }
};


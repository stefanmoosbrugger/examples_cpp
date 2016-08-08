#include "gtest/gtest.h"

#include "data_handler.hpp"
#include "storage.hpp"
#include "storage_info_sol.hpp"
#include "data_field_handler.hpp"

TEST(DataFieldTest, Basic) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    // create a storage info type
    constexpr storage_info_t si {3,3,3};
    // create data field
    data_field_handler<double, storage_info_t, 2, 3, 4> field(si);
    // create a host_view for the data_field (contains 2*3*4 views)
    auto host_view = field.host_view();
    // access the first storage of the first dimension and set the first value to 5
    host_view.get<0,0>()(0,0,0) = 5;
    host_view.get<0,1>()(0,0,0) = -5;
    // manually get the view of the first storage element in the data view (equivalent to get<0,0>...)
    data_handler<double, storage_info_t> partial_view_1 = std::get<0>(field.f)[0];
    data_handler<double, storage_info_t> partial_view_2 = std::get<0>(field.f)[1];
    EXPECT_EQ(partial_view_1.host_view()(0,0,0), 5);
    EXPECT_EQ(partial_view_2.host_view()(0,0,0), -5);
    swap<0,0>::with<0,1>(host_view);
    EXPECT_EQ(partial_view_1.host_view()(0,0,0), -5);
    EXPECT_EQ(partial_view_2.host_view()(0,0,0), 5);    
}


TEST(DataFieldTest, Device) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    // create a storage info type
    constexpr storage_info_t si {3,3,3};
    // create data field
    data_field_handler<double, storage_info_t, 2, 3, 4> field(si);
    auto host_view = field.host_view();
    auto device_view = field.device_view();

    // access the first storage of the first dimension and set the first value to 5
    host_view.get<0,0>()(0,0,0) = 5;
    host_view.get<0,1>()(0,0,0) = -5;

    // manually clone one storage of the field to the device
    field.clone_to_device(0,0);
    // create another view and check if on_host flag is equivalent
    data_handler<double, storage_info_t> partial_view_1 = std::get<0>(field.f)[0];
    EXPECT_EQ((host_view.get<0,0>().s->is_on_host()), (partial_view_1.s->is_on_host()));
    EXPECT_NE((host_view.get<0,1>().s->is_on_host()), (partial_view_1.s->is_on_host()));
    field.clone_to_device(0,1);
    EXPECT_EQ((host_view.get<0,1>().s->is_on_host()), (partial_view_1.s->is_on_host()));

    // clone all storages in the field back to the host
    field.clone_from_device();
    EXPECT_EQ(true, (partial_view_1.s->is_on_host()));
    EXPECT_EQ(true, (host_view.get<0,0>().s->is_on_host()));
    EXPECT_EQ(true, (host_view.get<0,1>().s->is_on_host()));
    EXPECT_EQ((host_view.get<0,0>().s->is_on_host()), (partial_view_1.s->is_on_host()));
}


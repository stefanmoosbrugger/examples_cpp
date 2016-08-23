#include "gtest/gtest.h"

#include "data.hpp"
#include "storage.hpp"
#include "storage_info_sol.hpp"
#include "data_field.hpp"
#include "data_view.hpp"
#include "data_field_view.hpp"

TEST(DataFieldTest, Basic) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    // create a storage info type
    constexpr storage_info_t si {3,3,3};
    // create data field
    data_field<double, storage_info_t, 2, 3, 4> field(si);
    // access the first storage of the first dimension and set the first value to 5
    auto hv = make_host_field_view(field);
    hv.get<0,0>()(0,0,0) = 5;
    hv.get<0,1>()(0,0,0) = -5;
    // manually get the view of the first storage element in the data view (equivalent to get<0,0>...)
    data<double, storage_info_t> partial_1 = std::get<0>(field.f)[0];
    data<double, storage_info_t> partial_2 = std::get<0>(field.f)[1];
    auto hv1 = make_host_view(partial_1);
    auto hv2 = make_host_view(partial_2);
    EXPECT_EQ(hv1(0,0,0), 5);
    EXPECT_EQ(hv2(0,0,0), -5);
    swap<0,0>::with<0,1>(field);
    // we have to update the host view because we swapped the ptrs
    hv1 = make_host_view(partial_1);
    hv2 = make_host_view(partial_2);    
    EXPECT_EQ(hv1(0,0,0), -5);
    EXPECT_EQ(hv2(0,0,0), 5);    
}

TEST(DataFieldTest, DeviceCloning) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    // create a storage info type
    constexpr storage_info_t si {3,3,3};
    // create data field
    data_field<double, storage_info_t, 2, 3, 4> field(si);
    auto host_view = make_host_field_view(field);

    // access the first storage of the first dimension and set the first value to 5
    host_view.get<0,0>()(0,0,0) = 5;
    host_view.get<0,1>()(0,0,0) = -5;

    // manually clone one storage of the field to the device
    field.clone_to_device(0,0);
    // create another view and check if on_host flag is equivalent
    data<double, storage_info_t> partial_1 = std::get<0>(field.f)[0];
    EXPECT_EQ((field.get<0,0>().is_valid_on_host()), (partial_1.is_valid_on_host()));
    EXPECT_NE((field.get<0,1>().is_valid_on_host()), (partial_1.is_valid_on_host()));
    field.clone_to_device(0,1);
    EXPECT_EQ((field.get<0,1>().is_valid_on_host()), (partial_1.is_valid_on_host()));

    // clone all storages in the field back to the host
    field.clone_from_device();
    EXPECT_EQ(true, (partial_1.is_valid_on_host()));
    EXPECT_EQ(true, (field.get<0,0>().is_valid_on_host()));
    EXPECT_EQ(true, (field.get<0,1>().is_valid_on_host()));
    EXPECT_EQ((field.get<0,0>().is_valid_on_host()), (partial_1.is_valid_on_host()));
}

TEST(DataFieldTest, GetSetFieldElement) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    // create a storage info type
    constexpr storage_info_t si {3,3,3};
    // create data field
    data_field<double, storage_info_t, 2, 3, 4> field(si);

    // clone to device
    field.clone_to_device();
    auto field_elem = field.get<0,0>();

    // try to overwrite the field 0 in dimension 1 (check for warning message)
    data<double, storage_info_t> some_other_data(si);
    ASSERT_DEATH((field.set<1,0>(some_other_data)), "WARNING: Data field element gets overridden.");

    // clone from device and try the same
    field.clone_from_device();
    field.set<1,0>(some_other_data);
}

TEST(DataFieldTest, ViewValid) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    // create a storage info type
    constexpr storage_info_t si {3,3,3};
    // create data field
    data_field<double, storage_info_t, 1, 1> field(si);

    // create a host write and a read view to the field
    auto  hrv = make_host_field_view<decltype(field), true>(field);
    auto  hwv = make_host_field_view<decltype(field), false>(field);
    
    // get a write view to <0,0>
    auto hwv00 = hwv.get<0,0>();
    // get a read view to <1,0>
    auto hrv10 = hrv.get<1,0>();
    
    // all views should be valid
    EXPECT_TRUE(valid(field.get<0,0>(), hwv00));    
    EXPECT_TRUE(valid(field.get<1,0>(), hrv10));

    // launch a kernel, this might call clone_to_device for some/all storages.
    field.clone_to_device(0,0);
    EXPECT_FALSE(valid(field.get<0,0>(), hwv00));    
    EXPECT_TRUE(valid(field.get<1,0>(), hrv10));

    // create a device write view to a non cloned storage (<1,1>)
    auto dwv = make_device_field_view<decltype(field), false>(field);
    auto dwv10_tmp = dwv.get<1,0>();
    EXPECT_FALSE(valid(field.get<1,0>(), dwv10_tmp)); // device view is invalid because the data is not at the device currently
    
    field.clone_to_device(1,0);
    EXPECT_FALSE(valid(field.get<0,0>(), hwv00));    
    EXPECT_TRUE(valid(field.get<1,0>(), hrv10)); // still valid because there exists no device write view to this storage
    // create a device write view and make the read view invalid
    auto drv = make_device_field_view<decltype(field), true>(field);
    auto dwv10 = make_device_view(field.get<1,0>()); // if we want to have single data element handling we are forced create the data field views on our own
    EXPECT_FALSE(valid(field.get<0,0>(), hwv00));    
    EXPECT_FALSE(valid(field.get<1,0>(), hrv10));

    auto drv10 = drv.get<1,0>();
    EXPECT_TRUE(valid(field.get<1,2>(), drv10)); 

    field.sync();
    EXPECT_FALSE(valid(field.get<0,0>(), hwv00));    
    EXPECT_FALSE(valid(field.get<1,0>(), dwv10));    
    EXPECT_TRUE(valid(field.get<1,0>(), hrv10));
    EXPECT_TRUE(valid(field.get<1,1>(), drv10));

    field.get<0,0>().reactivate_host_write_views();
    EXPECT_TRUE(valid(field.get<0,0>(), hwv00));    
    field.get<1,0>().reactivate_device_write_views();
    EXPECT_TRUE(valid(field.get<1,0>(), dwv10));
}




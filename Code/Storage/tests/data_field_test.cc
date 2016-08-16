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
    EXPECT_EQ((field.get<0,0>().is_on_host()), (partial_1.is_on_host()));
    EXPECT_NE((field.get<0,1>().is_on_host()), (partial_1.is_on_host()));
    field.clone_to_device(0,1);
    EXPECT_EQ((field.get<0,1>().is_on_host()), (partial_1.is_on_host()));

    // clone all storages in the field back to the host
    field.clone_from_device();
    EXPECT_EQ(true, (partial_1.is_on_host()));
    EXPECT_EQ(true, (field.get<0,0>().is_on_host()));
    EXPECT_EQ(true, (field.get<0,1>().is_on_host()));
    EXPECT_EQ((field.get<0,0>().is_on_host()), (partial_1.is_on_host()));
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
    testing::internal::CaptureStdout();
    field.set<1,0>(some_other_data);
    std::string output = testing::internal::GetCapturedStdout();
    EXPECT_NE(output.find("WARNING: Data field element that is on device gets overridden."), std::string::npos);

    // clone from device and try the same
    field.clone_from_device();
    testing::internal::CaptureStdout();
    field.set<1,0>(field_elem);
    output.clear();
    output = testing::internal::GetCapturedStdout();
    EXPECT_EQ(output.find("WARNING: Data field element that is on device gets overridden."), std::string::npos);
}


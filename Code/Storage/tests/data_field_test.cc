#include "gtest/gtest.h"

#include "data.hpp"
#include "storage.hpp"
#include "storage_info_sol.hpp"
#include "data_field.hpp"

TEST(DataFieldTest, Basic) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    // create a storage info type
    constexpr storage_info_t si {3,3,3};
    // create data field
    data_field<double, storage_info_t, 2, 3, 4> field(si);
    // access the first storage of the first dimension and set the first value to 5
    field.get<0,0>()(0,0,0) = 5;
    field.get<0,1>()(0,0,0) = -5;
    // manually get the view of the first storage element in the data view (equivalent to get<0,0>...)
    data<double, storage_info_t> partial_view_1 = std::get<0>(field.f)[0];
    data<double, storage_info_t> partial_view_2 = std::get<0>(field.f)[1];
    EXPECT_EQ(partial_view_1(0,0,0), 5);
    EXPECT_EQ(partial_view_2(0,0,0), -5);
    swap<0,0>::with<0,1>(field);
    EXPECT_EQ(partial_view_1(0,0,0), -5);
    EXPECT_EQ(partial_view_2(0,0,0), 5);    
}

TEST(DataFieldTest, DeviceCloning) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    // create a storage info type
    constexpr storage_info_t si {3,3,3};
    // create data field
    data_field<double, storage_info_t, 2, 3, 4> field(si);
    auto device_view = field.get_device_view();

    // access the first storage of the first dimension and set the first value to 5
    field.get<0,0>()(0,0,0) = 5;
    field.get<0,1>()(0,0,0) = -5;

    // manually clone one storage of the field to the device
    field.clone_to_device(0,0);
    // create another view and check if on_host flag is equivalent
    data<double, storage_info_t> partial_view_1 = std::get<0>(field.f)[0];
    EXPECT_EQ((field.get<0,0>().is_on_host()), (partial_view_1.is_on_host()));
    EXPECT_NE((field.get<0,1>().is_on_host()), (partial_view_1.is_on_host()));
    field.clone_to_device(0,1);
    EXPECT_EQ((field.get<0,1>().is_on_host()), (partial_view_1.is_on_host()));

    // clone all storages in the field back to the host
    field.clone_from_device();
    EXPECT_EQ(true, (partial_view_1.is_on_host()));
    EXPECT_EQ(true, (field.get<0,0>().is_on_host()));
    EXPECT_EQ(true, (field.get<0,1>().is_on_host()));
    EXPECT_EQ((field.get<0,0>().is_on_host()), (partial_view_1.is_on_host()));
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


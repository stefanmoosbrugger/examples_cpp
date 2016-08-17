#include "gtest/gtest.h"

#include "data.hpp"
#include "storage.hpp"
#include "storage_info_sol.hpp"

TEST(StorageState, Simple) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    typedef data<double, storage_info_t> data_t;

    // create a storage info type
    storage_info_t si(3,3,3);
    // create a storage
    data_t s(si);

    // check initial state
    EXPECT_TRUE(s.is_valid_on_host());
    EXPECT_FALSE(s.is_valid_on_device());
    
    // simple clone to device
    s.clone_to_device();
    EXPECT_FALSE(s.is_valid_on_host());
    EXPECT_TRUE(s.is_valid_on_device());

    // simple clone from device
    s.clone_from_device();
    EXPECT_TRUE(s.is_valid_on_host());
    EXPECT_FALSE(s.is_valid_on_device());
}


TEST(StorageState, InvalidState) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    typedef data<double, storage_info_t> data_t;

    // create a storage info type
    storage_info_t si(3,3,3);
    // create a storage
    data_t s(si);

    // check initial state
    EXPECT_TRUE(s.is_valid_on_host());
    EXPECT_FALSE(s.is_valid_on_device());
    
    // invalid clone from device
    ASSERT_DEATH(s.clone_from_device(), "Cannot clone a storage from the device that is in an invalid state.");
    // invalid host view
    ASSERT_DEATH(s.view_on_host(), "Cannot create a read-only host view from a storage that is not valid on the device.");

    // clone the storage to the device
    s.clone_to_device();

    // invalid clone to device
    ASSERT_DEATH(s.clone_to_device(), "Cannot clone a storage to the device that is in an invalid state.");
    // invalid device view
    ASSERT_DEATH(s.view_on_device(), "Cannot create a read-only device view from a storage that is not valid on the host.");
}

TEST(StorageState, Views) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    typedef data<double, storage_info_t> data_t;

    // create a storage info type
    storage_info_t si(3,3,3);
    // create a storage
    data_t s(si);

    // check initial state
    EXPECT_TRUE(s.is_valid_on_host());
    EXPECT_FALSE(s.is_valid_on_device());
    
    // get a device view
    s.view_on_device();
    EXPECT_TRUE(s.is_valid_on_host());
    EXPECT_TRUE(s.is_valid_on_device());
    // again device view, no changes should happen
    s.view_on_device();
    EXPECT_TRUE(s.is_valid_on_host());
    EXPECT_TRUE(s.is_valid_on_device());

    // do a clone to device
    s.clone_to_device();
    EXPECT_FALSE(s.is_valid_on_host());
    EXPECT_TRUE(s.is_valid_on_device());

    // get a host view
    s.view_on_host();
    EXPECT_TRUE(s.is_valid_on_host());
    EXPECT_TRUE(s.is_valid_on_device());
    // again host view, no changes should happen
    s.view_on_host();
    EXPECT_TRUE(s.is_valid_on_host());
    EXPECT_TRUE(s.is_valid_on_device());

    // do a clone from device
    s.clone_from_device();
    EXPECT_TRUE(s.is_valid_on_host());
    EXPECT_FALSE(s.is_valid_on_device());    
}

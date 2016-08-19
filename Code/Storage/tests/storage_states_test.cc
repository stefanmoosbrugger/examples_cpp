#include "gtest/gtest.h"

#include "data.hpp"
#include "data_view.hpp"
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

    auto val = [&](){
        std::cout << "on_device: " << s.s->get_state_machine_ptr()->m_od << "\n";
        std::cout << "host_needs_update: " << s.s->get_state_machine_ptr()->m_hnu << "\n";
        std::cout << "device_needs_update: " << s.s->get_state_machine_ptr()->m_dnu << "\n";
    }; 

    auto valid = [](auto& bla) { std::cout << bla.valid() << std::endl; };
    auto hwv = make_host_view<decltype(s), false>(s);
    auto hrv = make_host_view<decltype(s), true>(s);
    valid(hwv);
    valid(hrv);
    
    hwv(0,0,0) = 10;
    hwv(0,0,1) = 20;
    {
        s.sync();
        auto dwv = make_device_view<decltype(s), false>(s);
        auto drv = make_device_view<decltype(s), true>(s);
        valid(dwv);
        valid(drv);
        valid(hwv);
        valid(hrv);
        s.sync();
        valid(dwv);
        valid(drv);
        valid(hwv);
        valid(hrv);
        s.sync();
    s.reactivate_device_write_views();
        valid(dwv);
        valid(drv);
        valid(hwv);
        valid(hrv);
        s.sync();
    s.reactivate_host_write_views();
    }
}

/*
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
*/
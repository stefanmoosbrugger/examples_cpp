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

TEST(StorageState, StatesTest) {
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

    // create a host read and write view
    auto hwv = make_host_view<decltype(s), false>(s);
    auto hrv = make_host_view<decltype(s), true>(s);
    std::cout << "hwv: " << valid(s, hwv) << std::endl;
    std::cout << "hrv: " << valid(s, hrv) << std::endl;
    
    hwv(0,0,0) = 10;
    hwv(0,0,1) = 20;

    std::cout << "sync() (clone to device because we had a hwv)\n";
    s.sync();
    // create a device read and write view
    auto dwv = make_device_view<decltype(s), false>(s);
    auto drv = make_device_view<decltype(s), true>(s);
    std::cout << "dwv: " << valid(s, dwv) << std::endl;
    std::cout << "drv: " << valid(s, drv) << std::endl;
    std::cout << "hwv: " << valid(s, hwv) << std::endl;
    std::cout << "hrv: " << valid(s, hrv) << std::endl;
    std::cout << "sync() (clone from device because we had a dwv)\n";
    s.sync();
    std::cout << "dwv: " << valid(s, dwv) << std::endl;
    std::cout << "drv: " << valid(s, drv) << std::endl;
    std::cout << "hwv: " << valid(s, hwv) << std::endl;
    std::cout << "hrv: " << valid(s, hrv) << std::endl;
    std::cout << "sync() (nothing, no dwv and hwv got activated)\n";
    s.sync();
    std::cout << "reactivate_device_write_views()\n";
    s.reactivate_device_write_views();
    std::cout << "dwv: " << valid(s, dwv) << std::endl;
    std::cout << "drv: " << valid(s, drv) << std::endl;
    std::cout << "hwv: " << valid(s, hwv) << std::endl;
    std::cout << "hrv: " << valid(s, hrv) << std::endl;
    std::cout << "sync() (clone from device because we had a (reactivated) dwv)\n";
    s.sync();
    std::cout << "reactivate_host_write_views()\n";
    s.reactivate_host_write_views();
    std::cout << "dwv: " << valid(s, dwv) << std::endl;
    std::cout << "drv: " << valid(s, drv) << std::endl;
    std::cout << "hwv: " << valid(s, hwv) << std::endl;
    std::cout << "hrv: " << valid(s, hrv) << std::endl;
    std::cout << "sync() (clone from device because we had a (reactivated) hwv)\n";
    s.sync();
}

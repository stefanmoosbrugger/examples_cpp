#include "gtest/gtest.h"

#include <iostream>

#include "data_view.hpp"
#include "data_field_view.hpp"

TEST(ViewTest, Basic) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    typedef data<double, storage_info_t> data_ty;

    storage_info_t si(3,3,3);
    typedef data_field<double, storage_info_t, 2,3,4> data_field_ty;
    data_field_ty df(si);

    auto dfv = make_device_field_view(df);
    dfv.get_value<0,0>(1,2,1) = 5.0;
    EXPECT_EQ((dfv.get<0,0>()(1,2,1)), 5.0);

    auto hfv = make_host_field_view(df);
    hfv.get_value<0,0>(1,2,1) = 4.0;
    EXPECT_EQ((hfv.get<0,0>()(1,2,1)), 4.0);
    
    EXPECT_EQ((dfv.get<0,0>()(1,2,1)), 5.0);
    EXPECT_EQ((hfv.get<0,0>()(1,2,1)), 4.0);
}
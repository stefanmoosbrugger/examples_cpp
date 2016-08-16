#include "gtest/gtest.h"

#include <iostream>

#include "data_view.hpp"
#include "data_field_view.hpp"

TEST(ViewTest, Basic) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    typedef data<double, storage_info_t> data_ty;

    storage_info_t si(3,3,3);
    data_ty d(si);

    auto dv = make_device_view(d);
    dv(1,2,3);
    auto hv = make_host_view(d);
    hv(1,2,3);

    typedef data_field<double, storage_info_t, 2,3,4> data_field_ty;
    data_field_ty df(si);
    auto dfv = make_device_field_view(df);
    dfv.get_value<0,0>(1,2,1) = 5.0;
    EXPECT_EQ((dfv.get<0,0>()(1,2,1)), 5.0);
    auto dhv = make_host_field_view(df);
    dhv.get_value<0,0>(1,2,1) = 4.0;
    EXPECT_EQ((dfv.get<0,0>()(1,2,1)), 5.0);
    EXPECT_EQ((dhv.get<0,0>()(1,2,1)), 4.0);
}
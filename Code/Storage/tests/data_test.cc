#include "gtest/gtest.h"

#include <iostream>

#include "data.hpp"
#include "storage.hpp"
#include "storage_info_sol.hpp"

TEST(DataViewTest, Basic) {
    typedef storage_info_sol< layout_map<2,1,0> > storage_info_t;
    typedef data<double, storage_info_t> data_t;

    // create a storage info type
    storage_info_t si(3,3,3);
    // create two storage views
    data_t my_storage_1(si);
    data_t my_storage_2(si);
    // copy the storage views (internal storages stay the same)
    data_t my_storage_1_cpy = my_storage_1;
    data_t my_storage_2_cpy = my_storage_2;

    //check validity
    EXPECT_EQ(my_storage_1.s.get(), my_storage_1_cpy.s.get());
    EXPECT_EQ(my_storage_1.s.get(), my_storage_1_cpy.s.get());
    EXPECT_EQ(my_storage_2.m.get(), my_storage_2_cpy.m.get());
    EXPECT_EQ(my_storage_2.m.get(), my_storage_2_cpy.m.get());

    EXPECT_NE(my_storage_1.s.get(), my_storage_2.s.get());
    EXPECT_NE(my_storage_1_cpy.s.get(), my_storage_2_cpy.s.get());
    EXPECT_NE(my_storage_1.m.get(), my_storage_2_cpy.m.get());
    EXPECT_NE(my_storage_1_cpy.m.get(), my_storage_2_cpy.m.get());
    

    //fill with data
    int z=0;
    for(int i=0; i<3; ++i)
        for(int j=0; j<3; ++j)
            for(int k=0; k<3; ++k)
                my_storage_1(i,j,k) = ++z;

    for(int i=0; i<3; ++i)
        for(int j=0; j<3; ++j)
            for(int k=0; k<3; ++k) {
                std::cout << my_storage_1(i,j,k) << " == " << my_storage_1_cpy(i,j,k) << " != " << my_storage_2(i,j,k) << std::endl;
                EXPECT_EQ(my_storage_1(i,j,k), my_storage_1_cpy(i,j,k));
                EXPECT_NE(my_storage_2(i,j,k), my_storage_1_cpy(i,j,k));
            }
}

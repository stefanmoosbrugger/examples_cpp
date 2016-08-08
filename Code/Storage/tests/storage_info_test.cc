#include "gtest/gtest.h"

#include "storage_info_sol.hpp"

TEST(StorageInfo, Basic) {
    storage_info_sol< layout_map<2,1,0> > st(2,3,3);
    EXPECT_EQ(st.index(0,0,0), 0);
    EXPECT_EQ(st.index(0,0,1), 1);
    EXPECT_EQ(st.index(0,0,2), 2);

    EXPECT_EQ(st.index(0,1,0), 3);
    EXPECT_EQ(st.index(0,1,1), 4);
    EXPECT_EQ(st.index(0,1,2), 5);
    EXPECT_EQ(st.index(0,2,0), 6);
    EXPECT_EQ(st.index(0,2,1), 7);
    EXPECT_EQ(st.index(0,2,2), 8);

    EXPECT_EQ(st.index(1,0,0), 9);
    EXPECT_EQ(st.index(1,0,1), 10);
    EXPECT_EQ(st.index(1,1,2), 14);
    EXPECT_EQ(st.index(1,2,0), 15);
    EXPECT_EQ(st.index(1,2,1), 16);
    EXPECT_EQ(st.index(1,2,2), 17);
}

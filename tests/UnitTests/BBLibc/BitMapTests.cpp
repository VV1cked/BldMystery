#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#include <BBLibc/BitMap.h>


TEST(BitMapTests, SizeOf)
{
    EXPECT_EQ(sizeof(B_BitMap), 0x18);
}

TEST(BitMapTests, Fields)
{
    EXPECT_OFFSET_EQ(B_BitMap, type, 0x004);
    EXPECT_OFFSET_EQ(B_BitMap, dimension1, 0x008);
    EXPECT_OFFSET_EQ(B_BitMap, dimension2, 0x00C);
    EXPECT_OFFSET_EQ(B_BitMap, data, 0x010);
    EXPECT_OFFSET_EQ(B_BitMap, pal, 0x014);
}

TEST(BitMapTests, BytesPerTexelMapsOriginalTypeCodes)
{
    B_BitMap bitmap;
    const int expected[] = {0, 1, 1, 2, 3, 4, 0, 0};
    for (unsigned int type = 0; type < 8; ++type)
    {
        bitmap.type = static_cast<B_BitMap::BitMapType>(type);
        EXPECT_EQ(expected[type], bitmap.BytesPerTexel());
    }
    bitmap.type = static_cast<B_BitMap::BitMapType>(0xFFFFFFFFu);
    EXPECT_EQ(0, bitmap.BytesPerTexel());
}

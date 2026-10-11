#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#include <BBLibc/BitMap24.h>
#include <BBLibc/Color.h>
#include <cstring>
#include <new>


TEST(BitMap24Tests, Fields)
{
    EXPECT_OFFSET_EQ(B_BitMap24, dimension1, 0x004);
    EXPECT_OFFSET_EQ(B_BitMap24, dimension2, 0x008);
    EXPECT_OFFSET_EQ(B_BitMap24, data, 0x00C);
}

TEST(BitMap24Tests, DimensionsPreserveBitsAndOutputWriteOrder)
{
    B_BitMap24 bitmap;
    bitmap.dimension1 = 0xFFFFFFFFu;
    bitmap.dimension2 = 0x80000000u;
    int width = 0, height = 0;
    bitmap.GetDimension(width, height);
    EXPECT_EQ(-1, width);
    EXPECT_EQ(static_cast<int>(0x80000000u), height);
    bitmap.GetDimension(width, width);
    EXPECT_EQ(height, width);
}

TEST(BitMap24Tests, BitsReturnsStoredPointerIncludingNull)
{
    B_BitMap24 bitmap;
    unsigned char storage[3] = {};
    bitmap.data = storage;
    EXPECT_EQ(storage, bitmap.GetBits());
    bitmap.data = nullptr;
    EXPECT_EQ(nullptr, bitmap.GetBits());
}

TEST(BitMap24Tests, RgbOrderUsesPackedPixelsAndSignedWrappingCount)
{
    B_BitMap24 bitmap;
    unsigned char storage[] = {1, 2, 3, 4, 5, 6, 7, 8};
    const unsigned char expected[] = {3, 2, 1, 6, 5, 4, 7, 8};
    bitmap.dimension1 = 2; bitmap.dimension2 = 1; bitmap.data = storage;
    bitmap.ChangeRGBOrder();
    EXPECT_EQ(0, std::memcmp(storage, expected, sizeof(storage)));
    bitmap.ChangeRGBOrder();
    EXPECT_EQ(1, storage[0]); EXPECT_EQ(6, storage[5]);
    // Safe overflow: the product wraps to two pixels.
    bitmap.dimension1 = 0x80000001u; bitmap.dimension2 = 2;
    bitmap.ChangeRGBOrder();
    EXPECT_EQ(0, std::memcmp(storage, expected, sizeof(storage)));
    bitmap.data = nullptr;
    bitmap.dimension1 = 0; bitmap.ChangeRGBOrder();
    bitmap.dimension1 = 0x40000000u; bitmap.ChangeRGBOrder(); // Signed negative product.
}

TEST(BitMap24Tests, PixelAccessUsesRgbAndPreservesOutputOnFailure)
{
    B_BitMap24 bitmap;
    unsigned char storage[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    bitmap.dimension1 = bitmap.dimension2 = 2; bitmap.data = storage;
    B_Color color(99, 98, 97);
    EXPECT_EQ(1u, bitmap.GetPixelAt(1, 1, color));
    EXPECT_EQ(10, color.r); EXPECT_EQ(11, color.g); EXPECT_EQ(12, color.b);
    unsigned char before[sizeof(color)];
    std::memcpy(before, &color, sizeof(color));
    const int invalid[][2] = {{-1, 0}, {0, -1}, {2, 0}, {0, 2}};
    for (const auto &point : invalid)
    {
        EXPECT_EQ(0u, bitmap.GetPixelAt(point[0], point[1], color));
        EXPECT_EQ(0, std::memcmp(before, &color, sizeof(color)));
    }
    bitmap.dimension1 = 0x80000000u; bitmap.data = nullptr;
    EXPECT_EQ(0u, bitmap.GetPixelAt(0, 0, color));
    EXPECT_EQ(0, std::memcmp(before, &color, sizeof(color)));
    // Safe offset wrap: 3 * (2 * 0x2AAAAAAB) wraps to 2.
    bitmap.dimension1 = 0x2AAAAAABu; bitmap.dimension2 = 3; bitmap.data = storage;
    EXPECT_EQ(1u, bitmap.GetPixelAt(0, 2, color));
    EXPECT_EQ(3, color.r); EXPECT_EQ(4, color.g); EXPECT_EQ(5, color.b);
    bitmap.data = nullptr; // The fixture lends stack storage; it does not transfer ownership.
}

TEST(BitMap24Tests, ClearReleasesDataAndZerosDimensions)
{
    B_BitMap24 bitmap;
    bitmap.dimension1 = 2; bitmap.dimension2 = 3;
    bitmap.data = ::operator new(18);
    bitmap.Clear();
    EXPECT_EQ(nullptr, bitmap.data);
    EXPECT_EQ(0u, bitmap.dimension1); EXPECT_EQ(0u, bitmap.dimension2);
    bitmap.Clear();
    EXPECT_EQ(nullptr, bitmap.data);
}

TEST(BitMap24Tests, ClearZerosDimensionsEvenWhenDataIsNull)
{
    B_BitMap24 bitmap;
    bitmap.dimension1 = 0xFFFFFFFFu; bitmap.dimension2 = 0x80000000u; bitmap.data = nullptr;
    bitmap.Clear();
    EXPECT_EQ(nullptr, bitmap.data);
    EXPECT_EQ(0u, bitmap.dimension1); EXPECT_EQ(0u, bitmap.dimension2);
}

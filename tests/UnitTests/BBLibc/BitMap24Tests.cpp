#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#define private public
#include <BBLibc/BitMap24.h>
#undef private
#include <BBLibc/BitMap.h>
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

TEST(BitMap24Tests, AssignmentReusesBufferAndSupportsSelfAssignment)
{
    B_BitMap24 source, target;
    byte pixels[] = {1,2,3,4,5,6};
    source.dimension1 = target.dimension1 = 2; source.dimension2 = target.dimension2 = 1;
    source.data = pixels; target.data = ::operator new(6);
    void *oldData = target.data;
    EXPECT_EQ(&target, &(target = source)); EXPECT_EQ(oldData, target.data);
    EXPECT_EQ(0, std::memcmp(target.data, pixels, 6));
    EXPECT_EQ(&target, &(target = target)); EXPECT_EQ(oldData, target.data);
    EXPECT_EQ(0, std::memcmp(target.data, pixels, 6));
    source.data = nullptr; target.Clear();
}

TEST(BitMap24Tests, AssignmentResizesAndNormalizesZeroOnlyWhenDimensionsDiffer)
{
    B_BitMap24 source, target;
    byte pixels[] = {1,2,3,4,5,6};
    source.dimension1 = 2; source.dimension2 = 1; source.data = pixels;
    target.dimension1 = target.dimension2 = 1; target.data = ::operator new(3);
    target = source; EXPECT_EQ(2u, target.dimension1); EXPECT_EQ(1u, target.dimension2);
    EXPECT_EQ(0, std::memcmp(target.data, pixels, 6));
    source.dimension1 = 0; source.dimension2 = 7; source.data = nullptr;
    target = source; EXPECT_EQ(0u, target.dimension1); EXPECT_EQ(0u, target.dimension2); EXPECT_EQ(nullptr, target.data);
    target.dimension2 = 7; target = source;
    EXPECT_EQ(7u, target.dimension2); EXPECT_EQ(nullptr, target.data);
}

TEST(BitMap24Tests, SetBitsChecksExactFormatAndReusesBuffer)
{
    B_BitMap24 target;
    byte pixels[] = {1,2,3,4,5,6};
    target.dimension1 = 2; target.dimension2 = 1; target.data = ::operator new(6);
    std::memset(target.data, 0xA5, 6); void *oldData = target.data;
    const char badFormats[][4] = {{'B','G','R',0}, {'R','G','B','X'}, {'r','g','b',0}};
    for (const auto &format : badFormats)
    {
        EXPECT_EQ(0u, target.SetBits(99, 88, nullptr, format));
        EXPECT_EQ(oldData, target.data); EXPECT_EQ(2u, target.dimension1); EXPECT_EQ(1u, target.dimension2);
        EXPECT_EQ(0xA5, static_cast<byte *>(target.data)[0]);
    }
    EXPECT_EQ(1u, target.SetBits(2, 1, reinterpret_cast<const char *>(pixels), "RGB"));
    EXPECT_EQ(oldData, target.data); EXPECT_EQ(0, std::memcmp(target.data, pixels, 6));
    target.Clear();
}

TEST(BitMap24Tests, SetBitsResizesRetainsZeroDimensionsAndUsesWrappedSize)
{
    B_BitMap24 target; target.dimension1 = target.dimension2 = 0; target.data = nullptr;
    const char pixels[] = {1,2,3,4,5,6};
    EXPECT_EQ(1u, target.SetBits(static_cast<int>(0x80000001u), 2, pixels, "RGB"));
    EXPECT_EQ(0x80000001u, target.dimension1); EXPECT_EQ(2u, target.dimension2);
    EXPECT_EQ(0, std::memcmp(target.data, pixels, 6));
    EXPECT_EQ(1u, target.SetBits(0, 7, nullptr, "RGB"));
    EXPECT_EQ(0u, target.dimension1); EXPECT_EQ(7u, target.dimension2);
    ASSERT_NE(nullptr, target.data);
    void *zeroAllocation = target.data;
    EXPECT_EQ(1u, target.SetBits(0, 7, nullptr, "RGB")); EXPECT_EQ(zeroAllocation, target.data);
    target.Clear();
}

TEST(BitMap24Tests, SetBitsPreservesOriginalForwardDwordOverlap)
{
    B_BitMap24 target; byte pixels[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    target.dimension1 = 4; target.dimension2 = 1; target.data = pixels + 3;
    const byte expected[] = {1,2,3,1,2,3,4,2,3,4,8,3,4,8,12};
    EXPECT_EQ(1u, target.SetBits(4, 1, reinterpret_cast<const char *>(pixels), "RGB"));
    EXPECT_EQ(0, std::memcmp(pixels, expected, sizeof(pixels)));
    target.data = nullptr;
}

TEST(BitMap24Tests, RegistryLayoutMatchesPointerArray)
{
    EXPECT_EQ(24u, sizeof(B_Array<B_BitMap24>));
    EXPECT_OFFSET_EQ(B_Array<B_BitMap24>, data_, 4);
    EXPECT_OFFSET_EQ(B_Array<B_BitMap24>, size_, 8);
    EXPECT_OFFSET_EQ(B_Array<B_BitMap24>, increment_, 12);
    EXPECT_OFFSET_EQ(B_Array<B_BitMap24>, capacity_, 16);
    EXPECT_OFFSET_EQ(B_Array<B_BitMap24>, deleteItems_, 20);
}

TEST(BitMap24Tests, ConstructionRetainsZeroDimensionsAndWrapsAllocationSize)
{
    B_BitMap24 zero(0, 7), wrapped(static_cast<int>(0x80000001u), 2);
    EXPECT_EQ(0u, zero.dimension1); EXPECT_EQ(7u, zero.dimension2); EXPECT_EQ(nullptr, zero.data);
    EXPECT_EQ(0x80000001u, wrapped.dimension1); EXPECT_EQ(2u, wrapped.dimension2);
    ASSERT_NE(nullptr, wrapped.data); std::memset(wrapped.data, 0xA5, 6);
    B_BitMap24 copy(wrapped); ASSERT_NE(nullptr, copy.data); EXPECT_NE(wrapped.data, copy.data);
    EXPECT_EQ(0, std::memcmp(wrapped.data, copy.data, 6));
    B_BitMap24 zeroCopy(zero); EXPECT_EQ(7u, zeroCopy.dimension2); EXPECT_EQ(nullptr, zeroCopy.data);
}

TEST(BitMap24Tests, ConversionSupportsIndexedGrayAndRgbOnly)
{
    B_Pal palette; std::memset(&palette, 0, sizeof(palette));
    palette.SetRed(1, 200); palette.SetGreen(1, 130); palette.SetBlue(1, 127);
    for (unsigned int type = 1; type <= 5; ++type)
    {
        B_BitMap source(static_cast<B_BitMap::BitMapType>(type), 1, 1, &palette);
        byte *pixels = static_cast<byte *>(source.data); pixels[0] = 1;
        if (type == 4) { pixels[1] = 2; pixels[2] = 3; }
        B_BitMap24 target(source);
        if (type == 3 || type == 5)
        {
            EXPECT_EQ(0u, target.dimension1); EXPECT_EQ(0u, target.dimension2); EXPECT_EQ(nullptr, target.data);
        }
        else
        {
            ASSERT_NE(nullptr, target.data);
            const byte indexed[] = {144, 4, 254}, gray[] = {1,1,1}, rgb[] = {1,2,3};
            EXPECT_EQ(0, std::memcmp(target.data, type == 1 ? indexed : type == 2 ? gray : rgb, 3));
        }
    }
}

TEST(BitMap24Tests, RegistryGrowsShrinksAndPreservesOrder)
{
    B_Array<B_BitMap24> registry;
    ASSERT_EQ(0u, registry.size());
    B_BitMap24 *objects[18];
    for (unsigned int i = 0; i < 18; ++i) { objects[i] = new B_BitMap24; registry.push_back(objects[i]); }
    EXPECT_EQ(18u, registry.size()); EXPECT_EQ(32u, registry.capacity_); EXPECT_EQ(16u, registry.increment_);
    registry.remove(objects[8]); delete objects[8];
    EXPECT_EQ(17u, registry.size()); EXPECT_EQ(32u, registry.capacity_); // No shrink at size 18.
    registry.remove(objects[9]); delete objects[9];
    EXPECT_EQ(16u, registry.size()); EXPECT_EQ(16u, registry.capacity_);
    EXPECT_EQ(objects[10], registry[8]);
    for (unsigned int i = 0; i < 18; ++i) if (i != 8 && i != 9) { registry.remove(objects[i]); delete objects[i]; }
    EXPECT_EQ(0u, registry.size()); EXPECT_EQ(0u, registry.capacity_); EXPECT_EQ(nullptr, registry.data_);
}

TEST(BitMap24Tests, GlobalClearKeepsDimensionsAndForgetsLiveObjects)
{
    B_BitMap24 old(2, 3); std::memset(old.data, 0xA5, 18);
    B_BitMap24 copy(old);
    B_BitMap24::ClearBitMap24Data();
    EXPECT_EQ(2u, old.dimension1); EXPECT_EQ(3u, old.dimension2); EXPECT_EQ(nullptr, old.data);
    EXPECT_EQ(nullptr, copy.data);
    const char pixels[18] = {};
    EXPECT_EQ(1u, old.SetBits(3, 2, pixels, "RGB"));
    B_BitMap24 fresh(1, 1);
    B_BitMap24::ClearBitMap24Data();
    EXPECT_EQ(nullptr, fresh.data); ASSERT_NE(nullptr, old.data); // The old object is no longer registered.
    old.Clear();
}

#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#include <BBLibc/BitMap.h>
#include <cstring>
#include <new>


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

TEST(BitMapTests, DefaultConstructorClearsAllFields)
{
    B_BitMap bitmap;
    EXPECT_EQ(0, static_cast<int>(bitmap.type));
    EXPECT_EQ(0u, bitmap.dimension1); EXPECT_EQ(0u, bitmap.dimension2);
    EXPECT_EQ(nullptr, bitmap.data); EXPECT_EQ(nullptr, bitmap.pal);
}

TEST(BitMapTests, ConstructorAllocatesPixelsAndCopiesPaletteForEveryType)
{
    B_Pal palette;
    for (unsigned int i = 0; i < 256; ++i)
    {
        palette.SetRed(i, static_cast<byte>(i));
        palette.SetGreen(i, static_cast<byte>(i + 1));
        palette.SetBlue(i, static_cast<byte>(i + 2));
    }
    for (unsigned int type = 0; type <= 6; ++type)
    {
        B_BitMap bitmap(static_cast<B_BitMap::BitMapType>(type), 3, 5, &palette);
        EXPECT_EQ(type, static_cast<unsigned int>(bitmap.type));
        EXPECT_EQ(3u, bitmap.dimension1); EXPECT_EQ(5u, bitmap.dimension2);
        ASSERT_NE(nullptr, bitmap.data); ASSERT_NE(nullptr, bitmap.pal);
        EXPECT_NE(&palette, bitmap.pal);
        EXPECT_EQ(0, std::memcmp(&palette, bitmap.pal, sizeof(palette)));
        if (bitmap.BytesPerTexel())
            std::memset(bitmap.data, 0xA5, 15 * bitmap.BytesPerTexel());
        bitmap.pal->SetRed(7, 99);
        EXPECT_EQ(7, palette.GetRed(7));
    }
}

TEST(BitMapTests, ConstructorPreservesDimensionsWhenAllocationSizeWraps)
{
    B_BitMap bitmap(B_BitMap::BitMapType4, 0x80000001u, 2, nullptr);
    EXPECT_EQ(0x80000001u, bitmap.dimension1); EXPECT_EQ(2u, bitmap.dimension2);
    EXPECT_EQ(nullptr, bitmap.pal);
    ASSERT_NE(nullptr, bitmap.data);
    // 0x80000001 * 2 * 3 wraps to six bytes.
    std::memset(bitmap.data, 0xA5, 6);
}

TEST(BitMapTests, DestructorLeavesReleasedPointerFieldsUntouched)
{
    alignas(B_BitMap) unsigned char storage[sizeof(B_BitMap)];
    B_Pal palette;
    std::memset(&palette, 0xA5, sizeof(palette));
    B_BitMap *bitmap = new (storage) B_BitMap(B_BitMap::BitMapType2, 2, 3, &palette);
    unsigned char fields[sizeof(B_BitMap) - 4];
    std::memcpy(fields, storage + 4, sizeof(fields));
    bitmap->~B_BitMap();
    EXPECT_EQ(0, std::memcmp(fields, storage + 4, sizeof(fields)));
}

TEST(BitMapTests, CopyConstructorUsesIndependentPixelsAndTypeSpecificPalette)
{
    B_Pal palette;
    std::memset(&palette, 0xA5, sizeof(palette));
    for (unsigned int type = 1; type <= 5; ++type)
    {
        B_BitMap source(static_cast<B_BitMap::BitMapType>(type), 3, 5, &palette);
        const unsigned int size = 15 * source.BytesPerTexel();
        std::memset(source.data, 0x5A, size);
        B_BitMap copy(source);
        EXPECT_EQ(source.type, copy.type);
        EXPECT_EQ(3u, copy.dimension1); EXPECT_EQ(5u, copy.dimension2);
        EXPECT_NE(source.data, copy.data);
        EXPECT_EQ(0, std::memcmp(source.data, copy.data, size));
        static_cast<byte *>(copy.data)[0] = 1;
        EXPECT_EQ(0x5A, static_cast<byte *>(source.data)[0]);
        if (type == 1 || type == 3)
        {
            ASSERT_NE(nullptr, copy.pal); EXPECT_NE(source.pal, copy.pal);
            EXPECT_EQ(0, std::memcmp(source.pal, copy.pal, sizeof(palette)));
        }
        else EXPECT_EQ(nullptr, copy.pal);
    }
}

TEST(BitMapTests, UnknownTypeCopyLeavesDataStorageUntouched)
{
    B_BitMap source;
    source.type = static_cast<B_BitMap::BitMapType>(99);
    source.dimension1 = 2; source.dimension2 = 3;
    alignas(B_BitMap) unsigned char storage[sizeof(B_BitMap)];
    std::memset(storage, 0xA5, sizeof(storage));
    B_BitMap *copy = new (storage) B_BitMap(source);
    EXPECT_EQ(99, static_cast<int>(copy->type));
    EXPECT_EQ(2u, copy->dimension1); EXPECT_EQ(3u, copy->dimension2);
    for (unsigned int i = 0x10; i < 0x14; ++i) EXPECT_EQ(0xA5, storage[i]);
    EXPECT_EQ(nullptr, copy->pal);
    copy->data = nullptr; // Avoid freeing the intentionally untouched sentinel pointer.
    copy->~B_BitMap();
}

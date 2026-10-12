#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#include <BBLibc/BitMap.h>
#include <BBLibc/BitMap24.h>
#include <cstring>
#include <new>
#include <fcntl.h>
#include <vector>
#define private public
#include <BBLibc/ODataFile.h>
#include <BBLibc/IDataFile.h>
#undef private


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

TEST(BitMapTests, ConversionSelectsFirstRgbByteOrCopiesAllChannels)
{
    B_BitMap24 source;
    byte pixels[] = {1, 90, 80, 2, 70, 60, 3, 50, 40, 4, 30, 20};
    source.dimension1 = source.dimension2 = 2; source.data = pixels;
    B_BitMap single(B_BitMap::BitMapType2, source);
    B_BitMap rgb(B_BitMap::BitMapType4, source);
    const byte expected[] = {1, 2, 3, 4};
    EXPECT_EQ(B_BitMap::BitMapType2, single.type);
    EXPECT_EQ(B_BitMap::BitMapType4, rgb.type);
    EXPECT_EQ(2u, single.dimension1); EXPECT_EQ(2u, single.dimension2);
    EXPECT_EQ(2u, rgb.dimension1); EXPECT_EQ(2u, rgb.dimension2);
    ASSERT_NE(nullptr, single.data); ASSERT_NE(nullptr, rgb.data);
    EXPECT_EQ(0, std::memcmp(expected, single.data, sizeof(expected)));
    EXPECT_EQ(0, std::memcmp(pixels, rgb.data, sizeof(pixels)));
    EXPECT_NE(source.data, rgb.data);
    static_cast<byte *>(rgb.data)[0] = 99;
    EXPECT_EQ(1, pixels[0]);
    EXPECT_EQ(nullptr, single.pal); EXPECT_EQ(nullptr, rgb.pal);
    source.data = nullptr;
}

TEST(BitMapTests, UnsupportedConversionReturnsEmptyBitmapWithoutReadingPixels)
{
    B_BitMap24 source;
    source.dimension1 = source.dimension2 = 0xFFFFFFFFu; source.data = nullptr;
    const unsigned int types[] = {0, 1, 3, 5, 6, 0xFFFFFFFFu};
    for (unsigned int type : types)
    {
        B_BitMap bitmap(static_cast<B_BitMap::BitMapType>(type), source);
        EXPECT_EQ(0, static_cast<int>(bitmap.type));
        EXPECT_EQ(0u, bitmap.dimension1); EXPECT_EQ(0u, bitmap.dimension2);
        EXPECT_EQ(nullptr, bitmap.data); EXPECT_EQ(nullptr, bitmap.pal);
    }
}

TEST(BitMapTests, ConversionPreservesDimensionsAndUsesWrappedCount)
{
    B_BitMap24 source;
    byte pixels[] = {1, 2, 3, 4, 5, 6};
    source.dimension1 = 0x80000001u; source.dimension2 = 2; source.data = pixels;
    B_BitMap single(B_BitMap::BitMapType2, source), rgb(B_BitMap::BitMapType4, source);
    EXPECT_EQ(source.dimension1, single.dimension1); EXPECT_EQ(2u, single.dimension2);
    EXPECT_EQ(source.dimension1, rgb.dimension1); EXPECT_EQ(2u, rgb.dimension2);
    ASSERT_NE(nullptr, single.data); ASSERT_NE(nullptr, rgb.data);
    EXPECT_EQ(1, static_cast<byte *>(single.data)[0]);
    EXPECT_EQ(4, static_cast<byte *>(single.data)[1]);
    EXPECT_EQ(0, std::memcmp(pixels, rgb.data, sizeof(pixels)));
    source.data = nullptr;
}

TEST(BitMapTests, SerializationWritesHeaderPixelsAndTypeSpecificPalette)
{
    B_Pal palette;
    std::memset(&palette, 0xA5, sizeof(palette));
    for (unsigned int type = 1; type <= 5; ++type)
    {
        B_BitMap bitmap(static_cast<B_BitMap::BitMapType>(type), 2, 3, &palette);
        const unsigned int size = 6 * bitmap.BytesPerTexel();
        std::memset(bitmap.data, 0x5A, size);
        B_ODataFile output("NUL", _O_BINARY);
        ASSERT_NE(0u, output.OK());
        EXPECT_EQ(&output, &(output << bitmap));
        const unsigned int header[] = {type, 2, 3};
        const unsigned int paletteSize = type == 1 || type == 3 ? 768 : 0;
        ASSERT_EQ(12u + size + paletteSize, output.posInCacheBlock);
        EXPECT_EQ(0, std::memcmp(header, output.fileCache, sizeof(header)));
        EXPECT_EQ(0, std::memcmp(bitmap.data, output.fileCache + 12, size));
        if (paletteSize) EXPECT_EQ(0, std::memcmp(&palette, output.fileCache + 12 + size, paletteSize));
    }
}

TEST(BitMapTests, SerializationPreservesUnknownTypeWithoutPayload)
{
    B_BitMap bitmap;
    bitmap.type = static_cast<B_BitMap::BitMapType>(0xFFFFFFFFu);
    bitmap.dimension1 = 99; bitmap.dimension2 = 88;
    B_ODataFile output("NUL", _O_BINARY);
    ASSERT_NE(0u, output.OK());
    output << bitmap;
    const unsigned int header[] = {0xFFFFFFFFu, 99, 88};
    ASSERT_EQ(12u, output.posInCacheBlock);
    EXPECT_EQ(0, std::memcmp(header, output.fileCache, sizeof(header)));
}

TEST(BitMapTests, SerializationUsesWrappedByteCount)
{
    B_BitMap bitmap(B_BitMap::BitMapType4, 0x80000001u, 2, nullptr);
    std::memset(bitmap.data, 0x5A, 6);
    B_ODataFile output("NUL", _O_BINARY);
    ASSERT_NE(0u, output.OK());
    output << bitmap;
    ASSERT_EQ(18u, output.posInCacheBlock);
    EXPECT_EQ(0, std::memcmp(bitmap.data, output.fileCache + 12, 6));
}

static void InitializeBitmapInputCache(B_IDataFile &input, const std::vector<byte> &bytes)
{
    input.file_size = static_cast<unsigned int>(bytes.size());
    input.cacheBlockStartPos = input.posInCacheBlock = 0;
    std::memcpy(input.fileCache, bytes.data(), bytes.size());
}

TEST(BitMapTests, DeserializationReadsAllFormatsAndReplacesOldStorage)
{
    B_Pal palette; std::memset(&palette, 0xA5, sizeof(palette));
    const unsigned int bytesPerPixel[] = {0, 1, 1, 2, 3, 4};
    for (unsigned int type = 1; type <= 5; ++type)
    {
        const unsigned int size = 6 * bytesPerPixel[type];
        const unsigned int palSize = type == 1 || type == 3 ? 768 : 0;
        std::vector<byte> bytes(12 + size + palSize);
        const unsigned int header[] = {type, 2, 3}; std::memcpy(bytes.data(), header, 12);
        for (unsigned int i = 12; i < bytes.size(); ++i) bytes[i] = static_cast<byte>(i);
        B_IDataFile input; InitializeBitmapInputCache(input, bytes);
        B_BitMap bitmap(B_BitMap::BitMapType4, 1, 1, &palette);
        EXPECT_EQ(&input, &(input >> bitmap));
        EXPECT_EQ(type, static_cast<unsigned int>(bitmap.type));
        EXPECT_EQ(2u, bitmap.dimension1); EXPECT_EQ(3u, bitmap.dimension2);
        ASSERT_NE(nullptr, bitmap.data);
        EXPECT_EQ(0, std::memcmp(bytes.data() + 12, bitmap.data, size));
        if (palSize)
        {
            ASSERT_NE(nullptr, bitmap.pal);
            EXPECT_EQ(0, std::memcmp(bytes.data() + 12 + size, bitmap.pal, palSize));
        }
        else EXPECT_EQ(nullptr, bitmap.pal);
        EXPECT_EQ(static_cast<long>(bytes.size()), input.Tell());
    }
}

TEST(BitMapTests, UnknownInputTypeClearsStorageButKeepsDimensions)
{
    B_Pal palette; std::memset(&palette, 0xA5, sizeof(palette));
    B_BitMap bitmap(B_BitMap::BitMapType4, 1, 1, &palette);
    const unsigned int header[] = {0xFFFFFFFFu, 99, 88};
    std::vector<byte> bytes(13, 0x5A); std::memcpy(bytes.data(), header, 12);
    B_IDataFile input; InitializeBitmapInputCache(input, bytes);
    input >> bitmap;
    EXPECT_EQ(0, static_cast<int>(bitmap.type));
    EXPECT_EQ(99u, bitmap.dimension1); EXPECT_EQ(88u, bitmap.dimension2);
    EXPECT_EQ(nullptr, bitmap.data); EXPECT_EQ(nullptr, bitmap.pal);
    EXPECT_EQ(12, input.Tell());
}

TEST(BitMapTests, ShortPayloadReadsAvailableBytesWithoutChangingDeclaredDimensions)
{
    const unsigned int header[] = {4, 2, 1};
    std::vector<byte> bytes(15, 0x5A); std::memcpy(bytes.data(), header, 12);
    B_IDataFile input; InitializeBitmapInputCache(input, bytes);
    B_BitMap bitmap; input >> bitmap;
    EXPECT_EQ(B_BitMap::BitMapType4, bitmap.type);
    EXPECT_EQ(2u, bitmap.dimension1); EXPECT_EQ(1u, bitmap.dimension2);
    ASSERT_NE(nullptr, bitmap.data);
    EXPECT_EQ(0, std::memcmp(bytes.data() + 12, bitmap.data, 3));
    EXPECT_EQ(15, input.Tell());
    // The remaining three allocation bytes are intentionally not read by this test.
}

TEST(BitMapTests, ShortDimensionsRetainUnreadBytesForUnknownType)
{
    const unsigned int type = 99;
    std::vector<byte> bytes(6); std::memcpy(bytes.data(), &type, 4);
    bytes[4] = 1; bytes[5] = 2;
    B_IDataFile input; InitializeBitmapInputCache(input, bytes);
    B_BitMap bitmap;
    bitmap.dimension1 = 0x11220000u; bitmap.dimension2 = 88;
    input >> bitmap;
    EXPECT_EQ(0, static_cast<int>(bitmap.type));
    EXPECT_EQ(0x11220201u, bitmap.dimension1); EXPECT_EQ(88u, bitmap.dimension2);
    EXPECT_EQ(nullptr, bitmap.data); EXPECT_EQ(nullptr, bitmap.pal);
    EXPECT_EQ(6, input.Tell());
}

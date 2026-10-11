#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#include <cstring>
#include <fcntl.h>
#define private public
#include <BBLibc/Color.h>
#include <BBLibc/IDataFile.h>
#include <BBLibc/ODataFile.h>
#undef private


TEST(ColorTests, SizeOf)
{
    EXPECT_EQ(sizeof(B_Color), 0x0008);
}

TEST(ColorTests, Fields)
{
    EXPECT_OFFSET_EQ(B_Color, r, 0x0004);
    EXPECT_OFFSET_EQ(B_Color, g, 0x0005);
    EXPECT_OFFSET_EQ(B_Color, b, 0x0006);
}

static void SetColorInputCache(B_IDataFile &file, const void *data, unsigned int size)
{
    file.file_size = size;
    file.cacheBlockStartPos = 0;
    file.posInCacheBlock = 0;
    std::memcpy(file.fileCache, data, size);
}

TEST(ColorTests, SerializationWritesExactlyThreeRgbBytes)
{
    const B_Color color(0, 0x80, 0xFF);
    const unsigned char expected[] = {0, 0x80, 0xFF};
    B_ODataFile output("NUL", _O_BINARY);
    ASSERT_NE(0u, output.OK());
    EXPECT_EQ(&output, &(output << color));
    ASSERT_EQ(sizeof(expected), output.posInCacheBlock);
    EXPECT_EQ(0, std::memcmp(expected, output.fileCache, sizeof(expected)));
}

TEST(ColorTests, DeserializationPreservesDerivedStateAndDoesNotCallMakeCons)
{
    struct TaggedColor : B_Color
    {
        TaggedColor() : calls(0), tag(0x12345678u) {}
        void MakeCons() override { ++calls; }
        unsigned int calls;
        unsigned int tag;
    } color;
    unsigned char vtable[sizeof(void *)];
    std::memcpy(vtable, &color, sizeof(vtable));
    unsigned char *bytes = reinterpret_cast<unsigned char *>(&color);
    bytes[7] = 0xA5;
    const unsigned char encoded[] = {0xFF, 0, 0x80};
    B_IDataFile input;
    SetColorInputCache(input, encoded, sizeof(encoded));
    EXPECT_EQ(&input, &(input >> color));
    EXPECT_EQ(0xFF, color.r);
    EXPECT_EQ(0, color.g);
    EXPECT_EQ(0x80, color.b);
    EXPECT_EQ(0u, color.calls);
    EXPECT_EQ(0x12345678u, color.tag);
    EXPECT_EQ(0xA5, bytes[7]);
    EXPECT_EQ(0, std::memcmp(vtable, &color, sizeof(vtable)));
    EXPECT_EQ(3u, input.posInCacheBlock);
}

TEST(ColorTests, ShortReadLeavesUnavailableChannelsUnchanged)
{
    B_Color color(10, 20, 30);
    const unsigned char encoded[] = {0xFF};
    B_IDataFile input;
    SetColorInputCache(input, encoded, sizeof(encoded));
    EXPECT_EQ(&input, &(input >> color));
    EXPECT_EQ(0xFF, color.r);
    EXPECT_EQ(20, color.g);
    EXPECT_EQ(30, color.b);
}

TEST(ColorTests, ChainedColorsAndByteShareStreamPosition)
{
    const B_Color first(1, 2, 3), second(4, 5, 6);
    const unsigned char marker = 0xA5;
    const unsigned char expected[] = {1, 2, 3, 0xA5, 4, 5, 6};
    B_ODataFile output("NUL", _O_BINARY);
    ASSERT_NE(0u, output.OK());
    EXPECT_EQ(&output, &(output << first << marker << second));
    ASSERT_EQ(sizeof(expected), output.posInCacheBlock);
    EXPECT_EQ(0, std::memcmp(expected, output.fileCache, sizeof(expected)));
    B_IDataFile input;
    SetColorInputCache(input, expected, sizeof(expected));
    B_Color readFirst, readSecond;
    unsigned char readMarker = 0;
    EXPECT_EQ(&input, &(input >> readFirst >> readMarker >> readSecond));
    EXPECT_EQ(1, readFirst.r);
    EXPECT_EQ(2, readFirst.g);
    EXPECT_EQ(3, readFirst.b);
    EXPECT_EQ(marker, readMarker);
    EXPECT_EQ(4, readSecond.r);
    EXPECT_EQ(5, readSecond.g);
    EXPECT_EQ(6, readSecond.b);
    EXPECT_EQ(sizeof(expected), input.posInCacheBlock);
}

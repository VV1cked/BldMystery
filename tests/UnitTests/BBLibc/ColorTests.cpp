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

TEST(ColorTests, ComparisonsReturnBooleanForEachRgbChannel)
{
    B_Color color(0, 0x80, 0xFF);
    const B_Color same(0, 0x80, 0xFF);
    const B_Color different[] = {
        B_Color(1, 0x80, 0xFF), B_Color(0, 0x7F, 0xFF), B_Color(0, 0x80, 0xFE)
    };
    EXPECT_EQ(1u, color == same);
    EXPECT_EQ(0u, color != same);
    EXPECT_EQ(1u, color == color);
    EXPECT_EQ(0u, color != color);
    for (const B_Color &other : different)
    {
        EXPECT_EQ(0u, color == other);
        EXPECT_EQ(1u, color != other);
    }
}

TEST(ColorTests, ComparisonsIgnorePaddingAndDerivedType)
{
    struct DerivedColor : B_Color {
        void MakeCons() override {}
    } derived;
    B_Color base;
    base.SetColor(255, 128, 0);
    derived.SetColor(255, 128, 0);
    reinterpret_cast<unsigned char *>(&base)[7] = 0xA5;
    reinterpret_cast<unsigned char *>(&derived)[7] = 0x5A;
    EXPECT_EQ(1u, base == derived);
    EXPECT_EQ(1u, derived == base);
    EXPECT_EQ(0u, base != derived);
    EXPECT_EQ(0u, derived != base);
}

TEST(ColorTests, AdditionWrapsChannelsAndPreservesOperands)
{
    const B_Color left(255, 128, 1), right(1, 128, 254);
    const B_Color sum = left + right;
    EXPECT_EQ(0, sum.r);
    EXPECT_EQ(0, sum.g);
    EXPECT_EQ(255, sum.b);
    EXPECT_EQ(255, left.r);
    EXPECT_EQ(128, left.g);
    EXPECT_EQ(1, left.b);
    EXPECT_EQ(1, right.r);
    EXPECT_EQ(128, right.g);
    EXPECT_EQ(254, right.b);
    const B_Color doubled = left + left;
    EXPECT_EQ(254, doubled.r);
    EXPECT_EQ(0, doubled.g);
    EXPECT_EQ(2, doubled.b);
}

TEST(ColorTests, SetColorAndAdditionDoNotInvokeOperandMakeCons)
{
    struct DerivedColor : B_Color
    {
        DerivedColor() : calls(0) {}
        void MakeCons() override { ++calls; }
        unsigned int calls;
    } left, right;
    unsigned char before[sizeof(void *)];
    std::memcpy(before, &left, sizeof(before));
    reinterpret_cast<unsigned char *>(&left)[7] = 0xA5;
    left.SetColor(255, 128, 0);
    right.SetColor(1, 0, 255);
    const B_Color sum = left + right;
    EXPECT_EQ(0, sum.r);
    EXPECT_EQ(128, sum.g);
    EXPECT_EQ(255, sum.b);
    EXPECT_EQ(0u, left.calls);
    EXPECT_EQ(0u, right.calls);
    EXPECT_EQ(0xA5, reinterpret_cast<unsigned char *>(&left)[7]);
    EXPECT_EQ(0, std::memcmp(before, &left, sizeof(before)));
}

TEST(ColorTests, MakeConsLeavesEveryStoredByteValueUnchanged)
{
    for (unsigned int value = 0; value < 256; ++value)
    {
        B_Color color(static_cast<byte>(value), static_cast<byte>(255 - value),
            static_cast<byte>(value ^ 0x80));
        reinterpret_cast<unsigned char *>(&color)[7] = 0xA5;
        unsigned char before[sizeof(B_Color)];
        std::memcpy(before, &color, sizeof(before));
        color.MakeCons();
        EXPECT_EQ(0, std::memcmp(before, &color, sizeof(before)));
    }
}

TEST(ColorTests, DistanceToReturnsSquaredDistance)
{
    B_Color first(10, 20, 30), second(13, 24, 30);
    EXPECT_FLOAT_EQ(25.0F, first.DistanceTo(second));
    EXPECT_FLOAT_EQ(25.0F, second.DistanceTo(first));
    EXPECT_FLOAT_EQ(0.0F, first.DistanceTo(first));
    B_Color black(0, 0, 0), white(255, 255, 255);
    EXPECT_FLOAT_EQ(195075.0F, black.DistanceTo(white));
    EXPECT_FLOAT_EQ(195075.0F, white.DistanceTo(black));
}

TEST(ColorTests, ScalingTruncatesAndWrapsChannels)
{
    const B_Color source(100, 132, 155);
    const B_Color half = source / 2.0F;
    EXPECT_EQ(50, half.r);
    EXPECT_EQ(66, half.g);
    EXPECT_EQ(77, half.b);
    const B_Color doubled = source * 2.0F;
    EXPECT_EQ(200, doubled.r);
    EXPECT_EQ(8, doubled.g);
    EXPECT_EQ(54, doubled.b);
    const B_Color negative = source * -0.5F;
    EXPECT_EQ(206, negative.r);
    EXPECT_EQ(190, negative.g);
    EXPECT_EQ(179, negative.b);
    EXPECT_EQ(100, source.r);
    EXPECT_EQ(132, source.g);
    EXPECT_EQ(155, source.b);
}

TEST(ColorTests, ScalingDoesNotInvokeDerivedMakeCons)
{
    struct DerivedColor : B_Color
    {
        DerivedColor() : B_Color(10, 20, 30), calls(0) {}
        void MakeCons() override { ++calls; }
        unsigned int calls;
    } source;
    const B_Color product = source * 2.0F;
    const B_Color quotient = source / 2.0F;
    EXPECT_EQ(0u, source.calls);
    EXPECT_EQ(20, product.r);
    EXPECT_EQ(5, quotient.r);
}

TEST(ColorTests, DistanceToUsesUnsignedChannelsAndPreservesObjects)
{
    B_Color first(0, 0x80, 0xFF), second(0xFF, 0x7F, 0);
    reinterpret_cast<unsigned char *>(&first)[7] = 0xA5;
    reinterpret_cast<unsigned char *>(&second)[7] = 0x5A;
    unsigned char beforeFirst[sizeof(B_Color)], beforeSecond[sizeof(B_Color)];
    std::memcpy(beforeFirst, &first, sizeof(beforeFirst));
    std::memcpy(beforeSecond, &second, sizeof(beforeSecond));
    EXPECT_FLOAT_EQ(130051.0F, first.DistanceTo(second));
    EXPECT_EQ(0, std::memcmp(beforeFirst, &first, sizeof(beforeFirst)));
    EXPECT_EQ(0, std::memcmp(beforeSecond, &second, sizeof(beforeSecond)));
}

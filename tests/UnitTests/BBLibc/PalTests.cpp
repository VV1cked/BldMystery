#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#include <cstring>
#include <new>
#define private public
#include <BBLibc/Pal.h>
#undef private


TEST(PalTests, SizeOf)
{
    EXPECT_EQ(sizeof(B_Pal), 0x0300);
}

TEST(PalTests, Fields)
{
    EXPECT_OFFSET_EQ(B_Pal, data, 0x0000);
}

TEST(PalTests, DefaultConstructorLeavesStorageUntouched)
{
    alignas(B_Pal) unsigned char storage[sizeof(B_Pal)];
    std::memset(storage, 0xA5, sizeof(storage));
    B_Pal *palette = new (storage) B_Pal;
    for (unsigned char value : storage)
        EXPECT_EQ(0xA5, value);
    palette->~B_Pal();
}

TEST(PalTests, CopyConstructorCopiesAllBytesIndependently)
{
    B_Pal original;
    for (unsigned int i = 0; i < sizeof(original.data); ++i)
        original.data[i] = static_cast<byte>(i);
    B_Pal copy(original);
    EXPECT_EQ(0, std::memcmp(original.data, copy.data, sizeof(original.data)));
    copy.SetBlue(255, 0);
    EXPECT_EQ(255, original.GetBlue(255));
    EXPECT_EQ(0, copy.GetBlue(255));
    EXPECT_EQ(&original, &original.Id());
    EXPECT_EQ(&copy, &copy.Id());
}

TEST(PalTests, GettersUsePackedRgbOrderForEveryEntry)
{
    B_Pal palette;
    for (unsigned int i = 0; i < sizeof(palette.data); ++i)
        palette.data[i] = static_cast<byte>(i);
    for (unsigned int index = 0; index < 256; ++index)
    {
        EXPECT_EQ(static_cast<byte>(3 * index), palette.GetRed(index));
        EXPECT_EQ(static_cast<byte>(3 * index + 1), palette.GetGreen(index));
        EXPECT_EQ(static_cast<byte>(3 * index + 2), palette.GetBlue(index));
    }
}

TEST(PalTests, SettersChangeOnlyTheRequestedChannel)
{
    B_Pal palette;
    for (unsigned int index = 0; index < 256; ++index)
    for (unsigned int channel = 0; channel < 3; ++channel)
    {
        std::memset(palette.data, 0xA5, sizeof(palette.data));
        const byte value = static_cast<byte>(index);
        if (channel == 0) palette.SetRed(index, value);
        if (channel == 1) palette.SetGreen(index, value);
        if (channel == 2) palette.SetBlue(index, value);
        for (unsigned int i = 0; i < sizeof(palette.data); ++i)
            EXPECT_EQ(i == 3 * index + channel ? value : 0xA5, palette.data[i]);
    }
}

TEST(PalTests, GetIndexUsesAllChannelsAndKeepsEarliestTie)
{
    B_Pal palette;
    std::memset(palette.data, 255, sizeof(palette.data));
    const byte candidates[] = {10, 0, 0, 0, 10, 0, 0, 0, 10};
    std::memcpy(palette.data + 7 * 3, candidates, sizeof(candidates));
    EXPECT_EQ(7u, palette.GetIndex(0, 0, 0, 0));
    EXPECT_EQ(7u, palette.GetIndex(10, 0, 0, 0));
    EXPECT_EQ(8u, palette.GetIndex(0, 10, 0, 0));
    EXPECT_EQ(9u, palette.GetIndex(0, 0, 10, 0));
    EXPECT_EQ(8u, palette.GetIndex(0, 0, 0, 8));
}

TEST(PalTests, GetIndexSearchesOnlyFromTheSuppliedStart)
{
    B_Pal palette;
    std::memset(palette.data, 255, sizeof(palette.data));
    palette.SetRed(0, 0);
    palette.SetGreen(0, 0);
    palette.SetBlue(0, 0);
    EXPECT_EQ(0u, palette.GetIndex(0, 0, 0, 0));
    EXPECT_EQ(100u, palette.GetIndex(0, 0, 0, 100));
    EXPECT_EQ(255u, palette.GetIndex(0, 0, 0, 255));
}

TEST(PalTests, LargeIndexCanWrapToAnInBoundsByteOffset)
{
    B_Pal palette;
    std::memset(palette.data, 0, sizeof(palette.data));
    // 3 * 0xAAAAAAAB wraps to byte offset 1, without accessing outside the palette.
    const unsigned int index = 0xAAAAAAABu;
    palette.SetRed(index, 10);
    palette.SetGreen(index, 20);
    palette.SetBlue(index, 30);
    EXPECT_EQ(10, palette.GetRed(index));
    EXPECT_EQ(20, palette.GetGreen(index));
    EXPECT_EQ(30, palette.GetBlue(index));
    EXPECT_EQ(10, palette.GetGreen(0));
    EXPECT_EQ(20, palette.GetBlue(0));
    EXPECT_EQ(30, palette.GetRed(1));
    EXPECT_EQ(index, palette.GetIndex(10, 20, 30, index));
}

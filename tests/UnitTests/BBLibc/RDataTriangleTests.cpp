#include <gtest/gtest.h>
#include <cstring>
#include <new>
#include <Utils/FieldOffset.h>
#define private public
#include <BBLibc/RDataLight.h>
#include <BBLibc/RDataTriangle.h>
#undef private

TEST(RDataTriangleTests, Layout)
{
    EXPECT_EQ(4u, sizeof(B_RItem));
    EXPECT_EQ(28u, sizeof(B_RDataTriangle));
    EXPECT_OFFSET_EQ(B_RItem, unknown00, 0);
    EXPECT_OFFSET_EQ(B_RDataTriangle, unknown04, 4);
}

TEST(RDataTriangleTests, ItemConstructionAndAssignmentPreserveFullWord)
{
    B_RItem source(0xFEDCBA98ul), destination(0);
    EXPECT_EQ(0xFEDCBA98ul, source.unknown00);
    EXPECT_EQ(&destination, &(destination = source));
    EXPECT_EQ(0xFEDCBA98ul, destination.unknown00);
    EXPECT_EQ(&destination, &(destination = destination));
    EXPECT_EQ(0xFEDCBA98ul, destination.unknown00);
}

TEST(RDataTriangleTests, ConstructorInitializesOnlyBase)
{
    alignas(B_RDataTriangle) unsigned char bytes[sizeof(B_RDataTriangle)];
    std::memset(bytes, 0xA5, sizeof(bytes));
    B_RDataTriangle *object = new (bytes) B_RDataTriangle();
    EXPECT_EQ(1ul, object->unknown00);
    for (unsigned int i = 4; i < sizeof(bytes); ++i)
        EXPECT_EQ(0xA5, bytes[i]);
    object->~B_RDataTriangle();
}

TEST(RDataTriangleTests, AssignmentCopiesAllSevenWords)
{
    const unsigned int words[7] = {0xFFFFFFFFu, 0, 1, 0x80000000u,
        0x7F800001u, 0x12345678u, 0x87654321u};
    B_RDataTriangle source, destination;
    std::memcpy(&source, words, sizeof(words));
    EXPECT_EQ(&destination, &(destination = source));
    EXPECT_EQ(0, std::memcmp(&destination, words, sizeof(words)));
    EXPECT_EQ(0, std::memcmp(&source, words, sizeof(words)));
    EXPECT_EQ(&destination, &(destination = destination));
    EXPECT_EQ(0, std::memcmp(&destination, words, sizeof(words)));
}

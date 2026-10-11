#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <cstring>
#include <new>
#define private public
#include <BBLibc/MeshEdge.h>
#undef private

TEST(MeshEdgeTests, Layout)
{
    EXPECT_EQ(0x18u, sizeof(B_MeshEdge));
    EXPECT_OFFSET_EQ(B_MeshEdge, field, 0x00);
    EXPECT_OFFSET_EQ(B_MeshEdge, firstIndex, 0x04);
    EXPECT_OFFSET_EQ(B_MeshEdge, secondIndex, 0x08);
    EXPECT_OFFSET_EQ(B_MeshEdge, field_0C, 0x0C);
    EXPECT_OFFSET_EQ(B_MeshEdge, field_10, 0x10);
    EXPECT_OFFSET_EQ(B_MeshEdge, field_14, 0x14);
}

TEST(MeshEdgeTests, DefaultConstructorPreservesUntouchedWords)
{
    alignas(B_MeshEdge) unsigned int storage[6];
    for (unsigned int &word : storage) word = 0xA5A5A5A5u;
    B_MeshEdge *edge = new (storage) B_MeshEdge;
    const unsigned int expected[] = {0xA5A5A5A5u, 0, 0, 0xA5A5A5A5u, 0xA5A5A5A5u, 1};
    EXPECT_EQ(0, std::memcmp(storage, expected, sizeof(storage)));
    edge->~B_MeshEdge();
}

TEST(MeshEdgeTests, ConstructorOrdersUnsignedIndicesAndPreservesUnknownWords)
{
    const unsigned int values[] = {0, 1, 0x7FFFFFFFu, 0x80000000u, 0xFFFFFFFFu};
    for (unsigned int a : values)
    for (unsigned int b : values)
    {
        alignas(B_MeshEdge) unsigned int storage[6];
        std::memset(storage, 0xA5, sizeof(storage));
        B_MeshEdge *edge = new (storage) B_MeshEdge(a, b, 0xDEADBEEFu);
        EXPECT_EQ(0xDEADBEEFu, edge->field);
        EXPECT_EQ(a < b ? a : b, edge->firstIndex);
        EXPECT_EQ(a > b ? a : b, edge->secondIndex);
        EXPECT_EQ(0xA5A5A5A5u, edge->field_0C);
        EXPECT_EQ(0xA5A5A5A5u, edge->field_10);
        EXPECT_EQ(1u, edge->field_14);
        edge->~B_MeshEdge();
    }
}

TEST(MeshEdgeTests, AssignmentCopiesAllWordsAndReturnsDestination)
{
    B_MeshEdge source, destination;
    const unsigned int words[] = {1, 2, 3, 4, 5, 6};
    std::memcpy(&source, words, sizeof(source));
    EXPECT_EQ(&destination, &(destination = source));
    EXPECT_EQ(0, std::memcmp(&destination, words, sizeof(destination)));
    EXPECT_EQ(&destination, &(destination = destination));
    EXPECT_EQ(0, std::memcmp(&destination, words, sizeof(destination)));
    source.field_10 = 7;
    EXPECT_EQ(5u, destination.field_10);
}

TEST(MeshEdgeTests, EqualityUsesOnlyTheStoredPair)
{
    B_MeshEdge a(1, 2, 3), b(2, 1, 99), different(1, 3, 3);
    a.field_0C = 10; a.field_10 = 20; a.field_14 = 30;
    b.field_0C = 40; b.field_10 = 50; b.field_14 = 60;
    EXPECT_EQ(1u, a == b);
    EXPECT_EQ(0u, a == different);
    // Equality does not sort an already stored pair.
    b.firstIndex = 2; b.secondIndex = 1;
    EXPECT_EQ(0u, a == b);
}

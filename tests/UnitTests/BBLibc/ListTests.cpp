#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#include <cstring>
#define private public
#include <BBLibc/List.h>
#undef private


TEST(ListTests, SizeOf)
{
    EXPECT_EQ(sizeof(B_List), 0x0010);
}

TEST(ListTests, Fields)
{
    EXPECT_OFFSET_EQ(B_List, numElements, 0x0004);
    EXPECT_OFFSET_EQ(B_List, tail, 0x0008);
    EXPECT_OFFSET_EQ(B_List, head, 0x000C);
}

TEST(ListTests, ConstructorIgnoresInitialElement)
{
    B_ListElement element;
    B_List list(&element);
    EXPECT_EQ(0u, list.nElements());
    EXPECT_EQ(nullptr, list.GetFirst());
    EXPECT_EQ(nullptr, list.GetLast());
    EXPECT_EQ(nullptr, element.GetList());
    EXPECT_EQ(nullptr, element.GetNext());
    EXPECT_EQ(nullptr, element.GetPrev());
}

TEST(ListTests, AccessorsReturnStoredFieldsRegardlessOfCount)
{
    B_ListElement first, last;
    B_List list(nullptr);
    list.tail = &first;
    list.head = &last;
    list.numElements = -1;
    EXPECT_EQ(~0u, list.nElements());
    EXPECT_EQ(&first, list.GetFirst());
    EXPECT_EQ(&last, list.GetLast());
    list.numElements = 0;
    EXPECT_EQ(&first, list.GetFirst());
    EXPECT_EQ(&last, list.GetLast());
}

TEST(ListTests, GetAtFollowsNextAndAcceptsOnePastEnd)
{
    B_ListElement first, second, third;
    first.next = &second;
    second.next = &third;
    second.prev = &first;
    third.prev = &second;
    B_List list(nullptr);
    list.tail = &first;
    list.head = &third;
    list.numElements = 3;
    EXPECT_EQ(&first, list.GetAt(0));
    EXPECT_EQ(&second, list.GetAt(1));
    EXPECT_EQ(&third, list.GetAt(2));
    EXPECT_EQ(nullptr, list.GetAt(3));
    EXPECT_EQ(nullptr, list.GetAt(4));
    EXPECT_EQ(nullptr, list.GetAt(~0u));
}

TEST(ListTests, GetAtUsesStrictCountBoundaryWithoutValidatingLinks)
{
    B_ListElement first, sentinel;
    first.next = &sentinel;
    B_List list(nullptr);
    list.tail = &first;
    list.numElements = 0;
    EXPECT_EQ(&first, list.GetAt(0));
    EXPECT_EQ(nullptr, list.GetAt(1));
    list.numElements = 1;
    EXPECT_EQ(&sentinel, list.GetAt(1));
    list.numElements = -1;
    EXPECT_EQ(&sentinel, list.GetAt(1));
}

TEST(ListTests, ExistsScansLinksIgnoringCountAndOwner)
{
    B_ListElement first, second, foreign;
    B_List list(nullptr), other(nullptr);
    first.next = &second;
    list.tail = &first;
    list.numElements = 0;
    second.list = &other;
    foreign.list = &list;
    EXPECT_EQ(1u, list.Exists(&first));
    EXPECT_EQ(1u, list.Exists(&second));
    EXPECT_EQ(0u, list.Exists(&foreign));
    EXPECT_EQ(0u, list.Exists(nullptr));
    EXPECT_EQ(0u, other.Exists(&first));
    second.list = foreign.list = nullptr;
}

TEST(ListTests, CopyConstructorSharesNodesWithoutChangingTheirOwner)
{
    B_ListElement first, last;
    B_List original(nullptr);
    original.numElements = 2;
    original.tail = &first;
    original.head = &last;
    first.next = &last;
    last.prev = &first;
    first.list = last.list = &original;
    B_List copy(original);
    EXPECT_EQ(2u, copy.nElements());
    EXPECT_EQ(&first, copy.GetFirst());
    EXPECT_EQ(&last, copy.GetLast());
    EXPECT_EQ(&original, first.GetList());
    EXPECT_EQ(&original, last.GetList());
    EXPECT_EQ(&last, first.next);
    EXPECT_EQ(&first, last.prev);
    first.list = last.list = nullptr;
}

TEST(ListTests, ElementAssignmentCopiesLinksWithoutRelinkingOrChangingVtable)
{
    struct DerivedElement : B_ListElement {} destination;
    B_ListElement source, before, after;
    B_List list(nullptr);
    source.list = &list;
    source.next = &after;
    source.prev = &before;
    before.next = after.prev = &source;
    unsigned char vtable[sizeof(void *)];
    std::memcpy(vtable, &destination, sizeof(vtable));
    EXPECT_EQ(&destination, &(static_cast<B_ListElement &>(destination) = source));
    EXPECT_EQ(&list, destination.GetList());
    EXPECT_EQ(&before, destination.prev);
    EXPECT_EQ(&after, destination.next);
    EXPECT_EQ(&source, before.next);
    EXPECT_EQ(&source, after.prev);
    EXPECT_EQ(0, std::memcmp(vtable, &destination, sizeof(vtable)));
    EXPECT_EQ(&source, &(source = source));
    source.list = destination.list = nullptr;
}

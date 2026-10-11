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

struct ListMutationFixture
{
    ListMutationFixture() : list(nullptr) {}
    ~ListMutationFixture()
    {
        // Avoid automatic removal while testing deliberately inconsistent original states.
        for (B_ListElement &node : nodes)
            node.list = nullptr;
    }
    B_List list;
    B_ListElement nodes[8];
};

TEST(ListTests, AddPrependsAndMaintainsBothEnds)
{
    ListMutationFixture f;
    EXPECT_EQ(1u, f.list.Add(&f.nodes[0]));
    EXPECT_EQ(1u, f.list.Add(&f.nodes[1]));
    EXPECT_EQ(2u, f.list.nElements());
    EXPECT_EQ(&f.nodes[1], f.list.GetFirst());
    EXPECT_EQ(&f.nodes[0], f.list.GetLast());
    EXPECT_EQ(&f.nodes[0], f.nodes[1].next);
    EXPECT_EQ(nullptr, f.nodes[1].prev);
    EXPECT_EQ(&f.nodes[1], f.nodes[0].prev);
    EXPECT_EQ(nullptr, f.nodes[0].next);
    EXPECT_EQ(&f.list, f.nodes[0].list);
    EXPECT_EQ(&f.list, f.nodes[1].list);
}

TEST(ListTests, EmptyAddPreservesOldLinksAndDoesNotDetachPreviousOwner)
{
    ListMutationFixture f;
    B_List oldOwner(nullptr);
    f.nodes[0].list = &oldOwner;
    f.nodes[0].next = &f.nodes[1];
    f.nodes[0].prev = &f.nodes[2];
    oldOwner.tail = oldOwner.head = &f.nodes[0];
    oldOwner.numElements = 1;
    EXPECT_EQ(1u, f.list.Add(&f.nodes[0]));
    EXPECT_EQ(&f.nodes[1], f.nodes[0].next);
    EXPECT_EQ(&f.nodes[2], f.nodes[0].prev);
    EXPECT_EQ(&f.list, f.nodes[0].list);
    EXPECT_EQ(&f.nodes[0], oldOwner.GetFirst());
    EXPECT_EQ(1u, oldOwner.nElements());
    f.list.numElements = 0x7FFFFFFF;
    EXPECT_EQ(1u, f.list.Add(&f.nodes[3]));
    EXPECT_EQ(0x80000000u, f.list.nElements());
}

TEST(ListTests, InsertionsTakeNewNodeBeforeAnchor)
{
    ListMutationFixture f;
    f.list.Add(&f.nodes[0]);
    EXPECT_EQ(1u, f.list.AddAfter(&f.nodes[1], &f.nodes[0]));
    EXPECT_EQ(1u, f.list.AddBefore(&f.nodes[2], &f.nodes[0]));
    EXPECT_EQ(1u, f.list.AddAfter(&f.nodes[3], &f.nodes[0]));
    EXPECT_EQ(1u, f.list.AddBefore(&f.nodes[4], &f.nodes[1]));
    const unsigned int order[] = {2, 0, 3, 4, 1};
    ASSERT_EQ(5u, f.list.nElements());
    for (unsigned int i = 0; i < 5; ++i)
    {
        EXPECT_EQ(&f.nodes[order[i]], f.list.GetAt(i));
        EXPECT_EQ(&f.list, f.nodes[order[i]].list);
        EXPECT_EQ(i == 0 ? nullptr : &f.nodes[order[i - 1]], f.nodes[order[i]].prev);
        EXPECT_EQ(i == 4 ? nullptr : &f.nodes[order[i + 1]], f.nodes[order[i]].next);
    }
}

TEST(ListTests, DuplicateInsertionStillChangesCountAndLinks)
{
    ListMutationFixture f;
    f.list.Add(&f.nodes[0]);
    EXPECT_EQ(1u, f.list.AddAfter(&f.nodes[0], &f.nodes[0]));
    EXPECT_EQ(2u, f.list.nElements());
    EXPECT_EQ(&f.nodes[0], f.nodes[0].prev);
    EXPECT_EQ(nullptr, f.nodes[0].next);
    EXPECT_EQ(1u, f.list.AddBefore(&f.nodes[0], &f.nodes[0]));
    EXPECT_EQ(3u, f.list.nElements());
    EXPECT_EQ(&f.nodes[0], f.nodes[0].next);
    EXPECT_EQ(nullptr, f.nodes[0].prev);
}

TEST(ListTests, RemoveDetachesMiddleAndEndpoints)
{
    ListMutationFixture f;
    EXPECT_EQ(0u, f.list.Remove(nullptr, 0));
    EXPECT_EQ(nullptr, f.list.RemoveFirst(0));
    EXPECT_EQ(nullptr, f.list.RemoveLast(0));
    for (unsigned int i = 0; i < 3; ++i)
        f.list.Add(&f.nodes[i]);
    EXPECT_EQ(1u, f.list.Remove(&f.nodes[1], 0));
    EXPECT_EQ(2u, f.list.nElements());
    EXPECT_EQ(&f.nodes[0], f.nodes[2].next);
    EXPECT_EQ(&f.nodes[2], f.nodes[0].prev);
    EXPECT_EQ(nullptr, f.nodes[1].list);
    EXPECT_EQ(nullptr, f.nodes[1].next);
    EXPECT_EQ(nullptr, f.nodes[1].prev);
    EXPECT_EQ(1u, f.list.Remove(&f.nodes[2], 0));
    EXPECT_EQ(1u, f.list.Remove(&f.nodes[0], 0));
    EXPECT_EQ(0u, f.list.nElements());
    EXPECT_EQ(0u, f.list.Remove(&f.nodes[0], 0));
}

TEST(ListTests, LastRemovalLeavesOppositeEndpointUnchanged)
{
    ListMutationFixture first, last;
    first.list.Add(&first.nodes[0]);
    EXPECT_EQ(&first.nodes[0], first.list.RemoveFirst(0));
    EXPECT_EQ(0u, first.list.nElements());
    EXPECT_EQ(nullptr, first.list.GetFirst());
    EXPECT_EQ(&first.nodes[0], first.list.GetLast());
    EXPECT_EQ(nullptr, first.list.RemoveFirst(0));
    last.list.Add(&last.nodes[0]);
    EXPECT_EQ(&last.nodes[0], last.list.RemoveLast(0));
    EXPECT_EQ(0u, last.list.nElements());
    EXPECT_EQ(&last.nodes[0], last.list.GetFirst());
    EXPECT_EQ(nullptr, last.list.GetLast());
    EXPECT_EQ(nullptr, last.list.RemoveLast(0));
}

TEST(ListTests, NonDeletingDeleteListForgetsChainWithoutDetachingNodes)
{
    ListMutationFixture f;
    f.list.Add(&f.nodes[0]);
    f.list.Add(&f.nodes[1]);
    f.list.DeleteList(0);
    EXPECT_EQ(0u, f.list.nElements());
    EXPECT_EQ(nullptr, f.list.GetFirst());
    EXPECT_EQ(nullptr, f.list.GetLast());
    EXPECT_EQ(&f.list, f.nodes[0].list);
    EXPECT_EQ(&f.list, f.nodes[1].list);
    EXPECT_EQ(&f.nodes[0], f.nodes[1].next);
    EXPECT_EQ(&f.nodes[1], f.nodes[0].prev);
}

struct ListDeletionObserver : B_ListElement
{
    ListDeletionObserver(B_List &observed, unsigned int &count, bool &detached,
        unsigned int &destructions)
        : observed(observed), count(count), detached(detached), destructions(destructions) {}
    ~ListDeletionObserver() override
    {
        count = observed.nElements();
        detached = list == nullptr && next == nullptr && prev == nullptr;
        ++destructions;
    }
    B_List &observed;
    unsigned int &count;
    bool &detached;
    unsigned int &destructions;
};

TEST(ListTests, DeletionCountChangesBeforeEndpointAndAfterMiddleDestructor)
{
    ListMutationFixture f;
    unsigned int count = ~0u, destructions = 0;
    bool detached = false;
    B_ListElement *middle = new ListDeletionObserver(f.list, count, detached, destructions);
    f.list.Add(&f.nodes[0]);
    f.list.Add(middle);
    f.list.Add(&f.nodes[1]);
    EXPECT_EQ(1u, f.list.Remove(middle, 2));
    EXPECT_EQ(3u, count);
    EXPECT_EQ(2u, f.list.nElements());
    EXPECT_TRUE(detached);
    EXPECT_EQ(1u, destructions);
    f.list.Add(new ListDeletionObserver(f.list, count, detached, destructions));
    EXPECT_EQ(nullptr, f.list.RemoveFirst(~0u));
    EXPECT_EQ(2u, count);
    EXPECT_TRUE(detached);
    EXPECT_EQ(2u, destructions);
    f.list.AddAfter(new ListDeletionObserver(f.list, count, detached, destructions), &f.nodes[0]);
    EXPECT_EQ(nullptr, f.list.RemoveLast(2));
    EXPECT_EQ(2u, count);
    EXPECT_TRUE(detached);
    EXPECT_EQ(3u, destructions);
}

TEST(ListTests, DeleteListWithNonzeroFlagDeletesAndClearsBothEndpoints)
{
    B_List list(nullptr);
    unsigned int count = ~0u, destructions = 0;
    bool detached = false;
    for (unsigned int i = 0; i < 3; ++i)
        list.Add(new ListDeletionObserver(list, count, detached, destructions));
    list.DeleteList(2);
    EXPECT_EQ(3u, destructions);
    EXPECT_EQ(0u, count);
    EXPECT_TRUE(detached);
    EXPECT_EQ(0u, list.nElements());
    EXPECT_EQ(nullptr, list.GetFirst());
    EXPECT_EQ(nullptr, list.GetLast());
}

TEST(ListTests, ElementDestructorAutomaticallyUnlinksFromItsOwner)
{
    ListMutationFixture f;
    f.list.Add(&f.nodes[0]);
    B_ListElement *element = new B_ListElement;
    f.list.Add(element);
    f.list.Add(&f.nodes[1]);
    delete element;
    EXPECT_EQ(2u, f.list.nElements());
    EXPECT_EQ(&f.nodes[0], f.nodes[1].next);
    EXPECT_EQ(&f.nodes[1], f.nodes[0].prev);
}

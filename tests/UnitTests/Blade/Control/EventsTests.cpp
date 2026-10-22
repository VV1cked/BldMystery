#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#include <bld_python.h>
#define private public
#include <Control/Events.h>


TEST(EventsTests, Fields)
{
    EXPECT_OFFSET_EQ(B_Events, events, 0x0000);
    EXPECT_OFFSET_EQ(B_Events, events.elements, 0x0004);
    EXPECT_OFFSET_EQ(B_Events, events.size, 0x0008);
    EXPECT_OFFSET_EQ(B_Events, eventHash, 0x0018);
    EXPECT_OFFSET_EQ(B_Events, eventHash.hash, 0x001C);
    EXPECT_OFFSET_EQ(B_Events, eventHash.unknown1804, 0x181C);
    EXPECT_OFFSET_EQ(B_Events, eventHash.lastItem, 0x1820);
    EXPECT_EQ(sizeof(B_Events), 0x1824u);
}

TEST(EventsTests, RegistryInitialization)
{
    B_Events events;
    EXPECT_EQ(events.events.size, 0u);
    EXPECT_EQ(events.events.increment, 16);
    EXPECT_EQ(events.eventHash.size, 0u);
    EXPECT_EQ(events.eventHash.unknown1804, 1);
    EXPECT_EQ(events.eventHash.lastItem, nullptr);
    for (unsigned i = 0; i < 256; ++i)
    {
        EXPECT_EQ(events.eventHash.hash[i].size, 0u);
        EXPECT_EQ(events.eventHash.hash[i].elements, nullptr);
        EXPECT_EQ(events.eventHash.hash[i].increment, 16);
    }
}

TEST(EventsTests, RepeatedCaseSensitiveAndCollidingNames)
{
    B_Events events;
    EXPECT_EQ(events.GetEventIndex("AB"), 0);
    EXPECT_EQ(events.GetEventIndex("BA"), 1);
    EXPECT_EQ(events.GetEventIndex("AB"), 0);
    EXPECT_EQ(events.GetEventIndex("ab"), 2);
    EXPECT_EQ(events.GetEventIndex(""), 3);
    EXPECT_EQ(events.GetEventIndex(""), 3);
    EXPECT_STREQ(events.GetEventName(0).String(), "AB");
    EXPECT_STREQ(events.GetEventName(1).String(), "BA");
    EXPECT_STREQ(events.GetEventName(2).String(), "ab");
    EXPECT_EQ(events.events.size, 4u);
    EXPECT_EQ(events.eventHash.Size(), 4u);
}

namespace {
int destroyedEvents[2];
unsigned destroyedCount;
class TrackedEvent : public B_Event
{
public:
    TrackedEvent(const char *name, int index) : B_Event(B_Name(name), index) {}
    ~TrackedEvent() { destroyedEvents[destroyedCount++] = index; }
};
}

TEST(EventsTests, HashOwnsEventsAndIndexArraySharesThem)
{
    destroyedCount = 0;
    {
        B_Events events;
        B_Event *b = new TrackedEvent("B", 0);
        B_Event *a = new TrackedEvent("A", 1);
        events.events.Append(b);
        events.events.Append(a);
        events.eventHash.Add(b);
        events.eventHash.Add(a);
    }
    ASSERT_EQ(destroyedCount, 2u);
    EXPECT_EQ(destroyedEvents[0], 1); // Ascending hash buckets, not index order.
    EXPECT_EQ(destroyedEvents[1], 0);
}

namespace {
PyObject *EventNoop(PyObject *, PyObject *)
{
    Py_INCREF(Py_None);
    return Py_None;
}
int EventNativeMarker(B_Entity *, unsigned int) { return -17; }
}

TEST(EventsTests, PythonRegistrationOwnershipAndNativePreservation)
{
    Py_Initialize();
    PyMethodDef def = {"event_noop", EventNoop, METH_VARARGS, NULL};
    PyObject *old = PyCFunction_New(&def, NULL);
    PyObject *replacement = PyCFunction_New(&def, NULL);
    ASSERT_TRUE(old != NULL && replacement != NULL);
    const long oldRefs = old->ob_refcnt, newRefs = replacement->ob_refcnt;
    B_EventFuncs table(B_Name("fixture"));
    table.AddFunc(31, EventNativeMarker, "native");
    table.AddPyFunc(31, old, "old");
    EXPECT_EQ(old->ob_refcnt, oldRefs + 1);
    table.AddPyFunc(31, old, "same");
    EXPECT_EQ(old->ob_refcnt, oldRefs + 1);
    table.AddPyFunc(31, replacement, "new");
    EXPECT_EQ(old->ob_refcnt, oldRefs);
    EXPECT_EQ(replacement->ob_refcnt, newRefs + 1);
    EXPECT_EQ(table.eventFuncs.elements[0][31].func, &EventNativeMarker);
    table.AddPyFunc(32, old, "boundary");
    EXPECT_EQ(table.eventFuncs.size, 2u);
    EXPECT_EQ(table.eventFuncs.elements[1][0].pyFunc, old);
    table.AddPyFunc(31, NULL, "removed");
    EXPECT_EQ(replacement->ob_refcnt, newRefs);
    EXPECT_EQ(table.CallFunc(NULL, 31), -17); // Candidate-only supporting fixture.
    table.AddPyFunc(32, NULL, "removed");
    EXPECT_EQ(old->ob_refcnt, oldRefs);
    // Constructor/destructor replacement is a separate unresolved work package.
    // Release fixture blocks explicitly; the inherited implicit destructor only
    // releases the pointer array, not these pointees.
    for (unsigned i = 0; i < table.eventFuncs.size; ++i)
    {
        delete [] table.eventFuncs.elements[i];
        table.eventFuncs.elements[i] = NULL;
    }
    B_EventFuncs sparse(B_Name("sparse"));
    sparse.AddPyFunc(96, old, "first");
    EXPECT_EQ(sparse.eventFuncs.size, 4u);
    EXPECT_EQ(sparse.eventFuncs.elements[0], nullptr);
    EXPECT_EQ(sparse.eventFuncs.elements[1], nullptr);
    EXPECT_EQ(sparse.eventFuncs.elements[2], nullptr);
    EXPECT_EQ(sparse.eventFuncs.elements[3][0].pyFunc, old);
    EXPECT_EQ(old->ob_refcnt, oldRefs + 1);
    sparse.AddPyFunc(96, NULL, "removed");
    EXPECT_EQ(old->ob_refcnt, oldRefs);
    delete [] sparse.eventFuncs.elements[3];
    sparse.eventFuncs.elements[3] = NULL;
    Py_DECREF(old);
    Py_DECREF(replacement);
}

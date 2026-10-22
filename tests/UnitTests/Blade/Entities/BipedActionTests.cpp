#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#include <Entities/BipedEntity.h>
#include <string.h>

extern "C" { extern B_Events gbl_events; }


TEST(BipedActionTests, SizeOf)
{
    EXPECT_EQ(sizeof(B_BipedAction), 0x38);
}

TEST(BipedActionTests, Fields)
{
    EXPECT_OFFSET_EQ(B_BipedAction, animationEvents, 0x024);
    EXPECT_OFFSET_EQ(B_BipedAction, eventTable, 0x034);
    EXPECT_OFFSET_EQ(B_AnimationEventDefinition, phases, 0x010);
    EXPECT_OFFSET_EQ(B_AnimationEventDefinition, numPhases, 0x014);
    EXPECT_OFFSET_EQ(B_AnimationEventDefinition, eventIndices, 0x024);
    EXPECT_EQ(sizeof(B_AnimationEventDefinition), 0x28u); // Borrowed prefix only.
    EXPECT_EQ(__alignof(B_BipedAction), 4u);
}

namespace {
float FloatBits(unsigned bits)
{
    float value;
    memcpy(&value, &bits, sizeof(value));
    return value;
}

struct ActionFixture
{
    B_BipedAction action;
    B_AnimationEventDefinition definition;

    ActionFixture(float *phases, int *ids, unsigned count)
    {
        memset(action.unknown_fields, 0, sizeof(action.unknown_fields));
        memset(action.unknown_fields28, 0, sizeof(action.unknown_fields28));
        memset(&definition, 0, sizeof(definition));
        definition.phases = phases;
        definition.numPhases = count;
        definition.eventIndices = ids;
        action.animationEvents = &definition;
        action.eventTable = reinterpret_cast<B_EventFuncs *>(0x1234);
    }
};
}

TEST(BipedActionTests, NullDefinitionDoesNotInternName)
{
    ActionFixture fixture(NULL, NULL, 0);
    fixture.action.animationEvents = NULL;
    // Compare with a subsequent registration to detect a hidden lookup.
    int before = gbl_events.GetEventIndex("GetEventTimeTest_NullBefore");
    EXPECT_EQ(fixture.action.GetEventTime("GetEventTimeTest_NullQuery"), -1.0);
    EXPECT_EQ(gbl_events.GetEventIndex("GetEventTimeTest_NullAfter"), before + 1);
}

TEST(BipedActionTests, MissingNameIsInternedEvenForEmptyDefinition)
{
    ActionFixture fixture(NULL, NULL, 0);
    int before = gbl_events.GetEventIndex("GetEventTimeTest_EmptyBefore");
    EXPECT_EQ(fixture.action.GetEventTime("GetEventTimeTest_EmptyQuery"), -1.0);
    EXPECT_EQ(gbl_events.GetEventIndex("GetEventTimeTest_EmptyQuery"), before + 1);
    EXPECT_EQ(fixture.action.GetEventTime("GetEventTimeTest_EmptyQuery"), -1.0);
    EXPECT_EQ(gbl_events.GetEventIndex("GetEventTimeTest_EmptyAfter"), before + 2);
}

TEST(BipedActionTests, StrictFloatThresholdAndStoredPrecision)
{
    int id = gbl_events.GetEventIndex("GetEventTimeTest_Threshold");
    int ids[] = {id, id, id};
    float phases[] = {FloatBits(0xB58637BE), FloatBits(0xB58637BD), FloatBits(0xB58637BC)};
    ActionFixture fixture(phases, ids, 2);
    EXPECT_EQ(fixture.action.GetEventTime("GetEventTimeTest_Threshold"), -1.0);
    fixture.definition.numPhases = 3;
    EXPECT_EQ(fixture.action.GetEventTime("GetEventTimeTest_Threshold"),
              -9.9999988378840498626232147216796875e-7);
    phases[0] = FloatBits(0x3ED4FDF4); // Stored float for 0.416, not double 0.416.
    fixture.definition.numPhases = 1;
    EXPECT_EQ(fixture.action.GetEventTime("GetEventTimeTest_Threshold"),
              0.41600000858306884765625);
}

TEST(BipedActionTests, FirstMatchingStoredEntryAndReadOnlyStorage)
{
    int id = gbl_events.GetEventIndex("GetEventTimeTest_Order");
    int ids[] = {id + 1, id, id};
    float phases[] = {0.25f, 0.5f, 0.75f};
    ActionFixture fixture(phases, ids, 3);
    unsigned char actionBefore[sizeof(fixture.action)];
    unsigned char definitionBefore[sizeof(fixture.definition)];
    memcpy(actionBefore, &fixture.action, sizeof(actionBefore));
    memcpy(definitionBefore, &fixture.definition, sizeof(definitionBefore));
    EXPECT_EQ(fixture.action.GetEventTime("GetEventTimeTest_Order"), 0.5);
    EXPECT_EQ(memcmp(actionBefore, &fixture.action, sizeof(actionBefore)), 0);
    EXPECT_EQ(memcmp(definitionBefore, &fixture.definition, sizeof(definitionBefore)), 0);
    EXPECT_EQ(phases[0], 0.25f);
    EXPECT_EQ(phases[1], 0.5f);
    EXPECT_EQ(phases[2], 0.75f);
    EXPECT_EQ(ids[0], id + 1);
    EXPECT_EQ(ids[1], id);
    EXPECT_EQ(ids[2], id);
}

TEST(BipedActionTests, UnorderedInfinityAndSignedZero)
{
    int id = gbl_events.GetEventIndex("GetEventTimeTest_Special");
    int ids[] = {id};
    float phase = FloatBits(0x7FC00000);
    ActionFixture fixture(&phase, ids, 1);
    double value = fixture.action.GetEventTime("GetEventTimeTest_Special");
    EXPECT_NE(value, value); // Original FCOMP unordered sets C0 and accepts NaN.
    phase = FloatBits(0xFF800000);
    EXPECT_EQ(fixture.action.GetEventTime("GetEventTimeTest_Special"), -1.0);
    phase = FloatBits(0x7F800000);
    EXPECT_EQ(fixture.action.GetEventTime("GetEventTimeTest_Special"), static_cast<double>(phase));
    phase = FloatBits(0x80000000);
    value = fixture.action.GetEventTime("GetEventTimeTest_Special");
    unsigned words[2];
    memcpy(words, &value, sizeof(value));
    EXPECT_EQ(words[0], 0u);
    EXPECT_EQ(words[1], 0x80000000u);
}

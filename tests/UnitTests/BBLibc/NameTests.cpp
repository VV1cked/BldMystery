#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#define private public
#include <BBLibc/Name.h>


TEST(NameTests, SizeOf)
{
    EXPECT_EQ(sizeof(B_Name), 0x0008);
}

TEST(NameTests, Fields)
{
    EXPECT_OFFSET_EQ(B_Name, string, 0x0000);
    EXPECT_OFFSET_EQ(B_Name, len, 0x0004);
}


TEST(NameTests, CompareNoCaseEqualNames)
{
    const B_Name name("Blade");
    const B_Name same("Blade");
    const B_Name mixedCase("bLADE");

    EXPECT_EQ(1u, name.CompareNoCase(same));
    EXPECT_EQ(1u, name.CompareNoCase(mixedCase));
    EXPECT_EQ(1u, mixedCase.CompareNoCase(name));
    EXPECT_EQ(1u, name.CompareNoCase(name));
}


TEST(NameTests, CompareNoCaseDifferentNamesOfEqualLength)
{
    const B_Name name("Blade");
    const B_Name other("Blame");

    EXPECT_EQ(0u, name.CompareNoCase(other));
    EXPECT_EQ(0u, other.CompareNoCase(name));
}


TEST(NameTests, CompareNoCaseDifferentLengths)
{
    const B_Name name("Blade");
    const B_Name prefix("Blad");

    EXPECT_EQ(0u, name.CompareNoCase(prefix));
    EXPECT_EQ(0u, prefix.CompareNoCase(name));
}


TEST(NameTests, CompareNoCaseEmptyNames)
{
    const B_Name empty;
    const B_Name alsoEmpty("");
    const B_Name nonEmpty("Blade");

    EXPECT_EQ(1u, empty.CompareNoCase(alsoEmpty));
    EXPECT_EQ(0u, empty.CompareNoCase(nonEmpty));
    EXPECT_EQ(0u, nonEmpty.CompareNoCase(empty));
}


TEST(NameTests, CompareNoCaseChecksStoredLengthBeforeStrings)
{
    B_Name name("Blade");
    const B_Name prefix("bl");
    // String() exposes a mutable buffer without updating the stored length.
    name.String()[2] = '\0';

    EXPECT_EQ(0u, name.CompareNoCase(prefix));
    EXPECT_EQ(0u, prefix.CompareNoCase(name));
}


TEST(NameTests, CompareNoCaseStopsAtNullForEqualStoredLengths)
{
    B_Name name("Blade");
    B_Name other("bLxyz");
    name.String()[2] = '\0';
    other.String()[2] = '\0';

    EXPECT_EQ(1u, name.CompareNoCase(other));
    EXPECT_EQ(1u, other.CompareNoCase(name));
}


TEST(NameTests, EqualityMatchesIdenticalStrings)
{
    const char *values[] = {"", "a", "ab", "abc", "Blade", "\x80\xFF"};
    for (unsigned int i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    {
        const B_Name name(values[i]);
        const B_Name same(values[i]);
        EXPECT_EQ(1u, name == same);
        EXPECT_EQ(1u, name == values[i]);
        EXPECT_EQ(1u, name == name);
    }
}


TEST(NameTests, EqualityIsCaseSensitive)
{
    const B_Name name("Blade");
    const B_Name mixedCase("bLADE");
    EXPECT_EQ(0u, name == mixedCase);
    EXPECT_EQ(0u, mixedCase == name);
    EXPECT_EQ(0u, name == "bLADE");
}


TEST(NameTests, EqualityRejectsDifferentBytesAndPrefixes)
{
    const B_Name name("abcd");
    const char *others[] = {"", "xbcd", "axcd", "abxd", "abcx", "abc", "abcde"};
    for (unsigned int i = 0; i < sizeof(others) / sizeof(others[0]); ++i)
    {
        const B_Name other(others[i]);
        EXPECT_EQ(0u, name == other);
        EXPECT_EQ(0u, other == name);
        EXPECT_EQ(0u, name == others[i]);
    }
    const B_Name highBytes("\x80\xFF");
    EXPECT_EQ(0u, highBytes == B_Name("\x80\xFE"));
    EXPECT_EQ(0u, highBytes == "\x80\xFE");
}


TEST(NameTests, EqualityObjectChecksStoredLength)
{
    B_Name name("Blade");
    const B_Name prefix("Bl");
    name.String()[2] = '\0';
    EXPECT_EQ(0u, name == prefix);
    EXPECT_EQ(0u, prefix == name);
    EXPECT_EQ(1u, name == "Bl");
}


TEST(NameTests, EqualityStopsAtNullWithEqualStoredLengths)
{
    B_Name name("Blade");
    B_Name other("Blxyz");
    name.String()[2] = '\0';
    other.String()[2] = '\0';
    EXPECT_EQ(1u, name == other);
    EXPECT_EQ(1u, other == name);
    EXPECT_EQ(1u, name == "Bl\0different tail");
}


TEST(NameTests, InequalityEqualNames)
{
    const char *values[] = {"", "a", "ab", "abc", "Blade", "\x80\xFF"};
    for (unsigned int i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    {
        const B_Name name(values[i]);
        const B_Name same(values[i]);
        EXPECT_EQ(0u, name != same);
        EXPECT_EQ(0u, name != values[i]);
        EXPECT_EQ(0u, name != name);
    }
}


TEST(NameTests, InequalityReturnsComparisonSignForEqualLengths)
{
    const B_Name name("abcd");
    const char *others[] = {"xbcd", "axcd", "abxd", "abcx"};
    for (unsigned int i = 0; i < sizeof(others) / sizeof(others[0]); ++i)
    {
        const B_Name other(others[i]);
        EXPECT_EQ(0xFFFFFFFFu, name != other);
        EXPECT_EQ(1u, other != name);
        EXPECT_EQ(0xFFFFFFFFu, name != others[i]);
        EXPECT_EQ(1u, other != "abcd");
    }
}


TEST(NameTests, InequalityIsCaseSensitiveAndUsesUnsignedBytes)
{
    const char *values[] = {"A", "a", "\x7F", "\x80", "\xFF"};
    for (unsigned int i = 0; i + 1 < sizeof(values) / sizeof(values[0]); ++i)
    {
        const B_Name lower(values[i]);
        const B_Name higher(values[i + 1]);
        EXPECT_EQ(0xFFFFFFFFu, lower != higher);
        EXPECT_EQ(1u, higher != lower);
        EXPECT_EQ(0xFFFFFFFFu, lower != values[i + 1]);
        EXPECT_EQ(1u, higher != values[i]);
    }
}


TEST(NameTests, InequalityObjectReturnsOneForDifferentStoredLengths)
{
    const B_Name empty;
    const B_Name name("a");
    const B_Name longer("aa");
    EXPECT_EQ(1u, empty != name);
    EXPECT_EQ(1u, name != empty);
    EXPECT_EQ(1u, name != longer);
    EXPECT_EQ(1u, longer != name);
    EXPECT_EQ(0xFFFFFFFFu, empty != "a");
    EXPECT_EQ(0xFFFFFFFFu, name != "aa");

    B_Name mutated("Blade");
    const B_Name prefix("Bl");
    mutated.String()[2] = '\0';
    EXPECT_EQ(1u, mutated != prefix);
    EXPECT_EQ(1u, prefix != mutated);
    EXPECT_EQ(0u, mutated != "Bl");
}


TEST(NameTests, InequalityStopsAtNullWithEqualStoredLengths)
{
    B_Name name("Blade");
    B_Name other("Blxyz");
    name.String()[2] = '\0';
    other.String()[2] = '\0';
    EXPECT_EQ(0u, name != other);
    EXPECT_EQ(0u, other != name);
    EXPECT_EQ(0u, name != "Bl\0different tail");
}


TEST(NameTests, AssignmentReusesBufferForEqualStoredLengths)
{
    B_Name name("Blade");
    const B_Name source("Sword");
    char *buffer = name.String();
    EXPECT_EQ(&name, &(name = source));
    EXPECT_EQ(buffer, name.String());
    EXPECT_EQ(5u, name.Length());
    EXPECT_STREQ("Sword", name.String());
    EXPECT_STREQ("Sword", source.String());
    EXPECT_NE(source.String(), name.String());
}


TEST(NameTests, AssignmentHandlesGrowingShrinkingAndEmptyText)
{
    B_Name name("a");
    const B_Name longer("Blade");
    const B_Name shorter("Bl");
    const B_Name empty;
    name = longer;
    EXPECT_EQ(5u, name.Length());
    EXPECT_STREQ("Blade", name.String());
    name = shorter;
    EXPECT_EQ(2u, name.Length());
    EXPECT_STREQ("Bl", name.String());
    name = empty;
    EXPECT_EQ(0u, name.Length());
    EXPECT_STREQ("", name.String());
}


TEST(NameTests, AssignmentRecomputesLengthOnlyOnReallocation)
{
    B_Name source("Blade");
    source.String()[2] = '\0';
    B_Name sameLength("Sword");
    B_Name differentLength("a");
    char *buffer = sameLength.String();
    sameLength = source;
    differentLength = source;
    EXPECT_EQ(buffer, sameLength.String());
    EXPECT_EQ(5u, sameLength.Length());
    EXPECT_STREQ("Bl", sameLength.String());
    EXPECT_EQ(2u, differentLength.Length());
    EXPECT_STREQ("Bl", differentLength.String());
    EXPECT_EQ(5u, source.Length());
    EXPECT_STREQ("Bl", source.String());
}


TEST(NameTests, AssignmentPreservesSelfAssignmentAndIndependentOwnership)
{
    B_Name name("Blade");
    name.String()[2] = '\0';
    char *buffer = name.String();
    EXPECT_EQ(&name, &(name = name));
    EXPECT_EQ(buffer, name.String());
    EXPECT_EQ(5u, name.Length());
    EXPECT_STREQ("Bl", name.String());

    B_Name source("\x80\xFF");
    name = source;
    EXPECT_EQ(2u, name.Length());
    EXPECT_STREQ("\x80\xFF", name.String());
    source.String()[0] = 'x';
    EXPECT_STREQ("\x80\xFF", name.String());
    name.String()[1] = 'y';
    EXPECT_STREQ("x\xFF", source.String());
}


TEST(NameTests, AssignmentReturnsDestinationForChaining)
{
    B_Name first("a");
    B_Name second("bb");
    const B_Name source("Blade");
    EXPECT_EQ(&first, &(first = second = source));
    EXPECT_STREQ("Blade", first.String());
    EXPECT_STREQ("Blade", second.String());
    EXPECT_EQ(5u, first.Length());
    EXPECT_EQ(5u, second.Length());
    EXPECT_NE(first.String(), second.String());
}


TEST(NameTests, SlicesSelectPrefixAndSuffix)
{
    const B_Name name("Blade");
    const B_Name left = name.Left(2);
    const B_Name right = name.Right(2);
    EXPECT_EQ(2u, left.Length());
    EXPECT_EQ(2u, right.Length());
    EXPECT_STREQ("Bl", left.String());
    EXPECT_STREQ("de", right.String());
    EXPECT_EQ('\0', left.String()[2]);
    EXPECT_EQ('\0', right.String()[2]);
    EXPECT_STREQ("Blade", name.String());
}


TEST(NameTests, SlicesClampCountToStoredLength)
{
    const B_Name name("Blade");
    const unsigned int counts[] = {5u, 6u, 0xFFFFFFFFu};
    for (unsigned int i = 0; i < sizeof(counts) / sizeof(counts[0]); ++i)
    {
        const B_Name left = name.Left(counts[i]);
        const B_Name right = name.Right(counts[i]);
        EXPECT_EQ(5u, left.Length());
        EXPECT_EQ(5u, right.Length());
        EXPECT_STREQ("Blade", left.String());
        EXPECT_STREQ("Blade", right.String());
    }
}


TEST(NameTests, SlicesHandleZeroCountAndEmptyNames)
{
    const B_Name name("Blade");
    const B_Name empty;
    const B_Name left = name.Left(0);
    const B_Name right = name.Right(0);
    const B_Name emptyLeft = empty.Left(0xFFFFFFFFu);
    const B_Name emptyRight = empty.Right(0xFFFFFFFFu);
    EXPECT_EQ(0u, left.Length());
    EXPECT_EQ(0u, right.Length());
    EXPECT_EQ(0u, emptyLeft.Length());
    EXPECT_EQ(0u, emptyRight.Length());
    EXPECT_STREQ("", left.String());
    EXPECT_STREQ("", right.String());
    EXPECT_STREQ("", emptyLeft.String());
    EXPECT_STREQ("", emptyRight.String());
}


TEST(NameTests, SlicesUseStoredLengthAfterBufferMutation)
{
    B_Name name("Blade");
    name.String()[2] = '\0';
    const B_Name left = name.Left(4);
    const B_Name right = name.Right(2);
    const B_Name rightAcrossNull = name.Right(4);
    EXPECT_EQ(4u, left.Length());
    EXPECT_STREQ("Bl", left.String());
    EXPECT_EQ(2u, right.Length());
    EXPECT_STREQ("de", right.String());
    EXPECT_EQ(4u, rightAcrossNull.Length());
    EXPECT_STREQ("l", rightAcrossNull.String());
}


TEST(NameTests, SlicesOwnIndependentBuffersAndPreserveHighBytes)
{
    B_Name name("\x7F\x80\xFF");
    B_Name left = name.Left(2);
    B_Name right = name.Right(2);
    EXPECT_STREQ("\x7F\x80", left.String());
    EXPECT_STREQ("\x80\xFF", right.String());
    EXPECT_NE(name.String(), left.String());
    EXPECT_NE(name.String(), right.String());
    EXPECT_NE(left.String(), right.String());
    left.String()[1] = 'x';
    right.String()[0] = 'y';
    EXPECT_STREQ("\x7F\x80\xFF", name.String());
    EXPECT_STREQ("\x7F" "x", left.String());
    EXPECT_STREQ("y\xFF", right.String());
}


TEST(NameTests, OrderingEqualNames)
{
    const char *values[] = {"", "a", "ab", "abc", "Blade", "\x80\xFF"};
    for (unsigned int i = 0; i < sizeof(values) / sizeof(values[0]); ++i)
    {
        const B_Name name(values[i]);
        const B_Name same(values[i]);
        EXPECT_EQ(0u, name < same);
        EXPECT_EQ(0u, name > same);
        EXPECT_EQ(0u, name < name);
        EXPECT_EQ(0u, name > name);
    }
}


TEST(NameTests, OrderingEmptyNamesAndPrefixes)
{
    const char *values[] = {"", "a", "ab", "abc", "abcd"};
    for (unsigned int i = 0; i + 1 < sizeof(values) / sizeof(values[0]); ++i)
    {
        const B_Name shorter(values[i]);
        const B_Name longer(values[i + 1]);
        EXPECT_EQ(1u, shorter < longer);
        EXPECT_EQ(0u, shorter > longer);
        EXPECT_EQ(1u, longer > shorter);
        EXPECT_EQ(0u, longer < shorter);
    }
}


TEST(NameTests, OrderingUsesFirstDifferentByte)
{
    const B_Name name("abcd");
    const char *others[] = {"xbcd", "axcd", "abxd", "abcx"};
    for (unsigned int i = 0; i < sizeof(others) / sizeof(others[0]); ++i)
    {
        const B_Name other(others[i]);
        EXPECT_EQ(1u, name < other);
        EXPECT_EQ(0u, name > other);
        EXPECT_EQ(1u, other > name);
        EXPECT_EQ(0u, other < name);
    }
}


TEST(NameTests, OrderingIsCaseSensitiveAndUsesUnsignedBytes)
{
    const char *values[] = {"A", "a", "\x7F", "\x80", "\xFF"};
    for (unsigned int i = 0; i + 1 < sizeof(values) / sizeof(values[0]); ++i)
    {
        const B_Name lower(values[i]);
        const B_Name higher(values[i + 1]);
        EXPECT_EQ(1u, lower < higher);
        EXPECT_EQ(0u, lower > higher);
        EXPECT_EQ(1u, higher > lower);
        EXPECT_EQ(0u, higher < lower);
    }
}


TEST(NameTests, OrderingIgnoresStoredLengthAndStopsAtNull)
{
    B_Name name("Blade");
    const B_Name prefix("Bl");
    name.String()[2] = '\0';
    EXPECT_EQ(0u, name < prefix);
    EXPECT_EQ(0u, name > prefix);
    EXPECT_EQ(0u, prefix < name);
    EXPECT_EQ(0u, prefix > name);

    B_Name other("Blxyz");
    other.String()[2] = '\0';
    EXPECT_EQ(0u, name < other);
    EXPECT_EQ(0u, name > other);

    const B_Name shortName("z");
    const B_Name longName("aa");
    EXPECT_EQ(1u, shortName > longName);
    EXPECT_EQ(1u, longName < shortName);
    EXPECT_EQ(0u, shortName < longName);
    EXPECT_EQ(0u, longName > shortName);
}

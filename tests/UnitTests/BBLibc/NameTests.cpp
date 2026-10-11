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

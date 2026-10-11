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

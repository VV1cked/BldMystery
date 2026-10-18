#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#include <fcntl.h>
#include <cstring>
#define private public
#include <BBLibc/Name.h>
#include <BBLibc/IDataFile.h>
#include <BBLibc/ODataFile.h>
#undef private


static void SetNameInputCache(B_IDataFile &file, const void *data, unsigned int size)
{
    file.file_size = size;
    file.cacheBlockStartPos = 0;
    file.posInCacheBlock = 0;
    memcpy(file.fileCache, data, size);
}


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


TEST(NameTests, SerializationWritesLengthAndBytesWithoutTerminator)
{
    B_ODataFile file("NUL", _O_BINARY);
    ASSERT_NE(0u, file.OK());
    const B_Name name("Blade");
    const unsigned char expected[] = {5, 0, 0, 0, 'B', 'l', 'a', 'd', 'e'};
    EXPECT_EQ(&file, &(file << name));
    ASSERT_EQ(sizeof(expected), file.posInCacheBlock);
    EXPECT_EQ(0, memcmp(expected, file.fileCache, sizeof(expected)));
}


TEST(NameTests, SerializationPreservesBytesAfterEmbeddedNull)
{
    B_ODataFile file("NUL", _O_BINARY);
    ASSERT_NE(0u, file.OK());
    B_Name name("Blade");
    name.String()[2] = '\0';
    const unsigned char expected[] = {5, 0, 0, 0, 'B', 'l', 0, 'd', 'e'};
    file << name;
    ASSERT_EQ(sizeof(expected), file.posInCacheBlock);
    EXPECT_EQ(0, memcmp(expected, file.fileCache, sizeof(expected)));
}


TEST(NameTests, SerializationChainsEmptyAndHighByteNames)
{
    B_ODataFile file("NUL", _O_BINARY);
    ASSERT_NE(0u, file.OK());
    const B_Name empty, bytes("\x80\xFF");
    const unsigned char expected[] = {0, 0, 0, 0, 2, 0, 0, 0, 0x80, 0xFF};
    EXPECT_EQ(&file, &(file << empty << bytes));
    ASSERT_EQ(sizeof(expected), file.posInCacheBlock);
    EXPECT_EQ(0, memcmp(expected, file.fileCache, sizeof(expected)));
}


TEST(NameTests, DeserializationReusesEqualLengthBuffer)
{
    const unsigned char encoded[] = {5, 0, 0, 0, 'S', 'w', 'o', 'r', 'd'};
    B_IDataFile file;
    SetNameInputCache(file, encoded, sizeof(encoded));
    B_Name name("Blade");
    char *buffer = name.String();
    EXPECT_EQ(&file, &(file >> name));
    EXPECT_EQ(buffer, name.String());
    EXPECT_EQ(5u, name.Length());
    EXPECT_STREQ("Sword", name.String());
    EXPECT_EQ(sizeof(encoded), file.posInCacheBlock);
}


TEST(NameTests, DeserializationResizesAndPreservesEmbeddedNull)
{
    const unsigned char encoded[] = {4, 0, 0, 0, 'A', 0, 0x80, 0xFF};
    B_IDataFile file;
    SetNameInputCache(file, encoded, sizeof(encoded));
    B_Name name("Blade");
    file >> name;
    EXPECT_EQ(4u, name.Length());
    EXPECT_EQ(0, memcmp(encoded + 4, name.String(), 4));
    EXPECT_EQ('\0', name.String()[4]);
    EXPECT_STREQ("A", name.String());
}


TEST(NameTests, DeserializationChainsNamesAndHandlesZeroLength)
{
    const unsigned char encoded[] = {2, 0, 0, 0, 'B', 'l', 0, 0, 0, 0};
    B_IDataFile file;
    SetNameInputCache(file, encoded, sizeof(encoded));
    B_Name first, second("Blade");
    EXPECT_EQ(&file, &(file >> first >> second));
    EXPECT_EQ(2u, first.Length());
    EXPECT_STREQ("Bl", first.String());
    EXPECT_EQ(0u, second.Length());
    EXPECT_STREQ("", second.String());
    EXPECT_EQ(sizeof(encoded), file.posInCacheBlock);
}


TEST(NameTests, DeserializationShortReadKeepsExistingBufferTail)
{
    const unsigned char encoded[] = {5, 0, 0, 0, 'X', 'Y'};
    B_IDataFile file;
    SetNameInputCache(file, encoded, sizeof(encoded));
    B_Name name("Blade");
    char *buffer = name.String();
    file >> name;
    EXPECT_EQ(buffer, name.String());
    EXPECT_EQ(5u, name.Length());
    EXPECT_STREQ("XYade", name.String());
    EXPECT_EQ(sizeof(encoded), file.posInCacheBlock);
}


TEST(NameTests, SerializationAndDeserializationRoundTripRawBuffer)
{
    B_ODataFile output("NUL", _O_BINARY);
    ASSERT_NE(0u, output.OK());
    B_Name source("Blade");
    source.String()[2] = '\0';
    output << source;
    B_IDataFile input;
    SetNameInputCache(input, output.fileCache, output.posInCacheBlock);
    B_Name result;
    input >> result;
    EXPECT_EQ(source.Length(), result.Length());
    EXPECT_EQ(0, memcmp(source.String(), result.String(), source.Length()));
    EXPECT_EQ('\0', result.String()[result.Length()]);
    EXPECT_NE(source.String(), result.String());
}


TEST(NameTests, AppendReturnsDestinationAndConcatenatesText)
{
    B_Name name("Bl");
    const B_Name suffix("ade");
    EXPECT_EQ(&name, &(name += suffix));
    EXPECT_STREQ("Blade", name.String());
    EXPECT_EQ(5u, name.Length());
    EXPECT_EQ(&name, &(name += "!"));
    EXPECT_STREQ("Blade!", name.String());
    EXPECT_EQ(6u, name.Length());
    EXPECT_EQ(&name, &(name += '?'));
    EXPECT_STREQ("Blade!?", name.String());
    EXPECT_EQ(7u, name.Length());
    EXPECT_STREQ("ade", suffix.String());
}


TEST(NameTests, AppendEmptyTextStillReallocates)
{
    B_Name name("Blade");
    const B_Name empty;
    char *buffer = name.String();
    name += empty;
    EXPECT_NE(buffer, name.String());
    EXPECT_EQ(5u, name.Length());
    EXPECT_STREQ("Blade", name.String());
    buffer = name.String();
    name += "";
    EXPECT_NE(buffer, name.String());
    EXPECT_EQ(5u, name.Length());
    EXPECT_STREQ("Blade", name.String());
    B_Name emptyName;
    emptyName += "";
    EXPECT_EQ(0u, emptyName.Length());
    EXPECT_STREQ("", emptyName.String());
    emptyName += 'x';
    EXPECT_EQ(1u, emptyName.Length());
    EXPECT_STREQ("x", emptyName.String());
}


TEST(NameTests, AppendUsesStoredLengthsButCopiesTextUpToNull)
{
    B_Name source("Sword");
    source.String()[1] = '\0';
    B_Name objectTarget("Blade"), stringTarget("Blade");
    objectTarget.String()[2] = '\0';
    stringTarget.String()[2] = '\0';
    objectTarget += source;
    stringTarget += source.String();
    EXPECT_STREQ("BlS", objectTarget.String());
    EXPECT_STREQ("BlS", stringTarget.String());
    EXPECT_EQ(10u, objectTarget.Length());
    EXPECT_EQ(6u, stringTarget.Length());
    EXPECT_EQ(5u, source.Length());
    EXPECT_STREQ("S", source.String());
}


TEST(NameTests, AppendCharacterUsesStoredEndAfterEarlyNull)
{
    B_Name name("Blade");
    name.String()[2] = '\0';
    name += 'x';
    EXPECT_EQ(6u, name.Length());
    EXPECT_STREQ("Bl", name.String());
    EXPECT_EQ('x', name.String()[5]);
    EXPECT_EQ('\0', name.String()[6]);
    // The gap after the early NUL is uninitialized in the original allocation.
}


TEST(NameTests, AppendNullCharacterStillIncreasesStoredLength)
{
    B_Name name("Blade");
    char *buffer = name.String();
    name += '\0';
    EXPECT_NE(buffer, name.String());
    EXPECT_EQ(6u, name.Length());
    EXPECT_STREQ("Blade", name.String());
    EXPECT_EQ('\0', name.String()[5]);
    EXPECT_EQ('\0', name.String()[6]);
}


TEST(NameTests, AppendSupportsSelfAndAliasedStringBuffers)
{
    B_Name objectTarget("Blade"), stringTarget("Blade"), suffixTarget("Blade");
    objectTarget += objectTarget;
    stringTarget += stringTarget.String();
    suffixTarget += suffixTarget.String() + 2;
    EXPECT_STREQ("BladeBlade", objectTarget.String());
    EXPECT_STREQ("BladeBlade", stringTarget.String());
    EXPECT_STREQ("Bladeade", suffixTarget.String());
    EXPECT_EQ(10u, objectTarget.Length());
    EXPECT_EQ(10u, stringTarget.Length());
    EXPECT_EQ(8u, suffixTarget.Length());
}


TEST(NameTests, AppendPreservesHighBytesAndIndependentOwnership)
{
    B_Name name("\x80");
    B_Name source("\xFF");
    name += source;
    name += "\x7F";
    name += static_cast<char>(0x81);
    EXPECT_EQ(4u, name.Length());
    EXPECT_STREQ("\x80\xFF\x7F\x81", name.String());
    source.String()[0] = 'x';
    EXPECT_STREQ("\x80\xFF\x7F\x81", name.String());
}


TEST(NameTests, ConcatenationResolvesForMutableNamesAndTemporaries)
{
    B_Name left("Bl"), right("ade");
    const B_Name names = left + right;
    const B_Name suffix = left + "ade";
    const B_Name prefix = "Bl" + right;
    const B_Name temporaries = B_Name("Bl") + B_Name("ade");
    EXPECT_STREQ("Blade", names.String());
    EXPECT_STREQ("Blade", suffix.String());
    EXPECT_STREQ("Blade", prefix.String());
    EXPECT_STREQ("Blade", temporaries.String());
    EXPECT_EQ(6u, names.Length());
    EXPECT_EQ(6u, suffix.Length());
    EXPECT_EQ(6u, prefix.Length());
    EXPECT_EQ(6u, temporaries.Length());
    EXPECT_STREQ("Bl", left.String());
    EXPECT_STREQ("ade", right.String());
}


TEST(NameTests, ConcatenationPreservesExtraStoredLength)
{
    const B_Name left("Bl"), right("ade");
    const B_Name names = left + right;
    const B_Name suffix = left + "ade";
    const B_Name prefix = "Bl" + right;
    EXPECT_STREQ("Blade", names.String());
    EXPECT_STREQ("Blade", suffix.String());
    EXPECT_STREQ("Blade", prefix.String());
    EXPECT_EQ(6u, names.Length());
    EXPECT_EQ(6u, suffix.Length());
    EXPECT_EQ(6u, prefix.Length());
    // Object equality checks stored length despite identical text.
    EXPECT_EQ(0u, names == B_Name("Blade"));
    EXPECT_EQ(1u, names == "Blade");
}


TEST(NameTests, ConcatenationEmptyNamesStillAddExtraStoredByte)
{
    const B_Name empty;
    const B_Name names = empty + empty;
    const B_Name suffix = empty + "";
    const B_Name prefix = "" + empty;
    EXPECT_STREQ("", names.String());
    EXPECT_STREQ("", suffix.String());
    EXPECT_STREQ("", prefix.String());
    EXPECT_EQ(1u, names.Length());
    EXPECT_EQ(1u, suffix.Length());
    EXPECT_EQ(1u, prefix.Length());
    const B_Name name("a");
    EXPECT_EQ(2u, (name + empty).Length());
    EXPECT_EQ(2u, (empty + name).Length());
    EXPECT_STREQ("a", (name + empty).String());
    EXPECT_STREQ("a", (empty + name).String());
}


TEST(NameTests, ConcatenationUsesStoredLengthsButStopsCopyingAtNull)
{
    B_Name left("Blade"), right("Sword");
    left.String()[2] = '\0';
    right.String()[1] = '\0';
    const B_Name names = ::operator +(left, right);
    const B_Name suffix = ::operator +(left, "S\0ignored");
    const B_Name prefix = ::operator +("Bl\0ignored", right);
    EXPECT_STREQ("BlS", names.String());
    EXPECT_STREQ("BlS", suffix.String());
    EXPECT_STREQ("BlS", prefix.String());
    EXPECT_EQ(11u, names.Length());
    EXPECT_EQ(7u, suffix.Length());
    EXPECT_EQ(8u, prefix.Length());
}


TEST(NameTests, ConcatenationOwnsBufferAndSupportsRepeatedOperands)
{
    B_Name name("\x80\xFF");
    B_Name doubled = ::operator +(name, name);
    B_Name suffix = ::operator +(name, name.String());
    B_Name prefix = ::operator +(name.String(), name);
    EXPECT_STREQ("\x80\xFF\x80\xFF", doubled.String());
    EXPECT_STREQ(doubled.String(), suffix.String());
    EXPECT_STREQ(doubled.String(), prefix.String());
    EXPECT_EQ(5u, doubled.Length());
    EXPECT_EQ(5u, suffix.Length());
    EXPECT_EQ(5u, prefix.Length());
    EXPECT_NE(name.String(), doubled.String());
    doubled.String()[0] = 'x';
    EXPECT_STREQ("\x80\xFF", name.String());
    name.String()[0] = 'y';
    EXPECT_STREQ("\x80\xFF\x80\xFF", suffix.String());
    EXPECT_STREQ("\x80\xFF\x80\xFF", prefix.String());
}


TEST(NameTests, ConcatenationChainsAccumulateExtraStoredBytes)
{
    const B_Name result = ::operator +(::operator +(B_Name("a"), B_Name("b")), B_Name("c"));
    EXPECT_STREQ("abc", result.String());
    EXPECT_EQ(5u, result.Length());
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

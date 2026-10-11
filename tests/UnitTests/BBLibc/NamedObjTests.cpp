#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#include <fcntl.h>
#include <cstring>
#define private public
#include <BBLibc/NamedObj.h>
#include <BBLibc/IDataFile.h>
#include <BBLibc/ODataFile.h>
#undef private


static void SetNamedObjInputCache(B_IDataFile &file, const void *data, unsigned int size)
{
    file.file_size = size;
    file.cacheBlockStartPos = 0;
    file.posInCacheBlock = 0;
    memcpy(file.fileCache, data, size);
}


TEST(NamedObjTests, SizeOf)
{
    EXPECT_EQ(sizeof(B_NamedObj), 0x000C);
}

TEST(NamedObjTests, Fields)
{
    EXPECT_OFFSET_EQ(B_NamedObj, name, 0x0004);
}


TEST(NamedObjTests, SerializationWritesOnlyNameIncludingEmbeddedNull)
{
    B_ODataFile output("NUL", _O_BINARY);
    ASSERT_NE(0u, output.OK());
    B_NamedObj object(B_Name("Blade"));
    object.name.String()[2] = '\0';
    const unsigned char expected[] = {5, 0, 0, 0, 'B', 'l', 0, 'd', 'e'};
    EXPECT_EQ(&output, &(output << object));
    ASSERT_EQ(sizeof(expected), output.posInCacheBlock);
    EXPECT_EQ(0, memcmp(expected, output.fileCache, sizeof(expected)));
}


TEST(NamedObjTests, DeserializationReusesNameAndPreservesDerivedObject)
{
    struct TaggedNamedObj : B_NamedObj {
        TaggedNamedObj() : B_NamedObj(B_Name("Blade")), tag(0x12345678u) {}
        unsigned int tag;
    } object;
    unsigned char vtable[sizeof(void *)];
    memcpy(vtable, &object, sizeof(vtable));
    char *buffer = object.name.String();
    const B_Name *identity = &object.Id();
    const unsigned char encoded[] = {5, 0, 0, 0, 'S', 'w', 'o', 'r', 'd'};
    B_IDataFile input;
    SetNamedObjInputCache(input, encoded, sizeof(encoded));
    EXPECT_EQ(&input, &(input >> object));
    EXPECT_EQ(buffer, object.name.String());
    EXPECT_EQ(identity, &object.Id());
    EXPECT_STREQ("Sword", object.Id().String());
    EXPECT_EQ(5u, object.Id().Length());
    EXPECT_EQ(0x12345678u, object.tag);
    EXPECT_EQ(0, memcmp(vtable, &object, sizeof(vtable)));
}


TEST(NamedObjTests, MixedNameAndObjectStreamsShareFormat)
{
    B_ODataFile output("NUL", _O_BINARY);
    ASSERT_NE(0u, output.OK());
    const B_Name name("a");
    const B_NamedObj object(B_Name("BC")), empty;
    const unsigned char expected[] = {1, 0, 0, 0, 'a', 2, 0, 0, 0, 'B', 'C', 0, 0, 0, 0};
    EXPECT_EQ(&output, &(output << name << object << empty));
    ASSERT_EQ(sizeof(expected), output.posInCacheBlock);
    EXPECT_EQ(0, memcmp(expected, output.fileCache, sizeof(expected)));
    B_IDataFile input;
    SetNamedObjInputCache(input, expected, sizeof(expected));
    B_Name first;
    B_NamedObj second, third(B_Name("Blade"));
    EXPECT_EQ(&input, &(input >> first >> second >> third));
    EXPECT_STREQ("a", first.String());
    EXPECT_STREQ("BC", second.Id().String());
    EXPECT_EQ(0u, third.Id().Length());
    EXPECT_STREQ("", third.Id().String());
    EXPECT_EQ(sizeof(expected), input.posInCacheBlock);
}

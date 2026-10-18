#include <gtest/gtest.h>
#include <Utils/FieldOffset.h>
#include <bld_system.h>
#include <BBLibc/Resource.h>
#include <cstring>
#include <new>


TEST(ResourceTests, Fields)
{
    EXPECT_OFFSET_EQ(B_Resource, data, 0x00C);
}

TEST(ResourceTests, DefaultConstructorLeavesDataUntouched)
{
    alignas(B_Resource) unsigned char storage[sizeof(B_Resource)];
    std::memset(storage, 0xA5, sizeof(storage));
    B_Resource *resource = new (storage) B_Resource;
    EXPECT_EQ(0u, resource->Id().Length());
    EXPECT_STREQ("", resource->Id().String());
    for (unsigned int i = 0; i < sizeof(resource->data); ++i)
        EXPECT_EQ(0xA5, storage[0xC + i]);
    resource->~B_Resource();
    for (unsigned int i = 0; i < sizeof(resource->data); ++i)
        EXPECT_EQ(0xA5, storage[0xC + i]);
}

TEST(ResourceTests, NamedConstructorCopiesNameAndLeavesDataUntouched)
{
    B_Name name("blade");
    name.String()[2] = '\0';
    alignas(B_Resource) unsigned char storage[sizeof(B_Resource)];
    std::memset(storage, 0x5A, sizeof(storage));
    B_Resource *resource = new (storage) B_Resource(name);
    EXPECT_EQ(5u, resource->Id().Length());
    EXPECT_STREQ("bl", resource->Id().String());
    EXPECT_NE(name.String(), resource->Id().String());
    name.String()[0] = 'x';
    EXPECT_STREQ("bl", resource->Id().String());
    for (unsigned int i = 0; i < sizeof(resource->data); ++i)
        EXPECT_EQ(0x5A, storage[0xC + i]);
    resource->~B_Resource();
}

TEST(ResourceTests, DestructorDoesNotOwnData)
{
    unsigned char payload[] = {1, 2, 3, 4};
    alignas(B_Resource) unsigned char storage[sizeof(B_Resource)];
    B_Resource *resource = new (storage) B_Resource(B_Name("resource"));
    resource->data = payload;
    resource->~B_Resource();
    void *retainedData = nullptr;
    std::memcpy(&retainedData, storage + 0xC, sizeof(retainedData));
    EXPECT_EQ(payload, retainedData);
    EXPECT_EQ(1, payload[0]);
    EXPECT_EQ(4, payload[3]);
}

TEST(ResourceTests, DestructionThroughNamedObjectIsVirtual)
{
    struct DerivedResource : B_Resource
    {
        explicit DerivedResource(bool &destroyed) : destroyed(destroyed) {}
        ~DerivedResource() override { destroyed = true; }
        bool &destroyed;
    };
    bool destroyed = false;
    B_NamedObj *resource = new DerivedResource(destroyed);
    delete resource;
    EXPECT_TRUE(destroyed);
}

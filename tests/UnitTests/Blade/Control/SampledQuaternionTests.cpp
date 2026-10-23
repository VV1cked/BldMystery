#include <gtest/gtest.h>
#include <Control/SampledQuaternion.h>
#include <cstddef>
#include <type_traits>

TEST(SampledQuaternion, NativeBorrowedLayouts)
{
    EXPECT_EQ(8u, sizeof(B_SampledRootAdapterView));
    EXPECT_EQ(4u, alignof(B_SampledRootAdapterView));
    EXPECT_EQ(0x10u, offsetof(B_SampledRootDataView, nodes));
    EXPECT_EQ(0x14u, sizeof(B_SampledQuaternionNodeView));
    EXPECT_EQ(4u, offsetof(B_SampledQuaternionNodeView, keys));
    EXPECT_EQ(8u, offsetof(B_SampledQuaternionNodeView, keyCount));
    EXPECT_FALSE(std::is_polymorphic<B_SampledRootAdapterView>::value);
    EXPECT_TRUE(std::is_trivially_destructible<B_SampledRootAdapterView>::value);
}

TEST(SampledQuaternion, EndpointUsesNodeCountWithoutNormalization)
{
    const short keys[3][4]={{30000,0,0,0},{0,30000,0,0},{0,0,0,15000}};
    B_SampledQuaternionNodeView node={};node.keys=keys;node.keyCount=3;
    B_SampledRootDataView sample={};sample.nodes=&node;sample.rootCount=2;
    B_SampledRootAdapterView adapter={0,&sample};float out[4];
    adapter.SampleQuaternion(1.0,0,out,0);
    EXPECT_FLOAT_EQ(0.0f,out[0]);EXPECT_FLOAT_EQ(0.0f,out[1]);
    EXPECT_FLOAT_EQ(0.0f,out[2]);EXPECT_FLOAT_EQ(0.5f,out[3]);
}

TEST(SampledQuaternion, InteriorUsesRootCountAndNormalizes)
{
    const short keys[3][4]={{15000,0,0,0},{15000,0,0,0},{0,15000,0,0}};
    B_SampledQuaternionNodeView node={};node.keys=keys;node.keyCount=3;
    B_SampledRootDataView sample={};sample.nodes=&node;sample.rootCount=2;
    B_SampledRootAdapterView adapter={0,&sample};
    struct {unsigned guard;float out[4];unsigned end;} result={0x12345678,{},0x87654321};
    adapter.SampleQuaternion(0.75,0,result.out,0);
    EXPECT_FLOAT_EQ(1.0f,result.out[0]);EXPECT_FLOAT_EQ(0.0f,result.out[1]);
    EXPECT_FLOAT_EQ(0.0f,result.out[2]);EXPECT_FLOAT_EQ(0.0f,result.out[3]);
    EXPECT_EQ(0x12345678u,result.guard);EXPECT_EQ(0x87654321u,result.end);
}

TEST(SampledQuaternion, DecoderPreservesSignsAndDoesNotNormalize)
{
    const short key[4]={-15000,0,15000,30000};float out[4];
    B_DecodeSampledQuaternion(out,key);
    EXPECT_FLOAT_EQ(-0.5f,out[0]);EXPECT_FLOAT_EQ(0.0f,out[1]);
    EXPECT_FLOAT_EQ(0.5f,out[2]);EXPECT_FLOAT_EQ(1.0f,out[3]);
}

#include <gtest/gtest.h>
#include <Person/PoseCacheView.h>
#include <cstring>

namespace {
struct RootFixture {
    unsigned char bytes[0x730];
    RootFixture() {
        std::memset(bytes, 0, sizeof(bytes));
        double identity[16]={1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1};
        std::memcpy(bytes+0x6b0,identity,128);
    }
    double &value(unsigned offset) { return *reinterpret_cast<double *>(bytes+offset); }
    B_PoseCacheView *view() { return reinterpret_cast<B_PoseCacheView *>(bytes); }
};
}

// Host-only POD tests complement the independent original oracle. They do
// not create an engine animator or claim its complete layout/lifetime.
TEST(PoseRootCache, CacheHitLeavesAllBorrowedStorageUntouched)
{
    RootFixture f;
    f.value(0x88)=f.value(0x128)=0.25;
    std::memset(f.bytes+0xa8,0x5a,128);
    unsigned char before[sizeof(f.bytes)]; std::memcpy(before,f.bytes,sizeof(before));
    EXPECT_EQ(f.bytes+0xa8,reinterpret_cast<unsigned char *>(f.view()->RootMatrix()));
    EXPECT_EQ(0,std::memcmp(before,f.bytes,sizeof(before)));
}

TEST(PoseRootCache, EqualNegativeSentinelsStillForceRebuild)
{
    RootFixture f;
    f.value(0x88)=f.value(0x128)=-1;
    f.value(0x90)=13; f.value(0x98)=-5; f.value(0xa0)=7;
    f.view()->RootMatrix();
    EXPECT_EQ(13,f.value(0xa8+96));
    f.value(0x90)=27;
    f.view()->RootMatrix();
    EXPECT_EQ(27,f.value(0xa8+96));
    EXPECT_EQ(-1,f.value(0x128));
}

TEST(PoseRootCache, CorrectionTimesBaseKeepsOrderAndReplacesTranslation)
{
    RootFixture f;
    f.value(0x88)=0.5; f.value(0x128)=-1;
    *reinterpret_cast<unsigned *>(f.bytes+0x130)=2;
    double correction[16]={2,0,0,0,0,3,0,0,0,0,4,0,99,98,97,1};
    std::memcpy(f.bytes+0x570,correction,128);
    f.value(0x90)=5; f.value(0x98)=6; f.value(0xa0)=7;
    f.value(0x6b0+96)=11; f.value(0x6b0+104)=12; f.value(0x6b0+112)=13;
    f.view()->RootMatrix();
    EXPECT_EQ(16,f.value(0xa8+96));
    EXPECT_EQ(18,f.value(0xa8+104));
    EXPECT_EQ(20,f.value(0xa8+112));
    EXPECT_EQ(2,f.value(0xa8));
    EXPECT_EQ(0,std::memcmp(f.bytes+0x570,correction,128));
    EXPECT_EQ(0.5,f.value(0x128));
}

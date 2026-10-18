#include <gtest/gtest.h>
#include <cstring>
#include <new>
#include <Utils/FieldOffset.h>
#define private public
#include <BBLibc/RDataLight.h>
#undef private

TEST(RDataLightTests, Layout)
{
    EXPECT_EQ(16u, sizeof(B_RDataLight));
    EXPECT_OFFSET_EQ(B_RDataLight, unknown00, 0);
    EXPECT_OFFSET_EQ(B_RDataLight, unknown04, 4);
    EXPECT_OFFSET_EQ(B_RDataLight, unknown08, 8);
    EXPECT_OFFSET_EQ(B_RDataLight, unknown0C, 12);
}

TEST(RDataLightTests, BothConstructorsLeaveStorageUntouched)
{
    alignas(B_RDataLight) unsigned char storage[sizeof(B_RDataLight)];
    unsigned char expected[sizeof(B_RDataLight)];
    std::memset(storage, 0xA5, sizeof(storage));
    std::memcpy(expected, storage, sizeof(storage));
    B_RDataLight *object = new (storage) B_RDataLight();
    EXPECT_EQ(0, std::memcmp(storage, expected, sizeof(storage)));
    object->~B_RDataLight();
    object = new (storage) B_RDataLight(1.0F, 2.0F, 3.0F, 4.0F);
    EXPECT_EQ(0, std::memcmp(storage, expected, sizeof(storage)));
    object->~B_RDataLight();
}

TEST(RDataLightTests, AssignmentPreservesBitPatternsAndReturnsDestination)
{
    const unsigned int bits[4] = {0x80000000u, 0x7F800001u, 0x7FC12345u, 1u};
    B_RDataLight source, destination;
    std::memcpy(&source, bits, sizeof(bits));
    EXPECT_EQ(&destination, &(destination = source));
    EXPECT_EQ(0, std::memcmp(&destination, bits, sizeof(bits)));
    EXPECT_EQ(0, std::memcmp(&source, bits, sizeof(bits)));
    EXPECT_EQ(&destination, &(destination = destination));
    EXPECT_EQ(0, std::memcmp(&destination, bits, sizeof(bits)));
}

TEST(RDataLightTests, InterpolationUsesIndependentWeights)
{
    const float a[4] = {1, 2, 4, 8}, b[4] = {8, 4, 2, 1};
    const float expected[4] = {-2, 2, 7, 15.5F};
    B_RDataLight start, end, result;
    std::memcpy(&start, a, sizeof(a));
    std::memcpy(&end, b, sizeof(b));
    result.Interp(2.0, start, -0.5, end);
    EXPECT_EQ(0, std::memcmp(&result, expected, sizeof(expected)));
    EXPECT_EQ(0, std::memcmp(&start, a, sizeof(a)));
    EXPECT_EQ(0, std::memcmp(&end, b, sizeof(b)));
}

TEST(RDataLightTests, InterpolationSupportsInputAliasing)
{
    const float a[4] = {1, 2, 4, 8}, b[4] = {8, 4, 2, 1};
    const float expected[4] = {4.5F, 3, 3, 4.5F};
    B_RDataLight start, end;
    std::memcpy(&start, a, sizeof(a));
    std::memcpy(&end, b, sizeof(b));
    start.Interp(0.5, start, 0.5, end);
    EXPECT_EQ(0, std::memcmp(&start, expected, sizeof(expected)));
    std::memcpy(&start, a, sizeof(a));
    end.Interp(0.5, start, 0.5, end);
    EXPECT_EQ(0, std::memcmp(&end, expected, sizeof(expected)));
    start.Interp(0.25, start, 0.75, start);
    EXPECT_EQ(0, std::memcmp(&start, a, sizeof(a)));
}

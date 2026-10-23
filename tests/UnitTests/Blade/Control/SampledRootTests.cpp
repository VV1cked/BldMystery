#include <gtest/gtest.h>
#include <Control/SampledRoot.h>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

TEST(SampledRootTests, NonOwningX86Prefixes)
{
    static_assert(sizeof(void *) == 4, "This fixture validates the x86 ABI");
    static_assert(sizeof(B_SampledRootAdapterView) == 8, "adapter prefix");
    static_assert(offsetof(B_SampledRootAdapterView, originalVtable) == 0, "explicit vptr");
    static_assert(offsetof(B_SampledRootAdapterView, sample) == 4, "sample");
    static_assert(offsetof(B_SampledRootDataView, roots) == 0x34, "roots");
    static_assert(offsetof(B_SampledRootDataView, rootCount) == 0x38, "count");
    static_assert(sizeof(B_SampledRootDataView) == 0x3C, "storage prefix only");
    static_assert(alignof(B_SampledRootAdapterView) == 4, "adapter alignment");
    static_assert(!std::is_polymorphic<B_SampledRootAdapterView>::value, "no C++ vtable");
    static_assert(std::is_trivially_destructible<B_SampledRootAdapterView>::value, "no ownership");
    SUCCEED();
}

TEST(SampledRootTests, WritesOnlyThreeDoublesAndPreservesInputs)
{
    const double keys[2][3] = {{2, -4, 8}, {10, 12, -8}};
    B_SampledRootDataView data = {};
    data.roots = keys; data.rootCount = 2;
    B_SampledRootAdapterView view = {reinterpret_cast<void *>(0x1234), &data};
    const B_SampledRootAdapterView oldView = view;
    const B_SampledRootDataView oldData = data;
    double guarded[5] = {12345, 0, 0, 0, -67890};
    view.SampleRoot(.25, guarded + 1, reinterpret_cast<void *>(0x5678));
    EXPECT_EQ(12345, guarded[0]); EXPECT_EQ(-67890, guarded[4]);
    EXPECT_EQ(4, guarded[1]); EXPECT_EQ(0, guarded[2]); EXPECT_EQ(4, guarded[3]);
    EXPECT_EQ(0, std::memcmp(&view, &oldView, sizeof(view)));
    EXPECT_EQ(0, std::memcmp(&data, &oldData, sizeof(data)));
}

TEST(SampledRootTests, RetainsStoredFloatWeightPrecision)
{
    const double keys[2][3] = {{0, 0, 0}, {1, 1, 1}};
    B_SampledRootDataView data = {};
    data.roots = keys; data.rootCount = 2;
    B_SampledRootAdapterView view = {NULL, &data};
    double out[3];
    view.SampleRoot(.1, out, NULL);
    // Exact binary rational, distinct from a double-precision 0.1 lerp.
    const double expected = 1677722.0 / 16777216.0;
    for (unsigned i = 0; i < 3; ++i) EXPECT_DOUBLE_EQ(expected, out[i]);
}

TEST(SampledRootTests, EndpointCopiesBitsWithoutFloatingConversion)
{
    double keys[2][3] = {};
    const uint64_t last[3] = {0x8000000000000000ULL,
        0x7FF8000000001234ULL, 0x0000000000000001ULL};
    std::memcpy(keys[1], last, sizeof(last));
    B_SampledRootDataView data = {};
    data.roots = keys; data.rootCount = 2;
    B_SampledRootAdapterView view = {NULL, &data};
    double out[3];
    view.SampleRoot(1.0, out, NULL);
    EXPECT_EQ(0, std::memcmp(out, last, sizeof(last)));
}

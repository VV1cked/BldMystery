#include <gtest/gtest.h>
#include <AnalyticGeometry/Matrix.h>
#include <AnalyticGeometry/Vector.h>
#include <cstring>

// CODEX-RECON-008: focused regressions complement the original-executed
// differential. No layout change or general-inverse behavior is intended.
TEST(MatrixCore, NativeValueSizes)
{
    EXPECT_EQ(0x80u, sizeof(B_Matrix));
    EXPECT_EQ(0x18u, sizeof(B_Vector));
}

TEST(MatrixCore, TranslationLeavesOtherBytesAndRetainsOverlapOrder)
{
    B_Matrix matrix;
    double values[16];
    for (int i = 0; i < 16; ++i) values[i] = double(i);
    std::memcpy(&matrix, values, sizeof(values));
    const B_Vector &overlap = *reinterpret_cast<const B_Vector *>(
        reinterpret_cast<const unsigned char *>(&matrix) + 88);
    matrix.SetTranslation(overlap);
    double actual[16]; std::memcpy(actual, &matrix, sizeof(actual));
    EXPECT_EQ(0, std::memcmp(values, actual, 96));
    EXPECT_EQ(11., actual[12]); EXPECT_EQ(11., actual[13]);
    EXPECT_EQ(11., actual[14]); EXPECT_EQ(15., actual[15]);
}

TEST(MatrixCore, MultiplyWritesFullMatrixAndPreservesSeparateInputs)
{
    B_Matrix a, identity, result;
    double values[16];
    for (int i = 0; i < 16; ++i) values[i] = double(i - 7);
    std::memcpy(&a, values, sizeof(values));
    B_MultiplyMatrices(result, a, identity);
    EXPECT_EQ(0, std::memcmp(&result, values, sizeof(values)));
    EXPECT_EQ(0, std::memcmp(&a, values, sizeof(values)));
}

TEST(MatrixCore, RotateIgnoresTranslation)
{
    B_Matrix matrix(B_Vector(100., -200., 300.));
    B_Vector result = Rotate(B_Vector(1., -2., 3.), matrix);
    EXPECT_EQ(1., result.x); EXPECT_EQ(-2., result.y); EXPECT_EQ(3., result.z);
}

TEST(MatrixCore, RigidInversePreservesOutputFourthColumn)
{
    B_Matrix source(B_Vector(10., -20., 30.)), out;
    double fill[16]; for (int i = 0; i < 16; ++i) fill[i] = 42.;
    std::memcpy(&out, fill, sizeof(fill));
    source.RigidInverse(out);
    double result[16]; std::memcpy(result, &out, sizeof(result));
    for (int i = 3; i < 16; i += 4) EXPECT_EQ(42., result[i]);
    EXPECT_EQ(-10., result[12]); EXPECT_EQ(20., result[13]); EXPECT_EQ(-30., result[14]);
}

TEST(MatrixCore, NegationPreservesSignedZero)
{
    B_Vector result = B_Vector(0., -0., 1.).Negated();
    unsigned char bits[24]; std::memcpy(bits, &result, sizeof(bits));
    EXPECT_EQ(0x80u, bits[7]); EXPECT_EQ(0u, bits[15]); EXPECT_EQ(-1., result.z);
}

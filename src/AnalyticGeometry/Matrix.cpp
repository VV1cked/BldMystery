#include <AnalyticGeometry/Matrix.h>

#include <bld_system.h>
#include <AnalyticGeometry/Vector.h>

namespace {
// Keep the original load/multiply/add sequence and the single double store.
// In particular, no intermediate product is rounded through a C++ temporary.
void MatrixDot(double *result, const double *row, const double *column, int terms)
{
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        mov eax, row
        mov edx, column
        fld qword ptr [eax]
        fmul qword ptr [edx]
        fld qword ptr [eax+8]
        fmul qword ptr [edx+32]
        faddp st(1), st(0)
        fld qword ptr [eax+16]
        fmul qword ptr [edx+64]
        faddp st(1), st(0)
        cmp terms, 4
        jne storeResult
        fld qword ptr [eax+24]
        fmul qword ptr [edx+96]
        faddp st(1), st(0)
    storeResult:
        mov eax, result
        fstp qword ptr [eax]
    }
#else
    // Portable builds have no claim of original x87 precision/status parity.
    volatile double sum = row[0] * column[0];
    sum = sum + row[1] * column[4];
    sum = sum + row[2] * column[8];
    if (terms == 4) sum = sum + row[3] * column[12];
    *result = sum;
#endif
}

void CopyMatrixWordPair(double *out, const double *in)
{
    // Volatile accesses retain the original low-DWORD then high-DWORD order,
    // including the observable propagation when source and output overlap.
    volatile unsigned int *dst = reinterpret_cast<volatile unsigned int *>(out);
    const volatile unsigned int *src = reinterpret_cast<const volatile unsigned int *>(in);
    dst[0] = src[0];
    dst[1] = src[1];
}
}

#define DEFINE_MATRIX(\
    m11, m12, m13, m14,\
    m21, m22, m23, m24,\
    m31, m32, m33, m34,\
    m41, m42, m43, m44\
)\
    m[0][0] = m11; m[0][1] = m12; m[0][2] = m13; m[0][3] = m14;\
    m[1][0] = m21; m[1][1] = m22; m[1][2] = m23; m[1][3] = m24;\
    m[2][0] = m31; m[2][1] = m32; m[2][2] = m33; m[2][3] = m34;\
    m[3][0] = m41; m[3][1] = m42; m[3][2] = m43; m[3][3] = m44;\


/*
* Module:                 rOpenGL.dll
* Entry point:            0x10003430
* Module:                 Blade.exe
* Entry point:            0x00404BE0
* VC++ mangling:          ??0B_Matrix@@QAE@XZ
*/

B_Matrix::B_Matrix()
{
    DEFINE_MATRIX(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        0.0, 0.0, 0.0, 1.0
    )
}


/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 rOpenGL.dll
* Entry point:            0x1000377F
* Module:                 Blade.exe
* Entry point:            0x00404F2F
* VC++ mangling:          ??0B_Matrix@@QAE@ABVB_Vector@@@Z
*/

B_Matrix::B_Matrix(const B_Vector &v)
{
    DEFINE_MATRIX(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        v.x, v.y, v.z, 1.0
    )
}


/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 rOpenGL.dll
* Entry point:            0x10003F6F
* Module:                 Blade.exe
* Entry point:            0x0040571F
* VC++ mangling:          ?TranslationVector@B_Matrix@@QBE?AVB_Vector@@XZ
*/

B_Vector B_Matrix::TranslationVector() const
{
    return B_Vector(this->m[3][0], this->m[3][1], this->m[3][2]);
}


/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x406903
* VC++ mangling:          ??D@YA?AVB_Vector@@ABV0@ABVB_Matrix@@@Z
*/
#ifndef BLD_NATIVE
B_Vector operator *(const B_Vector& vector, const B_Matrix &matrix)
{
    return vector;
}
#endif

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x00406AF1
* VC++ mangling:          ?Rotate@@YA?AVB_Vector@@ABV1@ABVB_Matrix@@@Z
* Description: Row vector times the 3x3 block; translation is ignored. Compute
* z, y, x before writing the return buffer, preserving overlapping inputs.
* Origin: PROTOCOL_CPP, CODEX-RECON-008; inherited non-native stub replaced.
* Validation: DIFFERENTIAL_IN_SCOPE; original repeat and x86 /Od,/O2;
* 256 cluster cases, exact buffer bytes/x87 status; five sequential native
* switches with retained ASM CALL sites and sampled199 regressions all pass.
* Integration: native-created pose116 matrices/cache/FP match original;
* this body executed inside the real pose query fixture. Geometry startup
* reached four pipeline entries; first real multiply16 matched original.
* Unknown: Non-x86 FP parity and exceptional/non-default FP environments.
* Evidence: CODEX-RECON-008 / CODEX_RECON_008_MATRIX_CORE_RESULTS_2026-10-24.md
*/
B_Vector Rotate(const B_Vector &v, const B_Matrix &rotateMatrix)
{
    double z, y, x;
    MatrixDot(&z, &v.x, &rotateMatrix.m[0][2], 3);
    MatrixDot(&y, &v.x, &rotateMatrix.m[0][1], 3);
    MatrixDot(&x, &v.x, &rotateMatrix.m[0][0], 3);
    return B_Vector(x, y, z);
}

/*
* Module:                 Blade.exe
* Entry point:            0x0040578A
* VC++ mangling:          ?SetTranslation@B_Matrix@@QAEXABVB_Vector@@@Z
* Description: Copy six DWORDs to +0x60..+0x77, in increasing address order;
* all other bytes remain untouched. This is a reconstructed source name.
* Origin: PROTOCOL_CPP, CODEX-RECON-008; thiscall, one argument, RET 4.
* Validation: DIFFERENTIAL_IN_SCOPE; original repeat and x86 /Od,/O2;
* 256 cluster cases, exact buffer bytes/x87 status; five sequential native
* switches with retained ASM CALL sites and sampled199 regressions all pass.
* Integration: native-created pose116 matrices/cache/FP match original;
* this body executed inside the real pose query fixture. Geometry startup
* reached four pipeline entries; first real multiply16 matched original.
* Unknown: Original decorated name; invalid buffers are outside the contract.
* Evidence: CODEX-RECON-008 / CODEX_RECON_008_MATRIX_CORE_RESULTS_2026-10-24.md
*/
void B_Matrix::SetTranslation(const B_Vector &v)
{
    CopyMatrixWordPair(&m[3][0], &v.x);
    CopyMatrixWordPair(&m[3][1], &v.y);
    CopyMatrixWordPair(&m[3][2], &v.z);
}

/*
* Module:                 Blade.exe
* Entry point:            0x00406547
* VC++ mangling:          ?B_MultiplyMatrices@@YAXAAVB_Matrix@@ABV1@1@Z
* Description: Full 4x4 product, row-major output stores. Output aliasing is
* deliberately sequential: later results read values overwritten earlier.
* Origin: PROTOCOL_CPP, CODEX-RECON-008; cdecl explicit out, a, b; RET 0.
* Validation: DIFFERENTIAL_IN_SCOPE; original repeat and x86 /Od,/O2;
* 256 cluster cases, exact buffer bytes/x87 status; five sequential native
* switches with retained ASM CALL sites and sampled199 regressions all pass.
* Integration: native-created pose116 matrices/cache/FP match original;
* this body executed inside the real pose query fixture. Geometry startup
* reached four pipeline entries; first real multiply16 matched original.
* Unknown: Original decorated name; FP modes beyond PC53/nearest and non-x86.
* Evidence: CODEX-RECON-008 / CODEX_RECON_008_MATRIX_CORE_RESULTS_2026-10-24.md
*/
void B_MultiplyMatrices(B_Matrix &out, const B_Matrix &a, const B_Matrix &b)
{
    for (int row = 0; row < 4; ++row)
        for (int column = 0; column < 4; ++column)
            MatrixDot(&out.m[row][column], &a.m[row][0], &b.m[0][column], 4);
}

/*
* Module:                 Blade.exe
* Entry point:            0x00405AA2
* VC++ mangling:          ?RigidInverse@B_Matrix@@QBEXAAV1@@Z
* Description: Sequential transpose of 3x3, then negate the rotated translation.
* Output fourth column is never written. Self-alias uses the original destructive
* transpose order; this is not a general inverse. Reconstructed source name.
* Origin: PROTOCOL_CPP, CODEX-RECON-008; thiscall explicit out, RET 4.
* Validation: DIFFERENTIAL_IN_SCOPE; original repeat and x86 /Od,/O2;
* 256 cluster cases, exact buffer bytes/x87 status; five sequential native
* switches with retained ASM CALL sites and sampled199 regressions all pass.
* Integration: native-created pose116 matrices/cache/FP match original;
* this body executed inside the real pose query fixture. Geometry startup
* reached four pipeline entries; first real multiply16 matched original.
* Unknown: Original decorated name; exceptional and non-default FP modes.
* Evidence: CODEX-RECON-008 / CODEX_RECON_008_MATRIX_CORE_RESULTS_2026-10-24.md
*/
void B_Matrix::RigidInverse(B_Matrix &out) const
{
    for (int row = 0; row < 3; ++row)
        for (int column = 0; column < 3; ++column)
            CopyMatrixWordPair(&out.m[row][column], &m[column][row]);
    B_Vector translation(m[3][0], m[3][1], m[3][2]);
    B_Vector rotated = Rotate(translation, out);
    out.m[3][0] = -rotated.x;
    out.m[3][1] = -rotated.y;
    out.m[3][2] = -rotated.z;
}

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

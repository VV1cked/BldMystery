#include <Person/PoseCacheView.h>
#include <AnalyticGeometry/Matrix.h>
#include <AnalyticGeometry/Vector.h>

namespace {
template<class T> T &Field(void *owner, unsigned offset)
{
    return *reinterpret_cast<T *>(static_cast<unsigned char *>(owner) + offset);
}

bool EqualOrUnordered(const double &a, const double &b)
{
#if defined(_MSC_VER) && defined(_M_IX86)
    unsigned short status;
    __asm {
        mov eax, a
        fld qword ptr [eax]
        mov eax, b
        fcomp qword ptr [eax]
        fnstsw ax
        mov status, ax
    }
    return (status & 0x4000) != 0;
#else
    return a == b || a != a || b != b;
#endif
}
}

/*
* Module: Blade.exe
* Entry point: 0x00464A45
* VC++ mangling: ?RootMatrix@B_PoseCacheView@@QAEPAVB_Matrix@@XZ
* Description: Borrow animator+A8. Rebuild correction-or-translation times
* base(+6B0) when last phase(+88) is -1 or differs from cache(+128).
* The sentinel forces rebuilding even when both phases remain -1. Original
* x87 C3 tests also treat unordered comparisons as equal. Cache phase is copied
* as two DWORDs after all matrix stores. Returned storage remains owner-owned.
* Origin: PROTOCOL_CPP, CODEX-RECON-009; x86 thiscall, no stack arguments.
* Validation: DIFFERENTIAL_IN_SCOPE; 62 original snapshot pairs exact in
* production /Od and /O2, including both -1 phases and nonidentity correction.
* Linked native owner: 561 mode/flag/override cases plus two sentinel calls,
* matrix/cache/coefficient/scratch and recorded FP fields exact; pose116,
* math256 and sampled199 regressions pass. Borrowed return/stack/guards pass;
* host ESP/EBP/EBX/ESI/EDI preserved. Native build uses /Od /Ob1.
* Unknown: full owner layout/lifetime, concurrent access, other FP environments.
* Evidence: CODEX-RECON-009 / CODEX_RECON_009_POSE_CORE_RESULTS_2026-10-24.md
*/
B_Matrix *B_PoseCacheView::RootMatrix()
{
    const double sentinel = -1.0;
    const double &phase = Field<double>(this, 0x88);
    B_Matrix &out = Field<B_Matrix>(this, 0xa8);
    if (EqualOrUnordered(phase, sentinel) ||
        !EqualOrUnordered(Field<double>(this, 0x128), phase)) {
        const B_Vector &root = Field<B_Vector>(this, 0x90);
        const B_Matrix &base = Field<B_Matrix>(this, 0x6b0);
        if (Field<unsigned>(this, 0x130) & 2) {
            B_Matrix local = Field<B_Matrix>(this, 0x570);
            local.SetTranslation(root);
            B_MultiplyMatrices(out, local, base);
        } else {
            B_Matrix local(root);
            B_MultiplyMatrices(out, local, base);
        }
        volatile unsigned *dst = &Field<unsigned>(this, 0x128);
        const volatile unsigned *src = &Field<unsigned>(this, 0x88);
        dst[0] = src[0];
        dst[1] = src[1];
    }
    return &out;
}

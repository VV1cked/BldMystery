#include <Control/SampledRoot.h>

#include <math.h>
#include <string.h>

#if defined(USE_BICRT) && defined(_MSC_VER) && defined(_M_IX86)
// BICRT's existing helper consumes ST0 and returns its integer in EAX/EDX.
// A normal C++ cast would introduce the newer, unavailable __ftol2 helper.
extern "C" __declspec(dllimport) long __cdecl _ftol(void);
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x00470BF5
* VC++ mangling:          ?SampleRoot@B_SampledRootAdapterView@@QAEXNPANPAX@Z
*/
// Description: sampled adapter slot +0x0C writes three doubles. At phase >= 1
// it copies the last key verbatim. Otherwise the interval uses floor of the
// double product, while the weight separately rounds that product to float.
// The original control argument is unused. There is no lower phase clamp.
// Origin: PROTOCOL_CPP (CODEX-RECON-004).
// Validation: PARTIAL_DIFFERENTIAL; 21 original root cases / 63 double bits
// agree at PC53/nearest, including /Od, /O2 and the retained ASM table route
// in an x86 hybrid build. Stack, guards and input storage remain unchanged.
// Unknown: invalid/NaN/negative phase, malformed storage, aliasing output,
// non-default FP rounding/precision, FP status flags and unmasked exceptions.
// Evidence: CODEX-RECON-004 / CODEX_RECON_004_SAMPLED_ROOT_INTEGRATION_RESULTS_2026-10-22.md
void B_SampledRootAdapterView::SampleRoot(
    double phase, double *output, void *control)
{
    if (phase >= 1.0)
    {
        memcpy(output, sample->roots[sample->rootCount - 1], 3 * sizeof(double));
        return;
    }

    const double x = static_cast<double>(sample->rootCount - 1) * phase;
    // Explicit stores preserve the two distinct original float roundings.
    const volatile float roundedX = static_cast<float>(x);
    const double floored = floor(x);
    unsigned int index;
#if defined(USE_BICRT) && defined(_MSC_VER) && defined(_M_IX86)
    __asm
    {
        fld floored
        call _ftol
        mov index, eax
    }
#else
    index = static_cast<unsigned int>(floored);
#endif
    const volatile float weight = static_cast<float>(
        (static_cast<double>(index) + 1.0) - roundedX);
    const double firstWeight = weight;
    const double secondWeight = 1.0 - firstWeight;
    for (unsigned int j = 0; j < 3; ++j)
        output[j] = firstWeight * sample->roots[index][j] +
            secondWeight * sample->roots[index + 1][j];
}

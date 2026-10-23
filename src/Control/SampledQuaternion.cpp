#include <Control/SampledQuaternion.h>
#include <math.h>

#if defined(USE_BICRT) && defined(_MSC_VER) && defined(_M_IX86)
extern "C" __declspec(dllimport) long __cdecl _ftol(void);
#endif

// Exact original double bits: 0x3F0179EC9CB60095.
static const double sampledQuaternionScale = 0.00003333333333;

/*
* Module:                 Blade.exe
* Entry point:            0x004669B0
* VC++ mangling:          ?B_DecodeSampledQuaternion@@YAXPAMPBF@Z
*/
// Description: cdecl short4 decoder; signed components times double scale,
// each stored to float. The endpoint intentionally has no normalization.
// Origin: PROTOCOL_CPP (CODEX-RECON-005).
// Validation: PARTIAL_DIFFERENTIAL; 6 direct original helper cases / 24 float components
// and 4 sampler endpoints agree at PC53/nearest in production /Od, /O2 and
// linked x86 hybrid. Other original caller contexts were not executed.
// Unknown: output overlap, invalid pointers, FP modes/status/traps.
// Evidence: CODEX-RECON-005 / CODEX_RECON_005_SAMPLED_QUATERNION_RESULTS_2026-10-23.md
void __cdecl B_DecodeSampledQuaternion(float *output, const short *key)
{
    for (unsigned int j = 0; j < 4; ++j)
        output[j] = static_cast<float>(static_cast<double>(key[j]) * sampledQuaternionScale);
}

/*
* Module:                 Blade.exe
* Entry point:            0x00470CCC
* VC++ mangling:          ?B_BlendSampledQuaternion@@YAXPAMPBFM1@Z
*/
// Description: cdecl normalized linear blend; complement and scaled weights
// round independently to float before signed-short products. Mixed components
// and reciprocal norm also round to float before final multiplication.
// Origin: PROTOCOL_CPP (CODEX-RECON-005).
// Validation: PARTIAL_DIFFERENTIAL; 88 direct original helper cases / 352
// float components and 80 sampler interiors agree bitwise at PC53/nearest
// in production /Od, /O2 and linked x86 hybrid. Other callers untested.
// Unknown: other caller contexts, zero norm, overlap, FP modes/status/traps.
// Evidence: CODEX-RECON-005 / CODEX_RECON_005_SAMPLED_QUATERNION_RESULTS_2026-10-23.md
void __cdecl B_BlendSampledQuaternion(float *output, const short *first,
    float weight, const short *second)
{
    const volatile float complement = static_cast<float>(1.0 - static_cast<double>(weight));
    const volatile float a = static_cast<float>(static_cast<double>(weight) * sampledQuaternionScale);
    const volatile float b = static_cast<float>(static_cast<double>(complement) * sampledQuaternionScale);
    for (unsigned int j = 0; j < 4; ++j)
        output[j] = static_cast<float>(static_cast<double>(first[j]) * a +
            static_cast<double>(second[j]) * b);
    const double x = output[0], y = output[1], z = output[2], w = output[3];
    const volatile float inverse = static_cast<float>(1.0 / sqrt(((x*x + y*y) + z*z) + w*w));
    for (unsigned int j = 0; j < 4; ++j)
        output[j] = static_cast<float>(static_cast<double>(inverse) * output[j]);
}

/*
* Module:                 Blade.exe
* Entry point:            0x00470E13
* VC++ mangling:          ?SampleQuaternion@B_SampledRootAdapterView@@QAEXNIPAMPAX@Z
*/
// Description: thiscall slot +10 writes float4; interior uses sample.rootCount
// and floor of the double product, with separate float product/weight stores.
// Endpoint uses node.keyCount and does not normalize. Control is unused.
// Origin: PROTOCOL_CPP (CODEX-RECON-005).
// Validation: PARTIAL_DIFFERENTIAL; 84 original cases / 336 float components
// agree bitwise in production /Od, /O2 and retained ASM ctor/vtable slot +10
// dispatch in x86 hybrid. Stack, guards and input storage checks pass.
// Unknown: malformed storage, negative/NaN/Inf phase, overlap, FP modes/status/traps.
// Evidence: CODEX-RECON-005 / CODEX_RECON_005_SAMPLED_QUATERNION_RESULTS_2026-10-23.md
void B_SampledRootAdapterView::SampleQuaternion(
    double phase, unsigned int node, float *output, void *control)
{
    const B_SampledQuaternionNodeView &descriptor = sample->nodes[node];
    if (phase >= 1.0)
    {
        B_DecodeSampledQuaternion(output, descriptor.keys[descriptor.keyCount - 1]);
        return;
    }
    const double x = static_cast<double>(sample->rootCount - 1) * phase;
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
    const volatile float weight = static_cast<float>((static_cast<double>(index) + 1.0) - roundedX);
    B_BlendSampledQuaternion(output, descriptor.keys[index], weight, descriptor.keys[index + 1]);
}

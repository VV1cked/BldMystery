#define BBLIBC_LIB_EXPORT
#include "RDataLight.h"
#include <cstring>

namespace
{
void InterpolateLightSlot(float *result, const float *start, double t,
    const float *end, double u)
{
    // Keep both products on the x87 stack until the sum is stored as float.
    // SSE/double temporaries can change rounding, NaN propagation and flags.
    __asm {
        mov eax, start
        fld dword ptr [eax]
        fmul t
        mov ecx, end
        fld dword ptr [ecx]
        fmul u
        faddp st(1), st(0)
        mov edx, result
        fstp dword ptr [edx]
    }
}
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x10007EF0
* VC++ mangling:          ??4B_RDataLight@@QAEAAV0@ABV0@@Z
*/
B_RDataLight &B_RDataLight::operator=(const B_RDataLight &other)
{
    // Original copies four dwords in ascending order, without floating-point
    // loads. Preserve NaN payloads and self-assignment without FP exceptions.
    for (unsigned int offset = 0; offset < 16; offset += 4)
    {
        unsigned int word;
        std::memcpy(&word, reinterpret_cast<const unsigned char *>(&other) + offset, 4);
        std::memcpy(reinterpret_cast<unsigned char *>(this) + offset, &word, 4);
    }
    return *this;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10040CB0
* VC++ mangling:          ??0B_RDataLight@@QAE@XZ
*/
B_RDataLight::B_RDataLight()
{
    // Original leaves all four slots uninitialized.
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x10040CBE
* VC++ mangling:          ??0B_RDataLight@@QAE@MMMM@Z
*/
B_RDataLight::B_RDataLight(float a, float b, float c, float d)
{
    // Despite accepting four floats, the original writes only its stack
    // arguments back to themselves and leaves every object byte untouched.
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x10040CE6
* VC++ mangling:          ?Interp@B_RDataLight@@QAEXNAAV1@N0@Z
*/
void B_RDataLight::Interp(double t, B_RDataLight &start, double u, B_RDataLight &end)
{
    // Weights are independent: no normalization or clamping. Original stores
    // slots in ascending order; either input may be the destination itself.
    InterpolateLightSlot(&unknown00, &start.unknown00, t, &end.unknown00, u);
    InterpolateLightSlot(&unknown04, &start.unknown04, t, &end.unknown04, u);
    InterpolateLightSlot(&unknown08, &start.unknown08, t, &end.unknown08, u);
    InterpolateLightSlot(&unknown0C, &start.unknown0C, t, &end.unknown0C, u);
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10040F39
* VC++ mangling:          ??0B_RItem@@QAE@K@Z
*/
B_RItem::B_RItem(unsigned long value) : unknown00(value)
{
    // Original stores the argument verbatim in the sole base field.
}

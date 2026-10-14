#define BBLIBC_LIB_EXPORT
#include "RDataTriangle.h"
#include <cstring>


/*
* Module:                 BBLibc.dll
* Entry point:            0x10007F40
* VC++ mangling:          ??4B_RDataTriangle@@QAEAAV0@ABV0@@Z
*/
B_RDataTriangle &B_RDataTriangle::operator=(const B_RDataTriangle &other)
{
    // Original REP MOVSD copies seven words forwards, including the base.
    // Copy one word at a time to preserve that order, including self-copy.
    for (unsigned int offset = 0; offset < 28; offset += 4)
    {
        unsigned int word;
        std::memcpy(&word, reinterpret_cast<const unsigned char *>(&other) + offset, 4);
        std::memcpy(reinterpret_cast<unsigned char *>(this) + offset, &word, 4);
    }
    return *this;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10040F51
* VC++ mangling:          ??0B_RDataTriangle@@QAE@XZ
*/
B_RDataTriangle::B_RDataTriangle() : B_RItem(1)
{
    // Only the base dword is initialized; the six remaining words are untouched.
}

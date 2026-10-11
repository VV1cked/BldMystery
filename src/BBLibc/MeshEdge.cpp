#define BBLIBC_LIB_EXPORT
#include "MeshEdge.h"
#include <cstring>


/*
* Module:                 BBLibc.dll
* Entry point:            0x10007F70
* VC++ mangling:          ??4B_MeshEdge@@QAEAAV0@ABV0@@Z
*/
B_MeshEdge &B_MeshEdge::operator=(const B_MeshEdge &other)
{
    // Original copies all six dwords, including fields ignored by equality.
    if (this != &other)
        std::memcpy(this, &other, sizeof(B_MeshEdge));
    return *this;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x100321F0
* VC++ mangling:          ??0B_MeshEdge@@QAE@XZ
*/
B_MeshEdge::B_MeshEdge()
{
    // Original leaves field and offsets 0x0C/0x10 untouched.
    firstIndex = secondIndex = 0;
    field_14 = 1;
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x1003221C
* VC++ mangling:          ??0B_MeshEdge@@QAE@III@Z
*/
B_MeshEdge::B_MeshEdge(unsigned int a, unsigned int b, unsigned int c)
{
    // Original orders the pair using unsigned comparisons, preserving equal indices.
    field = c;
    firstIndex = a < b ? a : b;
    secondIndex = a > b ? a : b;
    // Offsets 0x0C and 0x10 remain uninitialized in this constructor too.
    field_14 = 1;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x1003227E
* VC++ mangling:          ??8B_MeshEdge@@QBEIABV0@@Z
*/
unsigned int B_MeshEdge::operator==(const B_MeshEdge &other) const
{
    // Original returns exactly 0/1 and ignores all fields outside the ordered pair.
    return firstIndex == other.firstIndex && secondIndex == other.secondIndex;
}


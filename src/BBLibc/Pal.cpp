#define BBLIBC_LIB_EXPORT
#include "Pal.h"
#include <cstring>

/*
* Module:                 BBLibc.dll
* Entry point:            0x10006BE0
* VC++ mangling:          ??0B_Pal@@QAE@XZ
*/
B_Pal::B_Pal()
{
    // Original leaves all 768 palette bytes uninitialized.
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x100093A7
* VC++ mangling:          ??0B_Pal@@QAE@ABV0@@Z
*/
B_Pal::B_Pal(const B_Pal &other)
{
    // Original copies the complete packed RGB table, with no allocation or normalization.
    std::memcpy(data, other.data, sizeof(data));
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x100093C8
* VC++ mangling:          ?GetRed@B_Pal@@QBEEI@Z
*/

unsigned char B_Pal::GetRed(unsigned int val) const
{
    // Original uses an unchecked, wrapping 32-bit RGB byte offset.
    return data[3u * val];
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x100093E1
* VC++ mangling:          ?GetGreen@B_Pal@@QBEEI@Z
*/

unsigned char B_Pal::GetGreen(unsigned int val) const
{
    return data[3u * val + 1u];
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x100093FB
* VC++ mangling:          ?GetBlue@B_Pal@@QBEEI@Z
*/

unsigned char B_Pal::GetBlue(unsigned int val) const
{
    return data[3u * val + 2u];
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10009415
* VC++ mangling:          ?SetRed@B_Pal@@QAEXIE@Z
*/
void B_Pal::SetRed(unsigned int arg_1, unsigned char arg_2)
{
    // Original setters change only the requested byte, with no index validation.
    data[3u * arg_1] = arg_2;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10009431
* VC++ mangling:          ?SetGreen@B_Pal@@QAEXIE@Z
*/
void B_Pal::SetGreen(unsigned int arg_1, unsigned char arg_2)
{
    data[3u * arg_1 + 1u] = arg_2;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x1000944E
* VC++ mangling:          ?SetBlue@B_Pal@@QAEXIE@Z
*/
void B_Pal::SetBlue(unsigned int arg_1, unsigned char arg_2)
{
    data[3u * arg_1 + 2u] = arg_2;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x1000946B
* VC++ mangling:          ?GetIndex@B_Pal@@QBEIEEEI@Z
*/
unsigned int B_Pal::GetIndex(
    unsigned char arg_1,
    unsigned char arg_2,
    unsigned char arg_3,
    unsigned int arg_4
) const
{
    // Original starts at arg_4, ignoring all earlier colors, and uses squared RGB distance.
    // Only strictly smaller distances replace the winner, so ties keep the earliest candidate.
    const auto distance = [this, arg_1, arg_2, arg_3](unsigned int index) {
        const int red = static_cast<int>(arg_1) - GetRed(index);
        const int green = static_cast<int>(arg_2) - GetGreen(index);
        const int blue = static_cast<int>(arg_3) - GetBlue(index);
        return red * red + green * green + blue * blue;
    };
    unsigned int best = arg_4;
    int bestDistance = distance(arg_4);
    // The initial index is not validated, even though later candidates stop at 256.
    for (unsigned int index = arg_4 + 1u; index < 256u; ++index)
    {
        const int candidateDistance = distance(index);
        if (candidateDistance < bestDistance)
        {
            best = index;
            bestDistance = candidateDistance;
        }
    }
    return best;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10009633
* VC++ mangling:          ?Id@B_Pal@@QAEABV1@XZ
*/
const B_Pal &B_Pal::Id()
{
    // Original returns this palette by reference, without inspecting or copying its bytes.
    return *this;
}

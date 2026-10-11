#define BBLIBC_LIB_EXPORT
#include "BitMap.h"
#include "ODataFile.h"
#include <new>
#include <cstring>


/*
* Module:                 BBLibc.dll
* Entry point:            0x100085F0
* VC++ mangling:          ??0B_BitMap@@QAE@XZ
*/
B_BitMap::B_BitMap()
    : type(static_cast<BitMapType>(0)), dimension1(0), dimension2(0), data(nullptr), pal(nullptr)
{
    // Original initializes all five fields to zero, including the otherwise unnamed type 0.
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10008639
* VC++ mangling:          ??0B_BitMap@@QAE@W4BitMapType@0@IIPBVB_Pal@@@Z
*/
B_BitMap::B_BitMap(BitMapType type, unsigned int width, unsigned int height, const B_Pal *palette)
    : type(type), dimension1(width), dimension2(height)
{
    // Original wraps the allocation size to 32 bits and does not initialize pixel bytes.
    // It still requests an allocation when the computed size is zero.
    const unsigned int size = width * height * static_cast<unsigned int>(BytesPerTexel());
    data = ::operator new(size);
    // The palette is copied independently of bitmap type; null stays null.
    pal = nullptr;
    if (palette)
    {
        void *storage = ::operator new(sizeof(B_Pal));
        pal = storage ? new (storage) B_Pal(*palette) : nullptr;
    }
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x100086E3
* VC++ mangling:          ??0B_BitMap@@QAE@ABV0@@Z
*/
B_BitMap::B_BitMap(const B_BitMap &src)
    : type(src.type), dimension1(src.dimension1), dimension2(src.dimension2), pal(nullptr)
{
    // Original copies palettes only for types 1 and 3, requiring a non-null source palette.
    // For unknown types it leaves data untouched, even though the destructor later frees it.
    const unsigned int code = static_cast<unsigned int>(type);
    if (code < 1u || code > 5u) return;
    if (code == 1u || code == 3u)
    {
        void *storage = ::operator new(sizeof(B_Pal));
        pal = storage ? new (storage) B_Pal(*src.pal) : nullptr;
    }
    const unsigned int size = dimension1 * dimension2 * static_cast<unsigned int>(BytesPerTexel());
    data = ::operator new(size);
    // Original copies exactly the wrapped byte count and does not check allocation failure.
    if (size) std::memcpy(data, src.data, size);
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10008974
* VC++ mangling:          ??0B_BitMap@@QAE@W4BitMapType@0@ABVB_BitMap24@@@Z
*/
#ifndef BLD_NATIVE
B_BitMap::B_BitMap(BitMapType arg_1, const B_BitMap24 &arg_2)
{
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x10008AF0
* VC++ mangling:          ?BytesPerTexel@B_BitMap@@QBEHXZ
*/
int B_BitMap::BytesPerTexel() const
{
    // Original maps both type 1 and type 2 to one byte; unknown types return zero.
    switch (static_cast<unsigned int>(type))
    {
        case 1: case 2: return 1;
        case 3: return 2;
        case 4: return 3;
        case 5: return 4;
        default: return 0;
    }
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10008B51
* VC++ mangling:          ??6@YAAAVB_ODataFile@@AAV0@ABVB_BitMap@@@Z
*/
#ifndef BLD_NATIVE
B_ODataFile &operator <<(B_ODataFile &file, B_BitMap const &bitMap)
{
    file.Write(&bitMap, sizeof(B_BitMap));
    return file;
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x10008C9B
* VC++ mangling:          ??5@YAAAVB_IDataFile@@AAV0@AAVB_BitMap@@@Z
*/
#ifndef BLD_NATIVE
B_IDataFile &operator >>(B_IDataFile &file, B_BitMap &bitMap)
{
    return file;
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x10008FDA
* VC++ mangling:          ??1B_BitMap@@UAE@XZ
*/
B_BitMap::~B_BitMap()
{
    // Original uses scalar deallocation for data, then palette, without clearing fields.
    // B_Pal has no destructor work; its raw allocation is released directly.
    if (data) ::operator delete(data);
    if (pal) ::operator delete(pal);
}

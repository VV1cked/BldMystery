#define BBLIBC_LIB_EXPORT
#include "BitMap24.h"
#include "ODataFile.h"
#include "Color.h"
#include <new>
#include <cstdio>
#include <cstring>
#include <cassert>

static void CopyBitmap24Bytes(void *destination, const void *source, unsigned int size)
{
    // Original copies forward in dwords, then bytes; overlapping buffers are not memmove.
    byte *out = static_cast<byte *>(destination);
    const byte *in = static_cast<const byte *>(source);
    unsigned int offset = 0;
    for (unsigned int i = 0; i < size / 4u; ++i, offset += 4)
    {
        unsigned int word;
        std::memcpy(&word, in + offset, 4);
        std::memcpy(out + offset, &word, 4);
    }
    for (unsigned int i = 0; i < size % 4u; ++i, ++offset)
        out[offset] = in[offset];
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10006E20
* VC++ mangling:          ?Clear@B_BitMap24@@QAEXXZ
*/
void B_BitMap24::Clear()
{
    // Original uses scalar delete, clears data after release, then height and width.
    // A null buffer still causes both dimensions to be cleared.
    if (data)
    {
        ::operator delete(data);
        data = nullptr;
    }
    dimension2 = 0;
    dimension1 = 0;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10007090
* VC++ mangling:          ?GetDimension@B_BitMap24@@QBEXAAH0@Z
*/
void B_BitMap24::GetDimension(int &arg_1, int &arg_2) const
{
    // Original writes width then height, so aliased outputs end with height.
    arg_1 = static_cast<int>(dimension1);
    arg_2 = static_cast<int>(dimension2);
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x100070C0
* VC++ mangling:          ?GetBits@B_BitMap24@@QBEPBEXZ
*/
const unsigned char *B_BitMap24::GetBits() const
{
    // Original returns the stored pointer, without allocation or null validation.
    return static_cast<const unsigned char *>(data);
}


/*
* Module:                 BBLibc.dll
* Data address:           0x1004A520
* VC++ mangling:          ?BitMap24DataList@B_BitMap24@@0V?$B_Array@VB_BitMap24@@@@A
*/
#ifndef BLD_NATIVE
B_Array<B_BitMap24> B_BitMap24::BitMap24DataList(1);
#endif


/*
* Module:                 BBLibc.dll
* Entry point:            0x10006CB0
* VC++ mangling:          ??0B_BitMap24@@QAE@HH@Z
*/
#ifndef BLD_NATIVE
B_BitMap24::B_BitMap24(int width, int height)
{

}
#endif


/*
* Module:                 BBLibc.dll
* Entry point:            0x10006E70
* VC++ mangling:          ??1B_BitMap24@@UAE@XZ
*/
#ifndef BLD_NATIVE
B_BitMap24::~B_BitMap24()
{

}
#endif


/*
* Module:                 BBLibc.dll
* Entry point:            0x10026AD5
* VC++ mangling:          ??0B_BitMap24@@QAE@ABV0@@Z
*/
#ifndef BLD_NATIVE
B_BitMap24::B_BitMap24(const B_BitMap24 &that)
{
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x10026C70
* VC++ mangling:          ?ClearBitMap24Data@B_BitMap24@@SAXXZ
*/
#ifndef BLD_NATIVE
void B_BitMap24::ClearBitMap24Data()
{

}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x10026D6D
* VC++ mangling:          ??0B_BitMap24@@QAE@ABVB_BitMap@@@Z
*/
#ifndef BLD_NATIVE
B_BitMap24::B_BitMap24(const B_BitMap &arg_1)
{
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x100271BE
* VC++ mangling:          ??4B_BitMap24@@QAEAAV0@ABV0@@Z
*/
B_BitMap24 &B_BitMap24::operator=(const B_BitMap24 &that)
{
    // Equal dimensions reuse the current buffer, including self-assignment and zero dimensions.
    if (dimension1 != that.dimension1 || dimension2 != that.dimension2)
    {
        dimension1 = that.dimension1;
        dimension2 = that.dimension2;
        ::operator delete(data);
        // Only the resize branch normalizes either zero dimension to an entirely empty image.
        if (!dimension1 || !dimension2)
        {
            dimension2 = dimension1 = 0;
            data = nullptr;
            return *this;
        }
        data = ::operator new(dimension1 * dimension2 * 3u);
    }
    // The original byte count wraps to 32 bits and does not validate source storage.
    CopyBitmap24Bytes(data, that.data, dimension1 * dimension2 * 3u);
    return *this;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x100272A5
* VC++ mangling:          ?SetBits@B_BitMap24@@QAEIHHPBD0@Z
*/
unsigned int B_BitMap24::SetBits(int width, int height, const char *bits, const char *format)
{
    // Original compares exactly four bytes, including the terminator; unsupported formats do nothing.
    if (std::memcmp(format, "RGB", 4) != 0) return 0;
    if (dimension1 != static_cast<unsigned int>(width) || dimension2 != static_cast<unsigned int>(height))
    {
        ::operator delete(data);
        dimension1 = static_cast<unsigned int>(width);
        dimension2 = static_cast<unsigned int>(height);
        // Unlike assignment, a zero dimension is retained and still requests a zero-byte allocation.
        data = ::operator new(dimension1 * dimension2 * 3u);
        if (!data) _assert("Bits", "C:\\code\\BITMAP24\\BITMAP24.CPP", 141);
    }
    CopyBitmap24Bytes(data, bits, dimension1 * dimension2 * 3u);
    return 1;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10027382
* VC++ mangling:          ?SaveToBMP@B_BitMap24@@QBEHPBD@Z
*/
#ifndef BLD_NATIVE
int B_BitMap24::SaveToBMP(const char *fileName) const
{
    return 0;
}
#endif


/*
* Module:                 BBLibc.dll
* Entry point:            0x1002778F
* VC++ mangling:          ?SaveToBMPSized@B_BitMap24@@QBEHPBDHH@Z
*/
#ifndef BLD_NATIVE
int B_BitMap24::SaveToBMPSized(const char *fileName, int width, int height) const
{
    return 0;
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x10027D86
* VC++ mangling:          ?ReadFromBMP@B_BitMap24@@QAEHPBD@Z
*/
#ifndef BLD_NATIVE
int B_BitMap24::ReadFromBMP(const char *arg_1)
{
    return 0;
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x10027E11
* VC++ mangling:          ?ReadFromFile@B_BitMap24@@QAEHPBD@Z
*/
#ifndef BLD_NATIVE
int B_BitMap24::ReadFromFile(const char *arg_1)
{
    return 0;
}
#endif


/*
* Module:                 BBLibc.dll
* Entry point:            0x10027F15
* VC++ mangling:          ?SaveToFile@B_BitMap24@@QBEHPBDH@Z
*/
#ifndef BLD_NATIVE
int B_BitMap24::SaveToFile(const char*, int) const
{
    return 0;
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x10028021
* VC++ mangling:          ??5@YAAAVB_IDataFile@@AAV0@AAVB_BitMap24@@@Z
*/
#ifndef BLD_NATIVE
B_IDataFile &operator >>(B_IDataFile &file, B_BitMap24 &bitMap)
{
    return file;
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x100280CC
* VC++ mangling:          ??6@YAAAVB_ODataFile@@AAV0@ABVB_BitMap24@@@Z
*/
#ifndef BLD_NATIVE
B_ODataFile &operator <<(B_ODataFile &file, B_BitMap24 const &bitMap)
{
    file.Write(&bitMap, sizeof(B_BitMap24));
    return file;
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x10028353
* VC++ mangling:          ?EscribeArchivoRAW@B_BitMap24@@QBEXPBD@Z
*/
void B_BitMap24::EscribeArchivoRAW(const char *fileName) const
{
    // Original writes only packed RGB bytes, with a wrapping 32-bit count and no header.
    const unsigned int size = dimension1 * dimension2 * 3u;
    FILE *file = std::fopen(fileName, "wb");
    if (!file) return; // Original reports no open/write error to the caller.
    std::fwrite(data, 1, size, file);
    // Both complete and short writes close the file; the result is ignored.
    std::fclose(file);
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x100283CD
* VC++ mangling:          ?ChangeRGBOrder@B_BitMap24@@QAEXXZ
*/
void B_BitMap24::ChangeRGBOrder()
{
    // Original wraps the dimension product to 32 bits, then compares it as signed.
    // A zero or negative product skips the buffer entirely; there is no null check.
    const int count = static_cast<int>(dimension1 * dimension2);
    unsigned char *pixel = static_cast<unsigned char *>(data);
    for (int i = 0; i < count; ++i, pixel += 3)
    {
        const unsigned char first = pixel[0];
        pixel[0] = pixel[2];
        pixel[2] = first;
    }
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10028434
* VC++ mangling:          ?GetPixelAt@B_BitMap24@@QBEIHHAAVB_Color@@@Z
*/
unsigned int B_BitMap24::GetPixelAt(int arg_1, int arg_2, B_Color &arg_3) const
{
    // Original compares dimensions as signed and leaves the output untouched on failure.
    if (arg_1 < 0 || arg_1 >= static_cast<int>(dimension1) ||
        arg_2 < 0 || arg_2 >= static_cast<int>(dimension2))
        return 0;
    // Offset arithmetic wraps to 32 bits. RGB bytes are written directly, with no color hook.
    const unsigned int offset = 3u * (static_cast<unsigned int>(arg_2) * dimension1 +
        static_cast<unsigned int>(arg_1));
    const unsigned char *pixel = static_cast<const unsigned char *>(data) + offset;
    arg_3.r = pixel[0];
    arg_3.g = pixel[1];
    arg_3.b = pixel[2];
    return 1;
}


//B_BitMap24 B_BitMap24::ReadFromJPEG(const char*)
//{
//}


/*
* Module:                 BBLibc.dll
* Entry point:            0x10029700
* VC++ mangling:          ?SaveToJPEG@B_BitMap24@@QBEHPBDH@Z
*/
#ifndef BLD_NATIVE
int B_BitMap24::SaveToJPEG(const char*, int) const
{
    return 0;
}
#endif

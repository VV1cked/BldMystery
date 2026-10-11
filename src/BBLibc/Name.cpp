#define BBLIBC_LIB_EXPORT
#include "Name.h"
#include "ODataFile.h"


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003300
* VC++ mangling:          ??0B_Name@@QAE@XZ
*/

B_Name::B_Name() : string(new char[1]), len(0)
{
    *string = 0;
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003337
* VC++ mangling:          ??0B_Name@@QAE@PBD@Z
*/

B_Name::B_Name(const char *str)
{
    this->len = static_cast<unsigned int>(strlen(str));
    this->string = new char[this->len + 1];
    strcpy(this->string, str);
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x100033A4
* VC++ mangling:          ??0B_Name@@QAE@ABV0@@Z
*/

B_Name::B_Name(const B_Name &src) : len(src.len)
{
    string = new char[len + 1];
    strcpy(string, src.string);
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003410
* VC++ mangling:          ??0B_Name@@QAE@I@Z
*/

B_Name::B_Name(unsigned int len) : len(len)
{
    string = new char[len + 1];
    string[len] = '\0';
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003457
* VC++ mangling:          ?String@B_Name@@QBEQADXZ
*/

char * const B_Name::String() const
{
    return this->string;
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003467
* VC++ mangling:          ??OB_Name@@QBEIABV0@@Z
*/
unsigned int B_Name::operator >(const B_Name &rhs) const
{
    // Original ordering ignores stored lengths and compares unsigned bytes up to NUL.
    return strcmp(this->string, rhs.string) > 0;
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x100034DA
* VC++ mangling:          ??MB_Name@@QBEIABV0@@Z
*/
unsigned int B_Name::operator <(const B_Name &rhs) const
{
    // Original ordering ignores stored lengths and compares unsigned bytes up to NUL.
    return strcmp(this->string, rhs.string) < 0;
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x1000354D
* VC++ mangling:          ??8B_Name@@QBEIABV0@@Z
*/
unsigned int B_Name::operator ==(const B_Name &rhs) const
{
    // String() can change the text without updating len; the original checks len first.
    if (this->len != rhs.len)
    {
        return 0;
    }
    return strcmp(this->string, rhs.string) == 0;
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x100035D2
* VC++ mangling:          ??8B_Name@@QBEIPBD@Z
*/
unsigned int B_Name::operator ==(const char *str) const
{
    // This overload ignores stored length and stops at the first NUL.
    return strcmp(this->string, str) == 0;
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003643
* VC++ mangling:          ??9B_Name@@QBEIABV0@@Z
*/
unsigned int B_Name::operator !=(const B_Name &rhs) const
{
    // Different stored lengths always return 1, regardless of lexical order.
    if (this->len != rhs.len)
    {
        return 1;
    }
    // The original returns the comparison sign, not a boolean: -1 becomes 0xFFFFFFFFu.
    const int comparison = strcmp(this->string, rhs.string);
    return static_cast<unsigned int>((comparison > 0) - (comparison < 0));
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x100036C5
* VC++ mangling:          ??9B_Name@@QBEIPBD@Z
*/
unsigned int B_Name::operator !=(const char *str) const
{
    // Stored length is ignored; preserve the original 0, 1 or 0xFFFFFFFFu result.
    const int comparison = strcmp(this->string, str);
    return static_cast<unsigned int>((comparison > 0) - (comparison < 0));
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10003730
* VC++ mangling:          ?CompareNoCase@B_Name@@QBEIABV1@@Z
*/
unsigned int B_Name::CompareNoCase(const B_Name &arg_1) const
{
    // Compare stored lengths first, even if mutable buffers now contain equal text.
    if (this->len != arg_1.len)
    {
        return 0;
    }
    return stricmp(this->string, arg_1.string) == 0;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10003769
* VC++ mangling:          ?CompareNoCase@B_Name@@QBEIPBD@Z
*/

unsigned int B_Name::CompareNoCase(const char *str) const
{
    return stricmp(this->string, str) == 0;
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x1000378E
* VC++ mangling:          ??4B_Name@@QAEAAV0@ABV0@@Z
*/
#ifndef BLD_NATIVE
B_Name &B_Name::operator =(const B_Name &src)
{
    if (this != &src) {
        //delete[] string;
        //delete string;
        len = src.len;
        string = new char[len + 1];
        strcpy(string, src.string);
    }
    return *this;
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x10003821
* VC++ mangling:          ?Right@B_Name@@QBE?AV1@I@Z
*/
B_Name B_Name::Right(unsigned int count) const
{
    // Clamp unsigned count to stored length; UINT_MAX requests the full stored length.
    if (count > len) {
        count = len;
    }

    B_Name result(count);
    // Use stored length for the suffix offset, so it may start after an earlier NUL.
    // The result keeps count as its length even if strncpy encounters a NUL.
    strncpy(result.string, string + (len - count), count);
    return result;
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003898
* VC++ mangling:          ?Left@B_Name@@QBE?AV1@I@Z
*/
B_Name B_Name::Left(unsigned int count) const
{
    // Clamp unsigned count to stored length; UINT_MAX requests the full stored length.
    if (count > len) {
        count = len;
    }

    B_Name result(count);
    // Keep count as the result length even if strncpy stops at an earlier NUL.
    strncpy(result.string, string, count);
    return result;
}

/*
* Module:                 BBLibc.dll
* Entry point:            0x10003904
* VC++ mangling:          ??5@YAAAVB_IDataFile@@AAV0@AAVB_Name@@@Z
*/
#ifndef BLD_NATIVE
B_IDataFile &operator >>(B_IDataFile &file, B_Name &name)
{
    return file;
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x1000398A
* VC++ mangling:          ??6@YAAAVB_ODataFile@@AAV0@ABVB_Name@@@Z
*/
#ifndef BLD_NATIVE
B_ODataFile &operator <<(B_ODataFile &file, B_Name const &name)
{
    file.Write(&name, sizeof(B_Name));
    return file;
}
#endif

/*
* Module:                 BBLibc.dll
* Entry point:            0x100039BA
* VC++ mangling:          ??1B_Name@@QAE@XZ
*/

B_Name::~B_Name() {
    len = 0;
    delete [] string;
}


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003A70
* VC++ mangling:          ??H@YA?AVB_Name@@ABV0@0@Z
*/
#ifndef BLD_NATIVE
B_Name operator +(B_Name const &lother, B_Name const &rother)
{
    return lother;
}
#endif


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003B1E
* VC++ mangling:          ??H@YA?AVB_Name@@ABV0@PBD@Z
*/
#ifndef BLD_NATIVE
B_Name operator +(B_Name const &other, const char *str)
{
    return other;
}
#endif


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003BC7
* VC++ mangling:          ??H@YA?AVB_Name@@PBDABV0@@Z
*/
#ifndef BLD_NATIVE
B_Name operator +(const char *str, const B_Name &bName)
{
    return str;
}
#endif


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003C70
* VC++ mangling:          ??YB_Name@@QAEABV0@PBD@Z
*/
#ifndef BLD_NATIVE
B_Name const &B_Name::operator +=(const char *str)
{
    return *this;
}
#endif


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003D30
* VC++ mangling:          ??YB_Name@@QAEABV0@D@Z
*/
#ifndef BLD_NATIVE
const B_Name &B_Name::operator+=(char c)
{
    return *this;
}
#endif


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003DCE
* VC++ mangling:          ??YB_Name@@QAEABV0@ABV0@@Z
*/
#ifndef BLD_NATIVE
B_Name const &B_Name::operator +=(const B_Name &other)
{
    return *this;
}
#endif


/*
* Module:                 BBLibc.dll
* Entry point:            0x10003E91
* VC++ mangling:          ?UpCase@B_Name@@QAEXXZ
*/

void B_Name::UpCase()
{
    strupr(this->string);
}

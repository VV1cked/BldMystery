#ifndef B_RDataTriangle_H
#define B_RDataTriangle_H
#include "RDataLight.h"

#ifdef BBLIBC_LIB_EXPORT
#define BUILD_LIB
#endif
#include <export.h>
#undef BUILD_LIB


class LIB_EXP B_RDataTriangle : public B_RItem
{
public:
    B_RDataTriangle();

    B_RDataTriangle &operator=(const B_RDataTriangle&);

private:
    // Original assignment copies seven dwords including the B_RItem base.
    // The remaining fields' interpretation is not yet established.
    unsigned long unknown04[6];
};

#endif  // B_RDataTriangle_H

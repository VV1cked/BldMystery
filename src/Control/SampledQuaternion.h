#ifndef SAMPLED_QUATERNION_H
#define SAMPLED_QUATERNION_H

#include <Control/SampledRoot.h>

// Borrowed node descriptor, stride 0x14; keys are preprocessed signed short4.
// Only the fields consumed by sampling are represented; no owner is introduced.
struct B_SampledQuaternionNodeView
{
    unsigned int unknown00;
    const short (*keys)[4];
    unsigned int keyCount;
    unsigned char unknown0C[8];
};

// Method contracts and scoped differential results are next to the definitions.
void __cdecl B_DecodeSampledQuaternion(float *output, const short *key);
void __cdecl B_BlendSampledQuaternion(float *output, const short *first,
    float weight, const short *second);

#endif

#ifndef SAMPLED_ROOT_H
#define SAMPLED_ROOT_H

// Non-owning prefix of the original sampled storage, not its complete owner.
// Description: nodes/count at +0x10/+0x14; roots/count at +0x34/+0x38.
// Root key stride is 24 bytes; quaternion node descriptors have stride 0x14.
// Origin: INTERFACE_ONLY.
// Validation: ORIGINAL_BEHAVIOR_IN_SCOPE; original ctor allocates an 8-byte
// adapter and stores this sample pointer at +4. No C++ lifecycle is introduced.
// Unknown: complete storage type and ownership outside the sampling call.
// Evidence: CODEX-RECON-004 / CODEX_RECON_004_SAMPLED_ROOT_INTEGRATION_RESULTS_2026-10-22.md
// Evidence: CODEX-RECON-005 / CODEX_RECON_005_SAMPLED_QUATERNION_RESULTS_2026-10-23.md
struct B_SampledRootDataView
{
    unsigned char unknown00[0x10];
    const struct B_SampledQuaternionNodeView *nodes;
    unsigned int nodeCount;
    unsigned char unknown18[0x1c];
    const double (*roots)[3];
    unsigned int rootCount;
};

// This view has no virtual functions, constructor or destructor. The explicit
// first word belongs to the original ASM vtable; C++ never creates a vtable.
struct B_SampledRootAdapterView
{
    const void *originalVtable;
    const B_SampledRootDataView *sample;

    void SampleRoot(double phase, double *output, void *control);
    void SampleQuaternion(double phase, unsigned int node, float *output, void *control);
};

#endif

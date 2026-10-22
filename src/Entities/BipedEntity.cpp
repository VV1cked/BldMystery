#include <Entities/BipedEntity.h>

#include <bld_system.h>
#include "bld_misc_funcs.h"


/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x004D6B8A
* VC++ mangling:          ?GetEventTime@B_BipedAction@@QAENPBD@Z
*/
// Description: original +0xD6B8A returns the first stored float phase for
// an event ID above the strict float threshold 0xB58637BD, promoted to double.
// Missing definition/match returns -1. A non-null definition interns names
// through GetEventIndex; the null-definition branch leaves the namespace alone.
// The x87 helper also accepts unordered (NaN) phases. It bounds by phase count,
// not the separate ID count, and leaves action/definition storage unchanged.
// Origin: PROTOCOL_CPP (CODEX-RECON-002).
// Validation: PARTIAL_DIFFERENTIAL; 19 original/candidate bit-exact returns,
// namespace counts, read-only storage and x86 thiscall/ST0/RET 4 agree.
// Unknown: full preloaded namespace, non-null callback-table runtime, invalid
// pointers, inconsistent storage, OOM, FP status flags and unmasked FP traps.
// Evidence: CODEX-RECON-002 / CODEX_RECON_002_GET_EVENT_TIME_RESULTS_2026-10-22.md
double B_BipedAction::GetEventTime(const char *eventName)
{
    if (animationEvents == NULL)
        return -1.0;

    int eventIndex = gbl_events.GetEventIndex(eventName);
    for (unsigned int i = 0; i < animationEvents->numPhases; ++i)
    {
        float phase = animationEvents->phases[i];
        // FCOMP's C0 test accepts unordered too; a plain '>' loses NaN.
        if (!(phase <= -1.0e-6f) &&
            animationEvents->eventIndices[i] == eventIndex)
            return static_cast<double>(phase);
    }
    return -1.0;
}

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x004D741B
* VC++ mangling:          ??0B_BipedData@@QAE@ABVB_Name@@0@Z
*/
#ifndef BLD_NATIVE
B_BipedData::B_BipedData(const B_Name &bipedName, const B_Name &kind)
{
}
#endif

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x004D7990
* VC++ mangling:          ?GetNodePose@B_BipedData@@QAEABVB_Matrix@@PAVB_BipedAction@@MH@Z
*/
#ifndef BLD_NATIVE
const B_Matrix &B_BipedData::GetNodePose(
    B_BipedAction *action, float time, int nodeIndex)
{
    return *(new B_Matrix());
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x004D7A82
* VC++ mangling:          ?GetPose@B_BipedData@@QAEABVB_Matrix@@PAVB_BipedAction@@M@Z
*/
#ifndef BLD_NATIVE
const B_Matrix &B_BipedData::GetPose(B_BipedAction *action, float time)
{
    return *(new B_Matrix());
}
#endif

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/


/*
* Module:                 Blade.exe
* Entry point:            0x004D7D97
* VC++ mangling:          ?GetRightFootPose@B_BipedData@@QAEHPAVB_BipedAction@@MPAVB_Vector@@1@Z
*/
#ifndef BLD_NATIVE
int B_BipedData::GetRightFootPose(
    B_BipedAction *action, float time, B_Vector *, B_Vector *)
{
    return 0;
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x04D7E72
* VC++ mangling:          ?GetLeftFootPose@B_BipedData@@QAEHPAVB_BipedAction@@MPAVB_Vector@@1@Z
*/
#ifndef BLD_NATIVE
int B_BipedData::GetLeftFootPose(
    B_BipedAction *action, float time, B_Vector *, B_Vector *)
{
    return 0;
}
#endif

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x004DDB85
* VC++ mangling:          ?GetAngle@B_BipedEntity@@QAENXZ
*/
#ifndef BLD_NATIVE
double B_BipedEntity::GetAngle()
{
    return 0;
}
#endif

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x004E3AD9
* VC++ mangling:          ?DoAction@B_BipedEntity@@QAEXABVB_Name@@NHPAN@Z
*/
#ifndef BLD_NATIVE
void B_BipedEntity::DoAction(
    const B_Name &action_name, double unknown1, int iUnknown,
    double *dUnknown)
{
}
#endif

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x004E3B6A
* VC++ mangling:          ?DoActionWI@B_BipedEntity@@QAEXABVB_Name@@HNNPANN@Z
*/
#ifndef BLD_NATIVE
void B_BipedEntity::DoActionWI(
    const B_Name &actionName, int interpolationType, double time,
    double unknown1, double *dUnknown, double unknown2)
{
}
#endif

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x004E56DE
* VC++ mangling:          ?IsIncorrectPosition@B_BipedEntity@@QAEHABVB_Vector@@@Z
*/
#ifndef BLD_NATIVE
int B_BipedEntity::IsIncorrectPosition(const B_Vector &pos)
{
    return 0;
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x004E5798
* VC++ mangling:          ?unknown004E5798@B_BipedEntity@@QAEXABVB_Vector@@@Z
*/
#ifndef BLD_NATIVE
void B_BipedEntity::unknown004E5798(const B_Vector &pos)
{
}
#endif

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x004E5DA7
* VC++ mangling:          ?SetOnFloor@B_BipedEntity@@QAEHXZ
*/
#ifndef BLD_NATIVE
int B_BipedEntity::SetOnFloor()
{
    return 0;
}
#endif

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x004E73AF
* VC++ mangling:          ?RaiseEvent@B_BipedEntity@@QAEHH@Z
*/
#ifndef BLD_NATIVE
int B_BipedEntity::RaiseEvent(int eventIndex)
{
    return 0;
}
#endif

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x004E7E61
* VC++ mangling:          ?GetFloorHeight@B_BipedEntity@@QAENPAX0@Z
*/
#ifndef BLD_NATIVE
double B_BipedEntity::GetFloorHeight(void *unknown1, void *unknown2)
{
    return 0.0;
}
#endif

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

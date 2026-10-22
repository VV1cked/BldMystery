#include <Control/Events.h>

#include <Entities/Entity.h>
#include "bld_misc_funcs.h"
#include <BBLibc/BBLibc.h>


/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x0048A900
* VC++ mangling:          ??0B_EventFunc@@QAE@XZ
*/

B_EventFunc::B_EventFunc()
 : pyFunc(NULL), func(NULL), name(NULL)
{
}


/*
* Module:                 Blade.exe
* Entry point:            0x0048A92B
* VC++ mangling:          ??0B_EventFuncs@@QAE@ABVB_Name@@@Z
*/
// Description: original +0x8A92B calls B_NamedObj(name), then initializes
// the array at +0x0C to elements=NULL, size=0, increment=1, capacity=0.
// Origin: STUB (!BLD_NATIVE); native constructor retained.
// Validation: ORIGINAL_BEHAVIOR_IN_SCOPE; original static instructions and
// two original objects agree. C++ equivalence is NOT ESTABLISHED.
// Unknown: replacement vtable/lifetime parity: the original outer vtable
// dispatches cleanup through +0x8C300 -> +0x8A98D, absent from this class.
// Evidence: CODEX-RECON-001 / CODEX_RECON_001_RESULTS_2026-10-22.md
#ifndef BLD_NATIVE
B_EventFuncs::B_EventFuncs(const B_Name &name)
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
* Entry point:            0x0048AB7F
* VC++ mangling:          ?AddPyFunc@B_EventFuncs@@QAEXIPAUPyObject@@PBD@Z
*/
// Description: Blade.exe+0x8AB7F registers a Python event callback;
// repeated registration replaces the handler (refcount +1/-1 observed).
// Original static +0x8AB7F preserves the native func field, decrements the
// previous pyFunc before incrementing func, and borrows the name pointer.
// In particular, do not reorder the decref/incref for identical pointers.
// Origin: STUB (!BLD_NATIVE); original path remains native ASM.
// Validation: ORIGINAL_BEHAVIOR_IN_SCOPE (CODEX-001, H02 partial);
// NOT a differential validation of this empty C++ body.
// Unknown: initial/error and last-reference alias runtime behavior; native/
// Python coexistence has static proof, but no mixed runtime differential.
// Evidence: CODEX-RECON-001 / CODEX_RECON_001_RESULTS_2026-10-22.md
// Prior runtime: CODEX_001_R1_RESULTS_2026-10-19.md
#ifndef BLD_NATIVE
void B_EventFuncs::AddPyFunc(
    unsigned int eventIndex, PyObject *func, const char *name)
{
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x0048AEB2
* VC++ mangling:          ?AddFunc@B_EventFuncs@@QAEXIP6AHPAVB_Entity@@I@ZPBD@Z
*/

void B_EventFuncs::AddFunc(
    unsigned int eventIndex, EventFn func, const char *name)
{
    unsigned int i = eventIndex / NUM_EVENT_FUNCS;
    unsigned int j = eventIndex % NUM_EVENT_FUNCS;
    if (this->eventFuncs.size <= i)
    {
        unsigned int funcsIndex = this->eventFuncs.size;
        this->eventFuncs.Resize(i + 1, true);
        for (; funcsIndex < this->eventFuncs.size; funcsIndex++)
        {
            this->eventFuncs.elements[funcsIndex] = NULL;
        }
    }
    if (this->eventFuncs.elements[i] == NULL)
    {
        B_EventFunc *eventFuncs =
            this->eventFuncs.elements[i] = new B_EventFunc[NUM_EVENT_FUNCS];
        for (unsigned int index = 0; index < NUM_EVENT_FUNCS; index++)
        {
            eventFuncs[index].pyFunc = NULL;
            eventFuncs[index].func = NULL;
            eventFuncs[index].name = NULL;
        }
    }
    this->eventFuncs.elements[i][j].func = func;
    this->eventFuncs.elements[i][j].name = name;
}

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x0048B30B
* VC++ mangling:          ?CallFunc@B_EventFuncs@@QAEHPAVB_Entity@@I@Z
*/
// Description: original +0x8B30B returns 0 for a missing block/handler;
// Python takes precedence over native and receives entity/event names.
// The Python branch returns 1 after CallPythonObject even for a NULL result;
// the native branch propagates the callback's integer return unchanged.
// Origin: LEGACY_CPP.
// Validation: ORIGINAL_BEHAVIOR_IN_SCOPE (bounded original static dispatch);
// this C++ has NOT been differentially validated by CODEX-RECON-001.
// Unknown: runtime native/mixed dispatch, exceptions and reentrant mutation.
// Evidence: CODEX-RECON-001 / CODEX_RECON_001_RESULTS_2026-10-22.md

int B_EventFuncs::CallFunc(B_Entity *entity, unsigned int eventIndex)
{
    unsigned int i = eventIndex / NUM_EVENT_FUNCS;
    unsigned int j = eventIndex % NUM_EVENT_FUNCS;
    if (this->eventFuncs.size > i && this->eventFuncs.elements[i] != NULL)
    {
        if (this->eventFuncs.elements[i][j].pyFunc != NULL)
        {
            PyObject *args = PyTuple_New(2);
            PyObject *entityName = PyString_FromString(entity->name.String());
            PyObject *eventName = PyString_FromString(
                gbl_events.GetEventName(eventIndex));
            PyTuple_SET_ITEM(args, 0, entityName);
            PyTuple_SET_ITEM(args, 1, eventName);
            PyObject *result = CallPythonObject(
                this->eventFuncs.elements[i][j].pyFunc, args);
            Py_DECREF(args);
            Py_XDECREF(result);
            return true;
        }
        if (this->eventFuncs.elements[i][j].func != NULL)
            return this->eventFuncs.elements[i][j].func(entity, eventIndex);
        return false;
    }
    return false;
}


/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x0048B74A
* VC++ mangling:          ?GetEventIndex@B_Events@@QAEHPBD@Z
*/
// Description: original +0x8B74A uses a case-sensitive cached/hash lookup,
// creates a missing event with the next array index, then indexes it.
// Origin: STUB (!BLD_NATIVE); original path remains native ASM.
// Validation: ORIGINAL_BEHAVIOR_IN_SCOPE (original static lookup only);
// C++ equivalence is NOT ESTABLISHED.
// Unknown: NULL/OOM behavior and numeric runtime oracle. Original accesses
// buckets from this+0x1C and a cached event at this+0x1820; the current
// B_Events declaration contains only the leading pointer array.
// Evidence: CODEX-RECON-001 / CODEX_RECON_001_RESULTS_2026-10-22.md
#ifndef BLD_NATIVE
int B_Events::GetEventIndex(const char *event_name)
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
* Entry point:            0x0048BEED
* VC++ mangling:          ?GetEventName@B_Events@@QAEABVB_Name@@H@Z
*/

const B_Name &B_Events::GetEventName(int eventIndex)
{
    B_Event *event = events.elements[eventIndex];
    return event->name;
}


/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

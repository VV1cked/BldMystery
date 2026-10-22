#ifndef EVENTS_H

#define EVENTS_H

#include <array.h>
#include <BBLibc/Name.h>
#include <BBLibc/NamedObj.h>
#include <Hash.h>

#include <Control/EventFuncC.h>

struct PyObject;

class B_EventFunc
{
public:
    B_EventFunc();

    PyObject *pyFunc;
    EventFn func;
    const char *name;
};

#define NUM_EVENT_FUNCS 32

class B_EventFuncs : public B_NamedObj
{
public:
    B_EventFuncs(const B_Name &name);
    void AddPyFunc(unsigned int eventIndex, PyObject *func, const char *name);
    void AddFunc(unsigned int eventIndex, EventFn func, const char *name);
    int CallFunc(B_Entity *entity, unsigned int eventIndex);
private:
    array_t<B_EventFunc *> eventFuncs;
};

class B_Event : public B_NamedObj
{
public:
    B_Event() {}
    // Description: original lookup constructs the name before storing index.
    // Origin: PROTOCOL_CPP (bounded construction inside GetEventIndex).
    // Validation: PARTIAL_DIFFERENTIAL for observed names/indices; OOM unknown.
    // Evidence: CODEX-RECON-001 / CODEX_RECON_001_RESULTS_2026-10-22.md
    B_Event(const B_Name &name, int index) : B_NamedObj(name), index(index) {}
    int index;
};

class B_Events
{
public:
    B_Events();
    ~B_Events();
    int GetEventIndex(const char *event_name);
    const B_Name &GetEventName(int eventIndex);
private:
    // Description: original +8BF20 initializes a non-owning leading array
    // and a name hash at +0x18; +8C02A destroys hash-owned events first.
    // Origin: INTERFACE_ONLY (layout); lifetime support is bounded C++.
    // Validation: ORIGINAL_BEHAVIOR_IN_SCOPE; see the per-method tests.
    // Unknown: allocation-failure and externally corrupted container states.
    // Evidence: CODEX-RECON-001 / CODEX_RECON_001_RESULTS_2026-10-22.md
    B_SharedPtrArray<B_Event> events;
    B_Hash<B_Event> eventHash;
};

#endif /* EVENTS_H */

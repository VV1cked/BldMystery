
#include <bld_system.h>
#include <Raster/RasterDevice.h>
#include <BWorld/BWorld.h>
#include <BWorld/BMap.h>
#include <BWorld/light.h>
#include "bld_misc_funcs.h"
#include <BBLibc/BBLibc.h>
#include <BBLibc/MessageManager.h>
#include <BBLibc/Vararg.h>
#include <math.h>

// Description: Blade.exe+0x3B644 advances 1/64-s notification ticks;
// Slot passes follow pairs of ticks or a final unpaired tick.
// Origin: ASM_RETAINED. Validation: ORIGINAL_BEHAVIOR_IN_SCOPE (CODEX-002);
// not proof of a global fixed physics timestep.
// Report: CODEX_002_TIME_ARCHITECTURE_RESULTS_2026-10-19.md
extern "C" void WorldAdvanceToFrame(B_World *world, int frame);

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

/*
* Module:                 Blade.exe
* Entry point:            0x00439F5D
* VC++ mangling:          ?unknown_00439F5D@B_World@@QAEXXZ
*/

#ifndef BLD_NATIVE

void B_World::unknown_00439F5D()
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
* Entry point:            0x0043A6A4
* VC++ mangling:          ?GetTime@B_World@@QAENXZ
*/
double B_World::GetTime()
{
    // Original reads the stored double at 0x1884 directly; no clock query or scaling.
    return time;
}

/*
* Module:                 Blade.exe
* Entry point:            0x0043A6B8
* VC++ mangling:          ?SetTime@B_World@@QAEHN@Z
*/
#ifndef BLD_NATIVE
int B_World::SetTime(double time)
{
    this->time = time;
    // The original stores floor(time / frameDuration), including for negative times.
    currentFrame = (int)floor(time / frameDuration);
    return true;
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
* Entry point:            0x0043AC07
* VC++ mangling:          ?GoToTime@B_World@@QAEXN@Z
*/
#ifndef BLD_NATIVE
void B_World::GoToTime(double time)
{
    int frame = (int)floor(time / frameDuration);
    // Keep the verified frame-step engine intact: it updates clocks, slots,
    // renderer state and scheduled callbacks, including its legacy stack quirk.
    WorldAdvanceToFrame(this, frame);
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
* Entry point:            0x0043B8E6
* VC++ mangling:          ?AddScheduledFunc@B_World@@QAEXNPAUPyObject@@0PBD@Z
*/
void B_World::AddScheduledFunc(
    double time, PyObject *func, PyObject *args, const char *name)
{
    if (!PyCallable_Check(func))
    {
        mout << vararg("Warning: AddScheduledFunc() called for %s with uncallable function.\n", name);
        return;
    }
    // Original appends without sorting or replacing equal names. The record
    // retains func and args; args is not validated here.
    scheduledFuncs.Append(new B_ScheduledFunc(name, time, func, args));
}

/*
* Module:                 Blade.exe
* Internal entry point:   0x0043BE95
*/
static int CallScheduledNativeCallback(B_ScheduledFunc *scheduled)
{
    int result;
    // The original keeps the record in ECX, pushes its stored argument, then
    // caller-cleans that stack slot after the indirect call.
    __asm
    {
        mov eax, scheduled
        mov ecx, eax
        push dword ptr [eax + 020h]
        call dword ptr [eax + 01Ch]
        add esp, 4
        mov result, eax
    }
    return result;
}

void B_World::ProcessScheduledFuncs(double time)
{
    unsigned int index = 0;
    while (index < scheduledFuncs.size)
    {
        B_ScheduledFunc *scheduled = scheduledFuncs[index];
        // The original x87 compare treats unordered (NaN) values as due.
        // It also uses strict less-than, so an equal-time callback waits.
        if (scheduled->time >= time)
        {
            ++index;
            continue;
        }

        // The record leaves the array before the callback, but stays alive
        // until afterwards. Callbacks may safely change the scheduled list.
        scheduledFuncs.Remove(index, false);
        if (scheduled->func)
        {
            PyObject *result = CallPythonObject(scheduled->func, scheduled->args);
            Py_XDECREF(result);
        }
        else if (scheduled->unknown1C)
        {
            CallScheduledNativeCallback(scheduled);
        }
        delete scheduled;
    }
}

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/


/*
* Module:                 Blade.exe
* Entry point:            0x0043C5FD
* VC++ mangling:          ?Update@B_World@@QAEXPAVB_ViewPose@@PAXNH@Z
*/
// Description: Blade.exe+0x3C5FD calls GoToTime, one Frame pass and
// conditional renderer virtual+0xF0 plus map facade +0x31B8B.
// Origin: STUB (!BLD_NATIVE); real execution remains original ASM.
// Validation: ORIGINAL_BEHAVIOR_IN_SCOPE (CODEX-005), native paths only;
// no validation of the empty C++ body. Unknown: inner map emission.
// Report: CODEX_005_FRAME_PRESENTATION_RESULTS_2026-10-20.md
// Description: map +0x31B8B first calls +0x4A1AC at +0x31C71 on the
// observed this+0x28==0 path; its global-list dispatch precedes list reset.
// Origin: ASM_RETAINED for this map path; Update C++ remains a stub.
// Validation: ORIGINAL_BEHAVIOR_IN_SCOPE, 24 original call/return pairs;
// C++ equivalence not established. Unknown: callback command emission.
// Evidence: CODEX-006 / OBS-0060 /
// CODEX_006_MAP_FACADE_RENDER_EDGE_RESULTS_2026-10-20.md
#ifndef BLD_NATIVE
void B_World::Update(
    B_ViewPose *viewPose, void *cameraView, double time, int updateRaster)
{
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x0043C8B2
* VC++ mangling:          ?SetSun@B_World@@QAEXHABVB_Vector@@@Z
*/

#ifndef BLD_NATIVE

void B_World::SetSun(int exists, const B_Vector &position)
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
* Entry point:            0x0043D434
* VC++ mangling:          ?GetEnemiesVisibleFrom@B_World@@QAEXABVB_Vector@@N0NPAV?$B_PtrArray@VB_Entity@@@@@Z
*/
#ifndef BLD_NATIVE
void B_World::GetEnemiesVisibleFrom(
    const B_Vector &position, double radius,
    const B_Vector &direction, double angle, B_PtrArray<B_Entity> *enemies)
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
* Entry point:            0x0043DBBE
* VC++ mangling:          ?FindSectorIndex@B_World@@QAEHABVB_Vector@@@Z
*/

#ifndef BLD_NATIVE

int B_World::FindSectorIndex(const B_Vector &point)
{
    return -1;
}

#endif

/*
* Module:                 Blade.exe
* Entry point:            0x0043DC20
* VC++ mangling:          ?IntersectSegment@B_World@@QAEHABVB_Vector@@00HHPAX@Z
*/
#ifndef BLD_NATIVE
int B_World::IntersectSegment(
    const B_Vector &startPoint, const B_Vector &endPoint,
    const B_Vector &intersection, int flags, int, void *)
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
* Entry point:            0x0043E8E8
* VC++ mangling:          ?GenerateEntityName@B_World@@QAEPBDPBD@Z
*/
#ifndef BLD_NATIVE
const char *B_World::GenerateEntityName(const char *prefix)
{
    return NULL;
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
* Entry point:            0x0043EF43
* VC++ mangling:          ?SetTriggerSectorFunc@B_World@@QAEHPBD0PAUPyObject@@@Z
*/
#ifndef BLD_NATIVE
int B_World::SetTriggerSectorFunc(
    const char *trigger_sector_name, const char *func_type,
    PyObject *func
)
{
    return 0;
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x0043F172
* VC++ mangling:          ?GetTriggerSectorFloorHeight@B_World@@QAEHPBD@Z
*/
#ifndef BLD_NATIVE
int B_World::GetTriggerSectorFloorHeight(const char *trigger_sector_name)
{
    return 0;
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x0043F209
* VC++ mangling:          ?GetTriggerSectorRoofHeight@B_World@@QAEHPBD@Z
*/
#ifndef BLD_NATIVE
int B_World::GetTriggerSectorRoofHeight(const char *trigger_sector_name)
{
    return 0;
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x0043F2A0
* VC++ mangling:          ?GetTriggerSectorGroup@B_World@@QAEPBDPBD@Z
*/
#ifndef BLD_NATIVE
const char *B_World::GetTriggerSectorGroup(const char *trigger_sector_name)
{
    return NULL;
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x0043F332
* VC++ mangling:          ?GetTriggerSectorPoints@B_World@@QAEPANPBD@Z
*/
#ifndef BLD_NATIVE
double *B_World::GetTriggerSectorPoints(const char *trigger_sector_name)
{
    return NULL;
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x0043F467
* VC++ mangling:          ?GetTriggerSectorFunc@B_World@@QAEPAUPyObject@@PBD0@Z
*/
#ifndef BLD_NATIVE
PyObject *B_World::GetTriggerSectorFunc(
    const char *trigger_sector_name, const char *func_type
)
{
    return NULL;
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x0043F56D
* VC++ mangling:          ?SetTriggerSectorData@B_World@@QAEHPBDPAUPyObject@@@Z
*/
#ifndef BLD_NATIVE
int B_World::SetTriggerSectorData(
    const char *trigger_sector_name, PyObject *data
)
{
    return 0;
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x0043F65D
* VC++ mangling:          ?GetTriggerSectorData@B_World@@QAEPAUPyObject@@PBD@Z
*/
#ifndef BLD_NATIVE
PyObject *B_World::GetTriggerSectorData(const char *trigger_sector_name)
{
    return NULL;
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
* Entry point:            0x000043F98A
* VC++ mangling:          ?LoadEntitiesData@B_World@@QAEHPBD@Z
*/

#ifndef BLD_NATIVE

int B_World::LoadEntitiesData(const char *filename)
{
    return 0;
}

#endif

/*
* Module:                 Blade.exe
* Entry point:            0x0043FD82
* VC++ mangling:          ?SaveEntitiesData@B_World@@QAEHPBD@Z
*/

#ifndef BLD_NATIVE

int B_World::SaveEntitiesData(const char *filename)
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
* Entry point:            0x0044008F
* VC++ mangling:          ?AddPoint@B_TriggerSector@@QAEXABVB_Vector@@H@Z
*/
#ifndef BLD_NATIVE
void B_TriggerSector::AddPoint(const B_Vector& point, int numPoints)
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
* Entry point:            0x00440A19
* VC++ mangling:          ?GetTriggerSectorName@B_World@@QAEPBDH@Z
*/
#ifndef BLD_NATIVE
const char *B_World::GetTriggerSectorName(int index)
{
    return NULL;
}
#endif

/*
* Module:                 Blade.exe
* Entry point:            0x00440A5D
* VC++ mangling:          ??5@YAAAVB_IDataFile@@AAV0@PAVB_World@@@Z
*/

B_IDataFile& operator >>(B_IDataFile& file, B_World *world)
{
        unsigned int num_lights;
        unsigned int i;

        world->unknown187C = 0;
        world->unknown1880 = 0;
        B_PtrArray<atmosphere_t> *atms = &world->atmospheres;
        if (atms->num_alloc != 0)
        {
                for (i = 0; i < atms->size; i++)
                {
                        atmosphere_t * atm = atms->elements[i];
                        delete atm;
                }
                delete(atms->elements);
                atms->elements = NULL;
                atms->size = 0;
                atms->num_alloc = 0;
        }
        file >> atms->size;
        atms->num_alloc = ((atms->increment + atms->size - 1) / atms->increment) * atms->increment;
        if (atms->num_alloc != 0)
        {
                atms->elements = new atmosphere_t * [atms->num_alloc];
                for(i = 0; i < atms->size; i++)
                {
                        atms->elements[i] = new atmosphere_t();
                        file >> atms->elements[i];
                }
        }

        if (B_3D_raster_device)
                B_3D_raster_device->unknown084();

        for(i = 0; i < atms->size; i++)
        {
                atmosphere_t * atm = (atmosphere_t *)atms->elements[i];
                B_3D_raster_device->AddAtmosphere(atm->Id(), atm->color, atm->intensity);
        }

        file >> gbl_world_points;
        file >> world->map;

        B_PtrArray<light_t> *lights = &world->lights;
        if (lights->num_alloc != 0)
        {
                for (i = 0; i < lights->size; i++)
                {
                        light_t * light = (light_t *)lights->elements[i];
                        delete light;
                }
                delete(lights->elements);
                lights->elements = NULL;
                lights->size = 0;
                lights->num_alloc = 0;
        }

        file >> num_lights;

        for(i = 0; i < num_lights; i++)
        {
                light_t *light = read_light(&file);
                B_PtrArray<light_t> *lights = &world->lights;
                if (lights->num_alloc <= lights->size)
                {
                        lights->num_alloc += lights->increment;
                        if (lights->size != 0)
                        {
                                light_t **elements = new light_t *[lights->num_alloc];
                                for(unsigned int j = 0; j < lights->size; j++)
                                {
                                        elements[j] = lights->elements[j]; 
                                }
                                delete lights->elements;
                                lights->elements = elements;
                        }
                        else
                        {
                                lights->elements = new light_t *[lights->num_alloc];
                        }
                }
                lights->elements[i] = light;
                lights->size++;
        }

        for(i = 0; i < num_lights; i++)
        {
                light_t *light = world->lights.elements[i];
                light->unknown008();
        }

        file >> world->initial_point_position >> world->initial_point_orientation;

        for(i = 0; i < world->map.size; i++)
        {
                B_Sector *sector = world->map.elements[i];
                file >> sector->groupId;
        }

        world->octTree.Build(world->map, 5000.0);

        return file;
}

/*
................................................................................
................................................................................
................................................................................
................................................................................
*/

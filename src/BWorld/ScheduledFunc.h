#ifndef B_SCHEDULED_FUNC_H
#define B_SCHEDULED_FUNC_H

#include <BBLibc/NamedObj.h>
#include <bld_python.h>

class B_ScheduledFunc : public B_NamedObj
{
public:
    // Original Python constructor: Blade.exe 0x004396B3.
    B_ScheduledFunc(const char *name, double time, PyObject *func, PyObject *args)
        : B_NamedObj(B_Name(name)), time(time), func(func), args(args),
          unknown1C(0), unknown20(0)
    {
        Py_XINCREF(func);
        Py_XINCREF(args);
    }

    // Original native-callback constructor: Blade.exe 0x00439750.
    B_ScheduledFunc(
        const char *name, double time,
        unsigned int callbackAddress, unsigned int callbackParam)
        : B_NamedObj(B_Name(name)), time(time), func(NULL), args(NULL),
          unknown1C(callbackAddress), unknown20(callbackParam)
    {
        // The scheduler calls callbackAddress with this record and
        // callbackParam when func is NULL; the callback's return is ignored.
    }

    // Original destructor: Blade.exe 0x004397C3.
    virtual ~B_ScheduledFunc()
    {
        // Original gates BOTH decrements on func, but does not check args.
        // Thus a null func leaves args untouched; non-null func requires args.
        if (func)
        {
            Py_DECREF(func);
            Py_DECREF(args);
        }
    }

    double time;
    PyObject *func;
    PyObject *args;
    unsigned int unknown1C;
    unsigned int unknown20;
};

#endif

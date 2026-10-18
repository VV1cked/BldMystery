#ifndef IMPULSE_BODY_H
#define IMPULSE_BODY_H
class B_Vector;
// Native physics-body view; objects and their vtables remain owned by ASM.
class B_ImpulseBody {
public:
    void ApplyImpulse(const B_Vector &point, const B_Vector &impulse);
};
#endif

#include <Entities/ImpulseBody.h>
#include <AnalyticGeometry/Vector.h>
namespace {
template<class T> T &BodyField(void *body,unsigned offset) {
 return *reinterpret_cast<T *>(static_cast<char *>(body)+offset);
}
}
/*
* Module:                 Blade.exe
* Entry point:            0x004ABC53
* VC++ mangling:          ?ApplyImpulse@B_ImpulseBody@@QAEXABVB_Vector@@0@Z
*/
void B_ImpulseBody::ApplyImpulse(const B_Vector &point,const B_Vector &impulse) {
 double mass=BodyField<double>(this,0x168);
 // Original divides even for zero/NaN mass. Preserve IEEE results.
 B_Vector delta(impulse.x/mass,impulse.y/mass,impulse.z/mass);
 B_Vector &linear=BodyField<B_Vector>(this,4);
 linear.x+=delta.x; linear.y+=delta.y; linear.z+=delta.z;
 // Read inputs after the linear update, matching original aliasing order.
 const B_Vector &centre=BodyField<B_Vector>(this,0x170);
 B_Vector r(point.x-centre.x,point.y-centre.y,point.z-centre.z);
 B_Vector torque(r.y*impulse.z-r.z*impulse.y,
                 r.z*impulse.x-r.x*impulse.z,
                 r.x*impulse.y-r.y*impulse.x);
 // Matrix comes from the object's inertial setup, orientation, mass and scale.
 double *m=&BodyField<double>(this,0xE8);
 B_Vector angularDelta((torque.x*m[0]+torque.y*m[4])+torque.z*m[8],
                       (torque.x*m[1]+torque.y*m[5])+torque.z*m[9],
                       (torque.x*m[2]+torque.y*m[6])+torque.z*m[10]);
 B_Vector &angular=BodyField<B_Vector>(this,0x1C);
 angular.x+=angularDelta.x; angular.y+=angularDelta.y; angular.z+=angularDelta.z;
}

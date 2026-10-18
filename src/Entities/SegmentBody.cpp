#include <Entities/SegmentBody.h>
#include <AnalyticGeometry/Vector.h>
#include <cmath>
namespace {
template<class T>T &SegmentField(void *p,unsigned offset){return *reinterpret_cast<T *>(static_cast<char *>(p)+offset);}
bool X87Less(double a,double b){return a<b || a!=a || b!=b;}
}
/*
* Module:                 Blade.exe
* Entry point:            0x004B34AF
* VC++ mangling:          ?Intersect@B_SegmentBody@@QAEHABVB_Vector@@0AAV2@@Z
*/
int B_SegmentBody::Intersect(const B_Vector &start,const B_Vector &end,B_Vector &contact){
 if(SegmentField<int>(this,0x58)==1)return 0;
 double t=0;
 Refresh();
 void *bounds=SegmentField<void *>(this,0x70);
 const B_Vector &lo=SegmentField<B_Vector>(bounds,0x2C);
 const B_Vector &hi=SegmentField<B_Vector>(bounds,0x44);
 // Original rejects both endpoints on either boundary plane.
 if(!(start.x>lo.x)&&!(end.x>lo.x))return 0;
 if(!(start.y>lo.y)&&!(end.y>lo.y))return 0;
 if(!(start.z>lo.z)&&!(end.z>lo.z))return 0;
 // x87 C0 treats unordered like less-than in these upper-bound branches.
 if(!X87Less(start.x,hi.x)&&!X87Less(end.x,hi.x))return 0;
 if(!X87Less(start.y,hi.y)&&!X87Less(end.y,hi.y))return 0;
 if(!X87Less(start.z,hi.z)&&!X87Less(end.z,hi.z))return 0;
 void *resource=SegmentField<void *>(this,0x74);
 if(!SegmentField<int>(resource,0xB0)&&!SegmentField<int>(resource,0xB4))return 0;
 TraceGeometry(start,end,static_cast<char *>(resource)+0xA4,&t);
 // No upper clamp. Equality and unordered fail this original strict test.
 if(!(t>0))return 0;
 B_Vector first(start.x*t,start.y*t,start.z*t);
 B_Vector second(end.x*(1-t),end.y*(1-t),end.z*(1-t));
 contact=B_Vector(first.x+second.x,first.y+second.y,first.z+second.z);
 return 1;
}
#ifndef BLD_NATIVE
// Non-native builds retain the project's placeholder geometry backend.
void B_SegmentBody::Refresh() {}
void B_SegmentBody::TraceGeometry(const B_Vector &,const B_Vector &,void *,double *) {}
#endif

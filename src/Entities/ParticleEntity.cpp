#include <Entities/ParticleEntity.h>
#include <AnalyticGeometry/Vector.h>
#include <BBLibc/Name.h>
#include <BWorld/BWorld.h>
#include <cstring>
static_assert(sizeof(B_Entity)==0x1A0,"Particle base layout");
static_assert(sizeof(B_Vector)==0x18,"Particle vector layout");
namespace {
template<class T> T &ParticleField(void *object, unsigned offset) {
    return *reinterpret_cast<T *>(static_cast<char *>(object)+offset);
}
void ParticlePin(B_Entity *entity) {
    typedef void (__thiscall *Subscribe)(B_Entity *, const B_Name &);
    B_Name pin("Pin");
    void **table=*reinterpret_cast<void ***>(entity);
    reinterpret_cast<Subscribe>(table[0x8C/4])(entity,pin);
}
}
/*
* Module:                 Blade.exe
* Entry point:            0x00530CFA
* VC++ mangling:          ?unknown_method098@B_ParticleEntity@@UAEXN@Z
*/
void B_ParticleEntity::unknown_method098(double time) {
    // Original uses strict >: equality and unordered continue movement.
    if (time > ParticleField<double>(this,0x260)) { ParticlePin(this); return; }
    double dt=time-ParticleField<double>(this,0x268);
    B_Vector &velocity=ParticleField<B_Vector>(this,0x220);
    const B_Vector &acceleration=ParticleField<B_Vector>(this,0x238);
    double damping=ParticleField<double>(this,0x250);
    B_Vector start=ParticleField<B_Vector>(this,0x200);
    B_Vector next;
    double *p=&next.x, *v=&velocity.x;
    const double *a=&acceleration.x, *old=&start.x;
    for(int i=0;i<3;++i) {
        double half=0.5*a[i];
        double displacement=(half*dt)*dt;
        p[i]=(old[i]+v[i]*dt)+displacement;
    }
    for(int j=0;j<3;++j) {
        double drag=(damping*v[j])*dt;
        v[j]+=a[j]*dt-drag;
    }
    // Timestamp and velocity change before the world query and hit callback.
    ParticleField<double>(this,0x268)=time;
    B_Vector point;
    struct HitRecord { unsigned first,second; B_Entity *entity; } hit={0,0,0};
    if(B_world.IntersectSegment(start,next,point,8,
            ParticleField<int>(this,0x270),&hit)) {
        B_Vector zero1,zero2;
        typedef void (__thiscall *OnHit)(B_Entity *,B_Entity *,
            const B_Vector &,const B_Vector &,const B_Vector &,const B_Vector &);
        void **table=*reinterpret_cast<void ***>(this);
        reinterpret_cast<OnHit>(table[0xAC/4])(this,hit.entity,point,velocity,zero1,zero2);
        // Original callback sees the old pose; position changes afterwards.
        std::memcpy(static_cast<char *>(static_cast<void *>(this))+0x200,&point,sizeof(point));
        ParticlePin(this);
    } else {
        std::memcpy(static_cast<char *>(static_cast<void *>(this))+0x200,&next,sizeof(next));
    }
}

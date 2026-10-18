#ifndef SEGMENT_BODY_H
#define SEGMENT_BODY_H
class B_Vector;
class B_SegmentBody {
public:
 int Intersect(const B_Vector &start,const B_Vector &end,B_Vector &contact);
 // Native helpers retained until their independent reconstruction.
 void Refresh();
 void TraceGeometry(const B_Vector &,const B_Vector &,void *,double *);
};
#endif

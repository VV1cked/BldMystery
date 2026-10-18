.586
.model flat
option M510
option casemap:none
assume fs: nothing

image segment write execute PUBLIC
include procedures.inc
include variables.inc
include export.inc
include import.inc
include native.asm
public _WorldAdvanceToFrame
_WorldAdvanceToFrame proc
           mov ecx, dword ptr [esp+004h]
           mov eax, dword ptr [esp+008h]
           push eax
           call l0043B644
           ret
_WorldAdvanceToFrame endp
; Segment intersection keeps its native cache and geometry helpers.
public ?Refresh@B_SegmentBody@@QAEXXZ
?Refresh@B_SegmentBody@@QAEXXZ proc
 jmp l004AD0E7
?Refresh@B_SegmentBody@@QAEXXZ endp
public ?TraceGeometry@B_SegmentBody@@QAEXABVB_Vector@@0PAXPAN@Z
?TraceGeometry@B_SegmentBody@@QAEXABVB_Vector@@0PAXPAN@Z proc
 jmp l004B30C5
?TraceGeometry@B_SegmentBody@@QAEXABVB_Vector@@0PAXPAN@Z endp
image ends
.data?
include uninitialised.asm

END __startup

#ifndef POSE_CACHE_VIEW_H
#define POSE_CACHE_VIEW_H

class B_Matrix;

// Borrowed partial view of an existing native animator. Never allocate this
// type: it describes neither the full owner layout nor its lifetime.
class B_PoseCacheView
{
public:
    B_Matrix *RootMatrix();
};

// Description: original Blade.exe+64C70 composes parent before child and
// returns a borrowed 0x14-byte node descriptor, including the unbound path.
// Origin: ASM_RETAINED; no NodePose C++ replacement is provided here.
// Validation: ORIGINAL_BEHAVIOR_IN_SCOPE; 561 producer cases and three
// native-constructor unbound calls; warm quaternion/matrix scratch measured.
// C++ equivalence NOT ESTABLISHED. Unknown: cold lazy-init/cleanup equivalence,
// helper bridges and independent production candidate comparison.
// Evidence: CODEX-RECON-009 / CODEX_RECON_009_POSE_CORE_RESULTS_2026-10-24.md

// Description: original Blade.exe+652DA evaluates phase/mode/correction state.
// Origin: ASM_RETAINED.
// Validation: ORIGINAL_BEHAVIOR_IN_SCOPE; modes0/1/2, flags0..7 on the query
// fixture; C++ equivalence NOT ESTABLISHED.
// Unknown: +622CF blended-mode2 receiver/context, complete evaluator branches.
// Evidence: CODEX-RECON-009 / CODEX_RECON_009_POSE_CORE_RESULTS_2026-10-24.md

#endif

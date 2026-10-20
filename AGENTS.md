# Agent instructions — BldMystery

These instructions apply to the entire repository. They complement the existing
ASM-to-C++ reconstruction method described in `README.md`.

## Mandatory reconstruction sequence

For **every** candidate original function or method, perform the following
steps **in order**:

1. **Establish behavior and boundaries before implementation.** Identify the
   exact original binary (module, SHA-256, image base, VA/RVA), calling
   convention and arguments as far as verified, observable inputs and outputs,
   side effects, branch conditions, object lifetime and interactions. Separate
   original binary/runtime evidence from reconstructed C++ and hypotheses.
   Do not infer a whole method from a single observed branch.
2. **Document the identified behavior directly beside the existing source
   declaration or implementation.** Add a short `Description` comment with
   `module+RVA`, experimental evidence (CODEX/EXP/OBS or test), confirmed
   scope, and unresolved paths. For ASM-only routines, annotate the nearest
   relevant declaration, wrapper or call site; retain the original entry
   address. **Do not rename existing unknown methods, fields or symbols, or
   change signatures, vtable layout or ABI at this stage.**
3. **Prepare differential tests against the original executable/DLL.** Cover
   the verified normal path plus distinguishing branches, boundaries and
   side effects (including timing, callback order and reference ownership when
   relevant). Record the original-binary identity and the comparison
   tolerances. A passing test of reconstructed C++ alone is not evidence
   of fidelity to the original. If the original behavior or oracle cannot be
   tested adequately, keep the routine as a candidate rather than replacing
   its ASM.
4. **Only then replace the bounded ASM routine with C++.** Preserve the
   original ABI and behavior, including observed quirks; use the repository's
   existing ASM replacement/build workflow. Make the smallest independently
   testable change, run focused differential tests and available regressions,
   and report outcomes and any remaining limitations.

**Do not skip or reverse the sequence.** Discovery or description is not
permission to implement; implementation is not permission to remove native
fallback before tests are ready. Large composite routines must be decomposed
into verified, replaceable parts instead of rewritten speculatively.

## Provenance and validation of reconstructed code

Treat two properties as **independent, per-method/per-branch attributes**:

- `Origin`: `LEGACY_CPP` for C++ recovered before this policy or without
  evidence that its full workflow was followed; `PROTOCOL_CPP` only when
  the specific ASM-to-C++ implementation change followed all four mandatory
  steps; `ASM_RETAINED`, `STUB`, or `INTERFACE_ONLY` otherwise.
- `Validation`: `NOT_AUDITED`; `ORIGINAL_BEHAVIOR_IN_SCOPE` (original
  behavior tested, C++ equivalence not established);
  `PARTIAL_DIFFERENTIAL` (some C++ vs exact original comparisons pass);
  `DIFFERENTIAL_IN_SCOPE` (documented tests establish the claimed C++
  equivalence for the declared scope). These are not global guarantees.

**Default old C++ to `LEGACY_CPP / NOT_AUDITED`**, unless specific evidence
supports a stronger validation status. A legacy function may earn
`PARTIAL_DIFFERENTIAL` or `DIFFERENTIAL_IN_SCOPE` after testing but stays
`LEGACY_CPP`: documentation, a new comment, a recent commit date or
revalidation must **never silently relabel its origin as PROTOCOL_CPP**.
Similarly, `confirmed_in_scope` for an architecture hypothesis validates an
original-engine observation, *not* the fidelity of reconstructed C++.

### Mandatory in-source validation conclusion

Whenever adding or revising a method description, place a concise validation
conclusion at the method/declaration/call site. Include what was observed,
the scope, what remains unknown, and a neutral report identifier (e.g.
`Evidence: CODEX-005 / CODEX_005_FRAME_PRESENTATION_RESULTS_2026-10-20.md`).
Never include URLs, repository names, internal paths or commit SHAs referring
to non-public research infrastructure. A report identifier is sufficient
for matching the evidence in separately maintained private records.

Example:

```cpp
// Description: Original <module>+<RVA> performs <identified behavior>.
// Origin: LEGACY_CPP
// Validation: ORIGINAL_BEHAVIOR_IN_SCOPE; original branch X observed,
//   not a differential validation of this C++ implementation.
// Scope: <scene/inputs/original SHA>. Unknown: branch Y, ABI edge cases.
// Evidence: CODEX-005 / CODEX_005_FRAME_PRESENTATION_RESULTS_2026-10-20.md
```

For a validated C++ replacement, specify the **actual differential test
name, count/tolerance/result** and the report containing them. If there was
no differential comparison, state that explicitly; never call original
runtime tracing alone a C++ fidelity test. Keep these comments factual
and concise; do not alter names, ABI, or runtime behavior merely to annotate.

## Evidence and change discipline

- Consult authorized external research reports when available, but never
  disclose their repository names, URLs, filesystem paths or access details
  in this public repository. Keep raw traces, dumps and binaries out of Git.
- Clearly label `Observed`, `Inferred`, and `Unknown` in code comments.
  An experimental `confirmed_in_scope` claim is not a global property of
  the engine.
- Keep verified original `module+RVA` addresses distinct from reconstructed
  labels and from addresses of renderer workaround DLLs; never mix DLL RVAs.
- Do not change names, calling conventions, public interfaces, struct offsets
  or vtable slots while merely documenting discoveries. Any later
  refactoring requires its own explicit decision and compatibility evidence.
- For each implementation change, cite the supporting evidence, affected
  function addresses and differential tests in the commit description.

## Public repository information boundary

Before any public-fork commit, inspect the entire diff (including code comments,
AGENTS.md, tests, documentation and commit message). Do not publish direct
links or identifying paths to private research systems. Use only neutral
CODEX/EXP/OBS identifiers and report basenames in source comments. Detailed
report locations and revision links belong in the private documentation only.

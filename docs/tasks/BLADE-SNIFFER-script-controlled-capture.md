# BLADE-SNIFFER — Script-Controlled Capture Windows

**Type:** standalone test-stand infrastructure task  
**Priority:** high  
**Dependency on RECON-05:** none  
**Deliverable:** working implementation, tests, documentation, commit

## Goal

Enable the Blade of Darkness test stand to begin and end event capture from in-game Python 1.5 scripts, rather than logging every intercepted event from engine startup. Preserve the existing startup/continuous mode.

**Principle:** instrumentation lifetime != capture lifetime. Installing hooks, entering their callbacks, processing an event and persisting its trace are different stages. Do not needlessly reinstall hooks at each capture boundary.

## 1. Investigate the existing implementation first

Locate the current BladeEx sniffer, installation of hooks, event ingestion and buffering, output serialization, configuration, and any existing Python-facing bridge. Determine which thread(s) invoke callbacks and how a captured event is assigned a timestamp/sequence/frame ID. Reuse existing components and naming. Do not assume particular filenames or classes.

Record the relevant code paths and the selected low-impact integration point in the final report. The solution should suppress costly event construction/formatting while capture is inactive, where it is safe to do so; simply disabling file output is insufficient if expensive work continues upstream.

## 2. Required behavior

Support two capture policies:

- **Continuous (legacy):** preserve present capture-from-start behavior and existing configs, including the current default unless there is a strong compatibility reason otherwise.
- **Script-triggered:** hooks may be installed at startup but no scenario events are recorded until a script signals capture start.

Expose a minimal Python 1.5-compatible control API, preferably via existing bindings:

```python
Sniffer.Begin("entity-test")
Sniffer.Mark("action-complete")
Sniffer.End()
```

The names are suggested, not an instruction to implement a parallel new Python subsystem. If the current API naturally demands different names, document the actual calls. Calls must not stop gameplay or reinitialize the engine. Permit multiple sequential sessions during one game process.

Optional only if naturally supported: `Pause()`, `Resume()`. No need for nested sessions. A second `Begin` during active capture must produce an explicit, safely handled diagnostic; an `End` without an active capture must not corrupt state and should be diagnosed.

Keep a central logical capture controller; implement only the states needed by the actual architecture. Each session must have a distinct ID, scenario name, start/end metadata, end reason, sequential markers, and trace-file association. Reuse existing clock/frame/event metadata where possible.

## 3. Exact capture boundaries and buffers

Capture membership must be established **when the event is intercepted**, not when a buffered or asynchronously queued event is written to disk. Preserve that session ID through the queue. Define and document the linearization point for Begin/End under multiple threads and what ordering guarantees can actually be made.

Prevent events before Begin or after End being assigned to that session, and prevent residual buffered events from one session appearing in the next. Ensure flushing/finalization at End and safe shutdown. If events can be in flight when End executes, specify and test the policy rather than silently discarding or relabeling them.

Guard against runaway output using existing limits (number of events, bytes, timeout) where available. Do not introduce a heavy watchdog unless required. Make recording failure and unexpected shutdown diagnosable.

## 4. Script use

Supply a minimal Python 1.5-compatible example (adapt to the real test scripts):

```python
def TestScenario():
    PrepareWorld()
    Sniffer.Begin("entity-test")
    try:
        PerformTestAction()
        Sniffer.Mark("action-complete")
    finally:
        Sniffer.End()
```

Do not globally instrument Python, rewrite its import system, introduce game-side dependencies, or bulk-modify original scripts. The intended breakpoint is a non-blocking *capture trigger*, not a debugger stop.

## 5. Testing and evidence

Test, at minimum:

| Case | Expected |
| --- | --- |
| Legacy continuous startup | Existing behavior preserved |
| Triggered startup | No scenario events before Begin |
| Begin → event → End | Event belongs to session |
| Event before Begin / after End | Not in session |
| Begin → Mark → End | Marker recorded in order |
| Two sequential sessions | Distinct metadata and no mixed events |
| Begin while capturing | Explicit safe error |
| End without Begin | Safe diagnostic |
| Script exception with finally | End executed |
| Shutdown / output limit / I/O error | Clean handling and diagnostic |
| Concurrent producers, if relevant | No session corruption or reassignment |

Add host-side automated tests without the actual game where possible. Run a game integration test when the environment permits. Clearly distinguish **PASS**, **FAIL**, **NOT RUN**; never claim an actual-game run that did not happen.

If possible compare the same scenario in continuous and script-triggered modes: emitted event count, trace size, and overhead when inactive. Do not invent benchmark numbers.

## 6. Documentation and reporting

Update the stand's documentation with configuration, exact Python API, sample use, session metadata, capture-boundary guarantees, known threading caveats, and error recovery. Existing rules for source-level validation comments remain in force. Never introduce links to private repositories in publicly committed comments.

Final Codex report must contain: starting architecture; changed files; integration rationale; exact Python API; test table with environment and status; measured comparison or why unavailable; remaining limitations; commit SHA.

## 7. Scope boundaries

Do **not**: recover additional BladeEx methods; change original game logic; redesign hooking; build a full Python debugger; add interactive game-stopping breakpoints or external orchestration server; bulk-edit scripts; alter RECON-05.

Future ideas—capture the next N frames, a ring buffer containing pre-trigger events, and interactive pauses—may be noted but are not required in this task.

## Acceptance criteria

- Legacy continuous capture remains operational.
- Script-triggered capture begins/ends through Python 1.5 without restarting the game.
- Sequential sessions remain isolated and correctly identified even with buffered output.
- Out-of-window events are excluded; marks are retained.
- Invalid operations and recording errors are handled safely.
- Tests and documentation reflect the implementation.
- Codex commits the result and reports the commit SHA.

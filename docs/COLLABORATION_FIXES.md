# Reliability and layer-design work (2026-11-11)

This records the GS-06-R1 reliability work. The later editor/background and mask
clarity correction is recorded in [HEADER_MASK_CLARITY.md](HEADER_MASK_CLARITY.md).
Results below apply to R1 and must not be read as validation of later code.

## Collaboration contract

Codex implements and runs local checks. Claude reviews individual decisions using
bounded code excerpts, rather than independently auditing the entire repository.
The owner explicitly authorized Claude to read the same files as Codex. Neither
agent may silently proceed past an unexplained failure or change live hardware,
services or scheduled tasks as part of a software-only test.

## Current status

| Item | Evidence | Status |
| --- | --- | --- |
| Offline device reconnect | A two-endpoint regression opened the healthy endpoint four times before the fix and once after it. Reappearance and isolated endpoint faults pass with fake transports. | Implemented; focused Release tests pass. |
| Recovery during continuous editing | Old debounce failed the continuous-edit regression. The bounded window now passes continuous edits, invalid save target, retry, clean state and undo tests. | Implemented; focused Release tests pass. |
| Startup/module documentation | Startup paths describe `LightingStudio-ReliabilityFix`; the protected module list correctly describes six DLLs. | Updated; package CRC, byte equality and SDK-free inspection pass. |
| Header logo | Full-image fit replaces crop/fade. Painted-bound tests failed before and pass after the fix at minimum, normal and wide widths. | Implemented; native startup screenshot visually inspected with the complete logo. |
| SignalRGB stop | Protected process metadata fails with error 5; SCM rights permit administrator Stop. Dedicated named-service UAC action reads the actual configured path. No matching scheduled task was found in the read-only task query. | Implemented; startup gate passes after correcting a fake-contract mismatch. Real UAC stop untested. |
| Layer model | Creation rules mix containers, child sources and standalone source roots. The [proposal](LAYER_MODEL_PLAN.md) separates these roles and defines blending/selection/migration. Neutral-container render comparisons pass at 0/0.37/2/86400 seconds. | Plan only; no schema or creation-rule change. |

## Decisions and validation boundaries

- Reconnect keeps Starting/Running schedulers with the same output scope and
  retries only missing scopes. A global scan, guard or preparation exception still
  stops all connections. Worker faults only replace the affected connection.
- Release baseline build and the existing startup/service CTest gates passed.
- Software tests use fake transports. No physical RGB write or live process/service
  stop has been performed for this work.
- Initial broad Claude jobs timed out. Subsequent collaboration uses short,
  single-decision prompts with inline excerpts and no repository-wide searches.
- Claude reviewed recovery scheduling, logo rendering/testing, the SignalRGB
  action design/native security boundary/UI state machine, and layer ownership.
  Missing-context concerns were checked against the actual callers rather than
  automatically accepted. Native pending-request rejection and UI exception
  handling already existed; combined rescan/stop errors now remain visible.
- Artificial paths such as `C:/rgb` appear only in fixtures. Valid-path tests also
  cover a custom directory on `D:` and the observed SCM configuration. Quoted paths,
  arguments, wrong names, unexpanded variables, relative/UNC/device paths and
  malformed input have explicit acceptance/rejection cases.

## Final software validation

- Complete Release build: passed (`build/multi`, Visual Studio developer shell).
- Full Release CTest run: 21/22 passed; the startup test double incorrectly
  returned an exit error while still pending. The native implementation does not
  do that. After correcting the fake and rebuilding, the startup gate passed
  (47.66 seconds). Thus all 22 gates have passing results for this product code;
  this was not a second all-green full-suite run.
- The full editor gate passed in 77.84 seconds. Recovery tests cover continuous
  edits, invalid parent path, visible failure/retry, clean state and undo.
- Native Windows header/startup run: 8 QtTest results passed, zero skips. It
  covers all three header sizes plus first-run, legacy toolbar and recorded owner
  window-state startup. Additional header runs cover 150% and 200% scaling.
- The native screenshot at `docs/images/reliability-header.png` was inspected:
  complete artwork and logo, centered with background on either side.
- Startup policy/UI tests cover valid custom paths, malformed identities, process
  mismatch, unsupported states, UAC/error codes, stable stop, restarted service,
  active-request refusal after timeout, cancellation and combined scan/stop errors.
- The Release documentation/tooling gate includes all 13 Python tests. Installer
  checks are WhatIf/planning checks, not an installed service update.
- Package ZIP CRC passed. Extracted editor/host bytes match the tested Release
  outputs. Packaged `--inspect` passed with only Windows directories on PATH and
  no SDK plugin path. No personal project is included.
- `git diff --check` passed. Graphify AST refresh completed without LLM use;
  parser partial-extraction warnings and stale semantic labels remain and do not
  constitute C++ compiler failures. No relabeling/API request was made.

## Package and scope limits

Portable executable: `build/dist/LightingStudio-ReliabilityFix/msi_mystic_light.exe`.
Build identity: `GS-06-R1`. `Start-Lighting.cmd` still prefers an installed service
editor when its `service.json` exists; no installed copy was replaced.

Archive: `build/dist/LightingStudio-ReliabilityFix-windows-x64.zip`.
SHA256: `f5b0a2cbb4d3ed16f8ad5791636f294aa8f03b9412c051f0c25460abe15d4b73`.
Editor SHA256: `2f0c0d27d0e62ea5a19550c516b9ca27c7f4cd14584a3b360578aa02d3f1ef52`.

Not checked: an actual UAC-approved SignalRGB stop, real reboot/vendor task
restart behavior, physical RGB output, the installed SYSTEM service and a full
Debug run. No live process or service was stopped. Portable normal startup can
automatically write configured hardware; software-only checks used `--inspect`
or hardware-free test windows. Committing or pushing this source does not install
the package or qualify real hardware/UAC behavior.

The owner still needs to review the layer proposal before its implementation.
Its legacy-migration and owner-interaction acceptance gates remain future work;
the neutral-container feasibility test does not complete them.

# Editor Fixes and Menu Cleanup Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Fix the owner-reported editor defects of 2026-11-17 and remove duplicated or ineffective menu entries, without the larger operating-concept redesign.

**Architecture:** Each task is a narrow change in the existing Qt widgets (`LightingWindow`, `ColorPicker`, `StudioWidgets`) or in `ServiceRuntime`, with a regression test in the existing QtTest suites. The operating-concept redesign (hardware cards per header, Component Lab, "functions where they act") is out of scope and gets its own spec via brainstorming first.

**Tech Stack:** C++20, Qt 6.8.3 Widgets, QtTest, CMake/Ninja (`build/build-r10.cmd`), MSVC 14.52.

**Spec:** The owner report of 2026-11-17 (this session): gradient stops stay gray after Apply; dragging a component no longer updates the lighting in real time; Start.cmd / portable version unclear; "Output: … active / … configured" duplicates the Devices button; File menu not usable in Devices; Edit entries without effect in Devices; "Add layer" exists twice with all submenus; the "+ with dropdown arrow" icon does not fit; the effect list is unsorted and needs categories.

## Global Constraints

- No duplicated entry points: every function has exactly one menu/button location (owner: "doppelt plazierte funktionen/einträge/menüs, alles weg").
- A function is disabled (or hidden) where it has no effect (owner: "Funktionen sollten nicht zur Verfügung stehen, an Stellen, an denen sie keinen Sinn bzw Auswirkung haben").
- File > Open/Save/Save as must work in every workspace (owner: "Speichern möchte man doch, egal wo man sich im programm befindet").
- Code, comments, docs and commit messages in English; no AI attribution in commits.
- Bump `studioBuildId` in `src/editor/LightingWindow.cpp` once for the whole plan (next free: `GS-06-R16`).
- All 22 ctest suites stay green: `build/build-r10.cmd`, then `ctest -C Release` in `build/r10`.

## Review Focus

- A color chosen in the picker by dragging the saturation field, the hue slider, typing in a number field or the hex field, or the screen pipette must survive Apply exactly like a swatch click (Task 1 tests each input path).
- Moving a placement while output is running must reach the hardware within one render tick, also when a scan is running or a device is busy (Task 2).
- A project change that does change output scopes (new channel, removed device) must still close and reopen exactly those connections (Task 2).
- Disabling Edit actions in Devices must not disable their keyboard shortcuts inside text fields of the Devices panels (Task 4).
- Effect search must still find effects after the list becomes categorized and sorted (Task 5).

---

### Task 1: Gradient stop colors survive Apply

**Files:**
- Modify: `src/editor/ColorPicker.cpp:116-220` (`openStudioColorPicker`)
- Test: `tests/editor/test_editor.cpp` (new slot `pickerApplyKeepsEveryInputPath`)

**Interfaces:**
- Consumes: `ColorListEditor` (`addColorStop` button, `selectedStopColor` `ColorEditor`), popup object names `liveColorPicker`, `pickerHue`, `pickerRed`, `pickerHex`, the `QDialogButtonBox` Apply button.
- Produces: nothing new.

- [ ] **Step 1: Write the failing test** — for each input path (hue slider `setValue(200)`, `pickerRed` value 10 + editingFinished, `pickerHex` text `#12AB34` + editingFinished, a mouse drag on the saturation field, a swatch click as control): add a stop with `addColorStop`, open the picker on `selectedStopColor`, change the color, click Apply, then `QCOMPARE(colorList->colors()[selected], expected)` and the document stop color. Also cover: focus leaves the popup between change and Apply.
- [ ] **Step 2: Run** `ctest -R lighting_editor --output-on-failure` in `build/r10`. Expected: FAIL for at least one path; record which (the observed gray is suspected to be the `finished(result!=Accepted)` revert at line 213 after the `Qt::Popup` closes itself).
- [ ] **Step 3: Fix the failing path in `openStudioColorPicker`** — the popup must treat only Cancel/Escape as reject; an auto-close of the `Qt::Popup` (outside click, focus change, sampler return) after a change must keep the chosen color (accept), not revert to `original`.
- [ ] **Step 4: Run** the suite. Expected: PASS for all paths.
- [ ] **Step 5: Commit** `fix(editor): keep picker colors on Apply for every input path`.

### Task 2: Project edits update running hardware without reopening it

**Files:**
- Modify: `src/service/ServiceRuntime.cpp` (`loadBundle`, line with `stopConnections();document_=std::move(candidate);`)
- Test: `tests/service/test_service.cpp` (new slot `projectEditKeepsRunningOutput`)

**Interfaces:**
- Consumes: `ServiceRuntime::command`, `applyScan` retire logic (`retire`, `sameOutputScope`), `Recording` fixture (`opens`, `writes`).
- Produces: unchanged API; `loadBundle` no longer calls `stopConnections()`.

- [ ] **Step 1: Write the failing test** — load `project(profile)`, wait for `record->writes>1`, note `opens==1`; send a second project whose placement is moved (same endpoint/profile/channels); assert `record->opens` stays 1 and `record->writes` keeps increasing within 200 ms (no gap). Second case: a project that removes the channel → the connection is retired and `status()` no longer lists it as Running after the scan.
- [ ] **Step 2: Run** `ctest -R windows_service_contract`. Expected: FAIL (`opens` becomes 2, writes pause until the next scan).
- [ ] **Step 3: Remove `stopConnections()` from `loadBundle`** — keep connections; `tick()` already matches frames by `sameOutputScope`, and the scan that `command("project")` starts retires connections whose scope changed. Keep the comment explaining why (owner report 2026-11-17: dragging froze the lights for one scan).
- [ ] **Step 4: Run** the full suite. Expected: 22/22 PASS (watch `fakeOutputPauseAndResume`, `hungDriverCallKeepsAnswering`, `editorSynchronization`).
- [ ] **Step 5: Commit** `fix(service): keep running outputs across project edits`.

### Task 3: Portable start and its documentation

**Files:**
- Modify: `Start-Lighting.cmd` (prefer `build\r10`, drop `r4`/`routing`/`aurora`), `PORTABLE_README.txt` (rewrite for the current package: unzip, run `msi_mystic_light.exe` for portable use, or `installer\Install.cmd` for the service), `scripts/package_release.py` only if the owner wants a start script in the ZIP (default: no; the exe is the start).
- Test: `tests/service/test_installer.ps1` is unaffected; verification is manual.

- [ ] **Step 1:** Update `Start-Lighting.cmd` candidates to `build\r10\Release\msi_mystic_light.exe`, then the installed editor; keep the existing ASCII-key PowerShell selection.
- [ ] **Step 2:** Rewrite `PORTABLE_README.txt` (current build id, portable vs. installed, that an installed service makes the editor use it).
- [ ] **Step 3: Verify** by running `Start-Lighting.cmd` from the repo root: the R16 editor opens (Help > About shows `build\r10`).
- [ ] **Step 4: Commit** `docs: current portable start and launcher`.

### Task 4: Menus are available where they act, once

**Files:**
- Modify: `src/editor/LightingWindow.cpp` — File menu (230-244), Edit menu (262 ff., `addLayerMenu` at 663), `updateWorkspacePanels()` (1381-1388), output status label (1752)
- Test: `tests/editor/test_editor.cpp` (new slot `menusFollowWorkspace`)

**Interfaces:**
- Consumes: `tabs_` index (0 Layout, 1 Effects, 2 Devices), action object names `saveProject`, `saveProjectAs`, `newPaintLayer`, `newGroup`, the existing lock/unlock/duplicate actions.
- Produces: `void LightingWindow::updateWorkspaceActions()` called from the workspace switch; it enables actions per workspace.

- [ ] **Step 1: Write the failing test** — switch to Devices (`workspaceHardware`): `saveProject`/`saveProjectAs`/open enabled; lock, unlock, duplicate, delete and every layer-creation action disabled; `addLayerMenu` is not a submenu of the Edit menu (only the Navigator "+" offers effects); the output status label has no link (`textFormat` plain / no `href`). Switch back to Effects: those Edit actions enabled again.
- [ ] **Step 2: Run** `ctest -R lighting_editor`. Expected: FAIL.
- [ ] **Step 3: Implement `updateWorkspaceActions()`**, remove `editMenu->addMenu(addMenu)` (keep `addMenu` only as the source for the library list), and make the status label plain text (it stays as status; the Devices button is the only navigation). Find and remove whatever disables the File menu in Devices (search the tab-change handler and `setEnabled` calls on `fileMenu`/its actions).
- [ ] **Step 4: Run** the suite. Expected: PASS.
- [ ] **Step 5: Commit** `fix(editor): one entry point per function, actions only where they act`.

### Task 5: Effect library with sorted categories and a clear add button

**Files:**
- Modify: `src/editor/LightingWindow.cpp:1016-1060` (`sourcesTool`, `effectLibraryResults`), `src/editor/StudioWidgets.cpp:358` only if a new icon name is added
- Test: `tests/editor/test_editor.cpp` (new slot `effectLibraryCategorized`)

**Interfaces:**
- Consumes: category menus `patterns`, `shapes`, `particlesMenu`, `organic`, `scenes` built at line 673.
- Produces: the library list shows each category heading followed by its effects sorted case-insensitively; search hides empty categories.

- [ ] **Step 1: Write the failing test** — open `sourcesTool`; in `effectLibraryResults` every non-heading item lies under a heading and items under each heading are in ascending `localeAwareCompare` order; typing "pla" leaves only matching items plus their headings; `sourcesTool` shows the text "Add effect" and no menu-indicator arrow (`toolButtonStyle()==Qt::ToolButtonTextBesideIcon`).
- [ ] **Step 2: Run** `ctest -R lighting_editor`. Expected: FAIL.
- [ ] **Step 3: Implement** — sort each category's actions before filling `results`; filter hides headings without visible children; `sources->setText("Add effect")`, `setToolButtonStyle(Qt::ToolButtonTextBesideIcon)`, stylesheet `QToolButton::menu-indicator{image:none;}`, widen from the fixed 32×32.
- [ ] **Step 4: Run** the suite. Expected: PASS.
- [ ] **Step 5: Commit** `feat(editor): categorized, sorted effect library and labeled add button`.

### Task 6: Release R16

- [ ] **Step 1:** Set `studioBuildId` to `GS-06-R16` with a one-line R16 note; add a `CONTINUE.md` entry and these TODOs: effect previews in the library; operating-concept redesign (needs a spec via brainstorming).
- [ ] **Step 2:** Build, run all 22 tests, run `graphify update .`.
- [ ] **Step 3:** Package `scripts/package_release.py --runtime build/r10/Release --archive %USERPROFILE%/Downloads/LightingStudio-local-r16.zip` and check the exe contains `GS-06-R16`.
- [ ] **Step 4: Commit** `chore: build GS-06-R16`.

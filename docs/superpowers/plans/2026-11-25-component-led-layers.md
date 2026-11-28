# Component LED Layers Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** LEDs that sit physically behind each other (e.g. ThermalTake SWAFAN front/back ring) keep identical positions for effects but can be shown, selected and moved separately in the Component Editor via depth layers.

**Architecture:** Each LED gets an optional zero-based display layer (`ComponentDefinition::layers`, empty = all layer 0). Layers never affect sampling, mapping or output; only the Component Editor reads them. SignalRGB import puts the n-th LED on an already-used coordinate on layer n. The editor shows the active layer (or all, inactive dimmed and locked), and colors the wiring path per run with dashed jumps.

**Tech Stack:** C++17, Qt 6.8 Widgets/Graphics View, Qt Test, CMake/Ninja.

**Spec:** Owner decisions in chat 2026-11-25 (layout sketch "Werkzeugleiste + layer box": layer icon, ▲, number, ▼ below the tool rail) plus the five agreed refinements:
1. Clicking the layer icon toggles "active layer only" / "all layers" (inactive dimmed, not selectable).
2. Layer 1 = front. ▲ = next layer back (number +1), ▼ = towards the front. Tooltips say so. Shortcuts PgUp / PgDn.
3. ▲/▼ only step through existing layers. New layers only via "move selection to new layer". Empty layers are removed on save.
4. Moving LEDs between layers: a LAYERS section in the right panel (right-click is already "delete LED"), plus Ctrl+PgUp / Ctrl+PgDn.
5. The number shows "active / count" (e.g. "2/3"); with one layer the box stays visible but disabled.

## Global Constraints

- Positions are never changed by layer logic (owner: shifting would alter effect sampling).
- `layers` is optional in component JSON, no schema bump: written only when any LED has layer > 0; older builds ignore the unknown key (`ComponentDefinition::fromJson` does not reject unknown keys). Valid values 0..15, array length == `ledCount`.
- Layout and Effects canvases, `LightingDocument` sampling and output stay untouched.
- Layers and the view mode are not part of undo except as draft data: changing the *active/visible* layer is view state (no undo entry); moving LEDs between layers is an edit (undo entry "Move to layer N").
- UI text in English, owner-facing numbering 1-based, data 0-based.
- Match the existing code style of `ComponentDraft.cpp` / `ComponentCanvas.cpp` / `ComponentEditorPage.cpp`; comments reference "owner request 2026-11-25".

## Review Focus

1. **Loading an old project/library without `layers`** → all LEDs layer 0, saving writes no `layers` key (byte-identical JSON). Task 1 `componentLayersJson`.
2. **Deleting all LEDs of the active layer** → active layer clamps to the last existing one, view does not show an empty canvas with "3/2". Task 4 `componentEditorPageLayers`.
3. **Ctrl+A / box select with "all layers" on** → only active-layer LEDs get selected. Task 3 `componentCanvasLayers`.
4. **Shape preset or wiring preset on a multi-layer draft** → shape resets to one layer; wiring preset keeps layers. Task 2 `componentDraftLayers`.
5. **Instance mode (locked count)** → layer view and moving between layers still work (display-only data). Task 4.

---

### Task 1: Data model and JSON

**Files:**
- Modify: `src/components/ComponentCatalog.h:7-20`, `src/components/ComponentCatalog.cpp:30-107`
- Test: `tests/components/test_components.cpp` (new slot `componentLayersJson`)

**Interfaces:**
- Produces: `QVector<int> ComponentDefinition::layers;` (empty or size == positions.size()); `int ComponentDefinition::layerOf(int row) const` (0 when empty); free function `QVector<int> overlapLayers(const QVector<QPointF>& positions);` declared in `ComponentCatalog.h` — the n-th occurrence (0-based) of an exactly equal point gets layer n; returns empty when no point repeats.

- [ ] **Step 1: Failing test `componentLayersJson`**
  - JSON without `layers` → `layers.isEmpty()`, `layerOf(5)==0`, `toJson()` has no `"layers"` key.
  - JSON with `"layers":[0,1,1]` (3 LEDs) round-trips through `toJson`/`fromJson` unchanged.
  - Rejected with `std::invalid_argument`: wrong length, value 16, value -1, non-integer.
  - All-zero `layers` array → stored empty, not written back.
  - `overlapLayers({{0,0},{1,0},{0,0},{0,0}}) == QVector<int>{0,0,1,2}`; `overlapLayers({{0,0},{1,0}})` is empty.
- [ ] **Step 2:** Run `build\r36-ctest.cmd`-equivalent `ctest --test-dir build/r36 -R "^components$" --output-on-failure` → FAIL.
- [ ] **Step 3:** Implement; parse with the file's existing `integer(value,0,15)` helper.
- [ ] **Step 4:** Same command → PASS.
- [ ] **Step 5:** Commit `feat(components): optional per-LED display layers`.

### Task 2: Import and draft operations

**Files:**
- Modify: `src/editor/HardwarePicker.cpp:61-104` (`installedProducts`), `src/editor/ComponentDraft.h/.cpp`
- Test: `tests/editor/test_editor.cpp` (new slot `componentDraftLayers`)

**Interfaces:**
- Consumes: Task 1 `layers`, `overlapLayers`.
- Produces on `ComponentDraft`: `QVector<int> layers;` (same rules; included in `fromDefinition`, `toDefinitionUnchecked`, `operator==`); `int layerCount() const` (max+1, at least 1).
  - `ComponentDraft withLayer(ComponentDraft draft, const QSet<int>& rows, int layer);` — layer may equal `layerCount()` (creates a new one); throws for < 0 or > 15.
  - `withAdded(draft, point, int layer = 0)` — new signature with default.
  - `withRemoved` drops the rows' layers; `withDuplicated` copies each source row's layer; `withMirror`/`withRotation90`/`withPreset`/`withSharedAddress`/`withWireOrder` keep layers; `withShape` clears layers.
  - `toDefinition()` compacts layers (removes empty layers, keeps order, all-zero → empty).
  - Import: after coordinate checks, `c.layers = overlapLayers(<raw coordinates before scaling>)` — exact duplicates in the source file; comment: SWAFAN / Freezer 36 rings, CoreLiquid LED 4/5, Vertagear SL5000.

- [ ] **Step 1: Failing test `componentDraftLayers`** covering every bullet above, incl. `toDefinition()` compaction `{0,2,2}` → `{0,1,1}`, and `withLayer(...,16)` throws. Import function is not unit-testable (reads `%LOCALAPPDATA%`); `overlapLayers` is covered by Task 1.
- [ ] **Step 2:** `ctest --test-dir build/r36 -R lighting_editor --output-on-failure` → FAIL.
- [ ] **Step 3:** Implement.
- [ ] **Step 4:** Same → PASS, and `componentDraftOperations` still PASS.
- [ ] **Step 5:** Commit `feat(editor): layer-aware component draft edits and overlap import`.

### Task 3: Canvas — layer view and colored path

**Files:**
- Modify: `src/editor/ComponentCanvas.h/.cpp` (`rebuild()` at `:115-161`, Ctrl+A handling, add-LED click)
- Test: `tests/editor/test_editor.cpp` (new slot `componentCanvasLayers`)

**Interfaces:**
- Produces:
  - `void setActiveLayer(int layer); int activeLayer() const; void setAllLayersVisible(bool on); bool allLayersVisible() const;` (redraw only, no signal).
  - Free function in `ComponentCanvas.h`: `QVector<int> pathRuns(const QVector<QPointF>& points, const QVector<int>& layers);` — for consecutive path points, returns one entry per edge: run index ≥ 0, or `-1` for a jump. An edge is a jump when its two layers differ or its length > 2.5 × the median edge length (median over all edges; no jump detection with < 3 edges).
- Behavior:
  - Active-only view: LEDs of other layers are not drawn; path edges with an invisible end are not drawn.
  - All-layers view: other layers drawn with opacity 0.35, `ItemIsSelectable`/`ItemIsMovable` off, z below the active layer; path drawn fully.
  - Path pen per run, cycling a fixed 4-color palette local to `ComponentCanvas.cpp` (the theme has only one hue): `#45DDF5` (= `@accent`), `#F5B945`, `#E56BF0`, `#7EE06A`, alpha 200, solid; jumps dashed in `@text` at 60 % with a small arrowhead at the target.
  - Clicking empty canvas adds the LED on the active layer (`withAdded(..., activeLayer())`).
  - Shared-address dotted lines unchanged.

- [ ] **Step 1: Failing test `componentCanvasLayers`**: SWAFAN-like draft (12+12 identical ring points, 6 center points, layers 0×12, 1×12, 0×6); active layer 0 → 18 LED items, active layer 1 → 12; all-layers view → 30 items, of which 12 not selectable; Ctrl+A selects only active-layer rows; click adds an LED whose draft layer == active layer. `pathRuns` on the 30 path points → edges 0–10 run 0, edge 11 (12→13) −1, edges 12–22 run 1, edge 23 (24→25) −1, edges 24–28 run 2.
- [ ] **Step 2:** Run `lighting_editor` → FAIL. **Step 3:** Implement. **Step 4:** PASS incl. `componentCanvasInteractions`.
- [ ] **Step 5:** Commit `feat(editor): component canvas layer view and per-run wiring colors`.

### Task 4: Page — layer box, LAYERS section, shortcuts

**Files:**
- Modify: `src/editor/ComponentEditorPage.h/.cpp` (tool rail `:400-416`, right panel next to `buildShapesAndWiring` `:472`, key bindings `:555-595`), `src/editor/StudioWidgets.cpp` (icon `"layers"`: two stacked parallelograms as in the sketch)
- Test: `tests/editor/test_editor.cpp` (new slot `componentEditorPageLayers`)

**Interfaces:**
- Consumes: Tasks 2–3.
- Object names (tests rely on them): `componentLayerView` (checkable tool button, icon `layers`, tooltip "Show all layers (inactive layers are dimmed and locked)"), `componentLayerBack` (▲, tooltip "Next layer back (PgUp)"), `componentLayerFront` (▼, "Next layer to the front (PgDn)"), `componentLayerLabel` (QLabel "2/3"), right panel section `LAYERS` with `componentMoveLayer` (QSpinBox, range 1..layerCount()+1, tooltip "The highest value creates a new layer") and `componentMoveLayerApply` ("Move selected", disabled without selection).
- Behavior: box placed directly below the tool rail tools, above the rail stretch (sketch); with `layerCount()==1` the view toggle and ▲/▼ are disabled, the label shows "1/1" (new layers come from the LAYERS section or Ctrl+PgUp); ▲/▼ stop at the ends; PgUp/PgDn = ▲/▼; Ctrl+PgUp/Ctrl+PgDn = move selection one layer back/front (back from the last layer creates a new one; front from layer 1 does nothing) and makes that layer active; after any edit the active layer is clamped to `layerCount()-1`; the help popup lists the four shortcuts; status-bar hint unchanged.

- [ ] **Step 1: Failing test `componentEditorPageLayers`**: open SWAFAN-like draft → label "1/2"; PgUp → "2/2", canvas active layer 1; select one LED, Ctrl+PgUp → "3/3", draft layer of that row 2, undo restores layers and label "2/2"; delete all LEDs of layer 2 → label clamps; single-layer draft → `componentLayerLabel` "1/1" and ▲/▼ disabled; instance mode → Ctrl+PgUp still works.
- [ ] **Step 2:** FAIL. **Step 3:** Implement. **Step 4:** full `ctest --test-dir build/r36` → 22/22 PASS.
- [ ] **Step 5:** Commit `feat(editor): layer box and LAYERS section in the Component Editor`.

### Task 5: Docs, build, handoff

**Files:** `docs/LIGHTING_EDITOR.md` (Component Editor: layers section), `CONTINUE.md`, build ID in `src/editor/LightingWindow.cpp` (`GS-06-R37`).

- [ ] **Step 1:** Update docs (what layers are, that they do not affect effects, the shortcuts).
- [ ] **Step 2:** `graphify update .`; bump build ID; `build\r36-build.cmd`; full CTest → PASS.
- [ ] **Step 3:** Package `python -I scripts/package_release.py --runtime build/r36/Release --archive %USERPROFILE%/Downloads/LightingStudio-local-r37.zip`; compare file list with R36 (56 files).
- [ ] **Step 4:** Commit `chore: build GS-06-R37`. Owner checks SWAFAN, Freezer 36, CoreLiquid, Vertagear in the real window.

## Out of scope

Split screen, 3D view, layers in Layout/Effects, layer names.

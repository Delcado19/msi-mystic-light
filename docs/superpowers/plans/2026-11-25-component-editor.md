# Component Editor Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the "Custom components" dialog and the modal `ComponentEditor` ("Component Lab") with a full-window **Component Editor** page: a component list with delete, a large LED canvas with direct mouse/keyboard editing and undo, and one properties column.

**Architecture:** The editor becomes a page in the existing `applicationPages_` `QStackedWidget` next to `tabs_`; **Back** returns to the workspace tabs. The hidden `QTableWidget componentLeds`, which is today's real data model, is replaced by a plain `ComponentDraft` value plus a page-local `QUndoStack` of snapshots. The reusable logic of `ComponentEditor.cpp` (shape generators, mirror, wiring presets, shared-address assignment, address compression) moves into pure functions on `ComponentDraft`.

**Tech Stack:** C++20, Qt 6 Widgets (`QGraphicsView`, `QUndoStack`), QtTest, CMake/Ninja (`build/redesign`).

**Spec:** This plan carries its own UX spec (next section). The reference is the owner's screenshots of 2026-11-21 (Component Lab for SignalRGB) and the owner's critique of 2026-11-25. Functional rules from `docs/UX_REDESIGN_PLAN.md` ("Custom components", "Component Lab") still apply unless this plan changes them. `DESIGN.md` is not on this branch; visual tokens come from `src/editor/StudioTheme.h`.

## UX spec

### Layout

```
+----------------------------------------------------------------------------------------------+
| < Back | Phanteks NV5 MKII / internal LED strip  Strips    [New][Import][Export][Delete] [Save]| top bar
+-------------+--+-----------------------------------------------------+-----------------------+
| COMPONENTS  |  |                                                     | v DEVICE              |
| [search   ] |Sel|   .   .   .   .   .   .   .   .   .   .   .         |  Name      [........] |
| My components|Wir|   .  (1)--(2)--(3)--(4)--(5)--(6)--(7)   .         |  Manufacturer [.....] |
|  NV5 strip *|--|   .   .   .   .   .   .   .   .   .   .   .         |  Category  [Strips v] |
|  My fan     |Dup|                                                     |  16 LEDs / 16 addr.   |
| Device copies|MiH|                                                    |  > Advanced (ID)      |
|  (12) v     |MiV|                                                     | v ADD SHAPES          |
| Built-in    |Rot|                                                     |  [Matrix][Strip][Ring]|
|  M25 Gen2   |Del|                                                     |  [Fan][L-Shape][U-Sh.]|
|  D30-120    |--|                                                     |  LEDs [ 12 ]          |
|             |Grd|                                                     | v WIRING              |
|             |Pth|                                                     |  Click LEDs in data   |
|             |--|                                                     |  order with Wire (W)  |
|             |Z+ |                                                     |  [Rows left->right]   |
|             |Z- |                                                     |  [Rows serpentine]    |
|             |Fit|                                                     |  [Columns top->bottom]|
|             |? |                                                     |  [Clockwise]          |
|             |  |                                                     |  Address [3] [Assign] |
|             |  |                                                     | > DANGER ZONE         |
+-------------+--+-----------------------------------------------------+-----------------------+
| 16 LEDs  grid 3, -1  zoom 186%      click = add  drag = move  R-click = delete  M-drag = pan  | status bar
+----------------------------------------------------------------------------------------------+
```

Column widths: list 260 px, tool rail 48 px, properties 320 px, the canvas takes the rest. Right-panel sections are collapsible (`QToolButton` header + body); their open/closed state is not persisted.

### Behavior

- **Entry.** The Devices button "Custom components" is renamed **Component Editor** and opens the page. Devices "Edit component" opens the same page in *instance mode* for the selected placement.
- **Component list.** Three groups. **My components**: library entries whose ID does not start with `instance.` or `restored.`. **Device copies (n)**: `instance.*`/`restored.*` entries, collapsed by default and labelled with the placement that uses them or "unused". **Built-in**: `installedCatalog()` plus the hardware-picker products, read-only. An entry used by the open document gets a dot marker. A search field filters by name. Selecting an entry with unsaved changes asks Save / Discard / Cancel.
- **Built-in entries** open read-only. Save becomes **Save as copy**, which registers a new library entry with ID `custom.<slug>.<8 hex>`.
- **Canvas.** Dot grid, LEDs as numbered circles (number = address + 1). The selection uses `@accent` `#45DDF5`. The wire path is a dashed line in address order, and points sharing an address are joined by a dotted line. Hover shows a tooltip "LED n · x, y".
- **Mouse.** Select tool (V): left-click empty = add LED at the snapped position; left-drag LED = move the selection; left-drag empty = box select; Shift+click/drag = add to the selection; right-click LED = delete it. Wire tool (W): left-click LEDs in data order. Everywhere: middle-drag or Space+drag = pan; wheel = zoom at the cursor.
- **Keyboard.** V, W, R (rotate selection 90° clockwise), D (duplicate the selection with a +1 grid offset), M / Shift+M (mirror H/V), Del/Backspace (delete the selection, or the LED under the cursor), Ctrl+Z / Ctrl+Y / Ctrl+Shift+Z, Ctrl+A, Esc (leave Wire mode / clear the selection), G / P (toggle grid / wire path), + / - / 0 (zoom in / out / fit), ? (help overlay).
- **Undo.** Every completed gesture is one undo step: a drag on release, a shape, a preset, an assign, a delete, a property edit on `editingFinished`. The stack is cleared when another entry is loaded.
- **Add shapes.** Each button replaces all LEDs with the shape at the LED count in the "LEDs" spin box. That count defaults to the current count, or 12 when there are none. The shapes are Matrix, Strip (line), Ring (circle), Fan (circle plus `category="Fans"`), L-Shape and U-Shape. It is one undo step.
- **Wiring.** Wire mode is a numbered click sequence. Once all addresses are clicked it applies automatically; Esc cancels it. There are four preset buttons. **Address** spin + **Assign** makes the selected LEDs share an address, then compresses the gaps (the existing logic).
- **Danger zone.** "Clear all LEDs" (one undo step) and "Delete component" (the same as the top bar Delete).
- **Delete.** Only for library entries (My components, Device copies). It is **blocked** while any placement in the open document references the entry; the message names the placements. Otherwise a confirmation dialog appears and the entry is removed from `custom-components.json` under the existing `QLockFile`. Delete is not undoable, and the dialog says so.
- **Save.** For a library entry, Save **overwrites that entry in place**. Today `reconcileDefinition()` forks a changed definition into `id.<sha256>`, which is where the junk variants in the owner's library come from. If the open document uses the entry, its `state.components` copy is updated in the same `edit()`, and the edit is rejected if channel capacity validation fails.
- **Instance mode** (from Devices "Edit component"). The list column is hidden, and the top bar shows "Editing <placement name> on <connector>" with **Apply** instead of Save. On a fixed channel (`arrangementOnly`) or with `nonPhysicalSlots` (e.g. Vulcan): LED count, add/delete, shapes and the Wiring section are disabled; only moving, mirroring and rotating stay. Apply calls `setComponentGeometry` (fixed) or `applyComponentVariant` (otherwise), exactly as `LightingWindow.cpp:543-552` does today.
- **Back / leaving with unsaved changes** asks Save (Apply) / Discard / Cancel.
- **Status bar.** The LED count, the grid coordinate under the cursor (`--, --` outside), the zoom percentage, and a hint line for the active tool.

## Global Constraints

- Code, comments, UI text and docs in English.
- No new dependencies. Qt 6 Widgets only.
- Colors only via `studioThemeColor()` roles; accent `#45DDF5` for the selection.
- Saved component JSON stays `ComponentDefinition::toJson()` (schema 1/2). No new fields: there is no "display name" and no "image URL".
- Positions are stored in component units (the current `coordinateScale` of 24 px per unit at 100 % zoom stays the canvas base scale).
- LED limit: 1..65535 positions, as in the current spin box; `ComponentDefinition::fromJson` remains the only validator.
- Do not touch the service or the saved project schema. `HardwarePicker` changes only by the pure extraction in Task 4.

## Review Focus

1. **Deleting a library entry that another, not-open layout references.** Expected: allowed, and the confirmation text warns that other layouts will report the component as missing on open (existing MissingComponents dialog). Test in Task 2.
2. **Saving an edited built-in entry.** Expected: never writes to `installedCatalog`, always creates a copy. Test in Task 2.
3. **An instance with `nonPhysicalSlots` (Vulcan, 144 slots).** Expected: count and mapping cannot change, and mapping is identical after Apply. Test in Task 6.
4. **Undo after a shape on a large count (e.g. 1000 LEDs).** Expected: one step restores every position. Test in Task 1.
5. **Leaving the page with unsaved edits through the window's workspace shortcuts or close.** Expected: the same Save/Discard/Cancel prompt as Back. Test in Task 5.

---

## File structure

| File | Responsibility |
|---|---|
| Create `src/editor/ComponentDraft.h/.cpp` | Value type + pure edit operations (no widgets). |
| Create `src/editor/ComponentCanvas.h/.cpp` | `QGraphicsView` canvas: drawing, tools, mouse, hover, zoom/pan. Emits finished edits. |
| Create `src/editor/ComponentEditorPage.h/.cpp` | The page: top bar, list, tool rail, properties, status bar, undo, dirty handling, instance mode. |
| Modify `src/editor/LightingDocument.h/.cpp` | `removeCustomComponent`, `updateCustomComponent`, `componentUsers`. |
| Modify `src/editor/LightingWindow.h/.cpp` | Host the page, rewire the three callers, delete the library dialog. |
| Delete `src/editor/ComponentEditor.h/.cpp` | Removed in Task 6 after all callers move. |
| Modify `CMakeLists.txt:107` | Swap the sources. |
| Modify `tests/editor/test_editor.cpp` | New tests; migrate `componentEditor()`, `componentAuthoring()` and every `findChild` on `componentLab`, `componentLeds`, `componentGeometry`, `startWireOrder`, `assignLedAddress`, `customComponentLibrary`. |
| Modify `docs/LIGHTING_EDITOR.md` ("Component authoring and import"), `docs/UX_REDESIGN_PLAN.md` (status table) | Docs. |

---

### Task 1: `ComponentDraft` model and pure operations

**Files:** Create `src/editor/ComponentDraft.h`, `src/editor/ComponentDraft.cpp`; Modify `CMakeLists.txt:107` (add source); Test `tests/editor/test_editor.cpp` (new slot `componentDraftOperations`).

**Interfaces — Produces:**
```cpp
struct ComponentDraft {
    QString id, name, manufacturer, category;
    QVector<QPointF> positions;   // component units
    QVector<int> mapping;         // zero-based address per position
    QVector<int> nonPhysicalSlots;
    static ComponentDraft fromDefinition(const ComponentDefinition&);
    ComponentDefinition toDefinition() const;          // throws via ComponentDefinition::fromJson
    bool operator==(const ComponentDraft&) const = default;
};
enum class DraftShape { Matrix, Strip, Ring, Fan, LShape, UShape };
enum class WiringPreset { RowsLeftToRight, RowsSerpentine, ColumnsTopToBottom, Clockwise };
ComponentDraft withShape(ComponentDraft, DraftShape, int count);          // Fan also sets category "Fans"; addresses 0..count-1
ComponentDraft withPreset(ComponentDraft, WiringPreset);                  // port of ComponentEditor.cpp:350-382
ComponentDraft withSharedAddress(ComponentDraft, const QSet<int>& rows, int address); // port of :383-401 incl. compression
ComponentDraft withWireOrder(ComponentDraft, const QVector<int>& addressOrder);       // port of renumber :306-314
ComponentDraft withMirror(ComponentDraft, const QSet<int>& rows, Qt::Orientation);    // empty rows = all physical
ComponentDraft withRotation90(ComponentDraft, const QSet<int>& rows);     // clockwise about the rows' bounding-box center
ComponentDraft withAdded(ComponentDraft, QPointF);                        // new address = max+1
ComponentDraft withRemoved(ComponentDraft, const QSet<int>& rows);        // compresses addresses; throws if nonPhysicalSlots non-empty
ComponentDraft withDuplicated(ComponentDraft, const QSet<int>& rows, QPointF offset);
bool draftLocksCount(const ComponentDraft&);                              // true when nonPhysicalSlots non-empty
```

- [ ] **Step 1: Write the failing test** `componentDraftOperations`:
  - `withShape(d, DraftShape::Ring, 12)`: 12 positions, all at distance 1.3 ± 1e-9 from (1.3,1.3), mapping `0..11`.
  - `withShape(d, DraftShape::Fan, 12).category == "Fans"`.
  - `withShape(d, DraftShape::Strip, 16)`: every y equal, x strictly increasing.
  - `withPreset(3×2 matrix, RowsSerpentine).mapping == {0,1,2,5,4,3}` (rows by y, second row reversed).
  - `withSharedAddress(12-LED draft, {0,11}, 0)`: rows 0 and 11 have address 0, the max address is 10, and no gaps.
  - `withRemoved(16-LED strip, {3})`: 15 positions, mapping `0..14`.
  - `withRotation90` applied four times returns the original positions ± 1e-9.
  - `withMirror` applied twice returns the original.
  - Round trip: `ComponentDraft::fromDefinition(catalog().first()).toDefinition().toJson() == catalog().first().toJson()`.
  - Vulcan draft (from `components/peripheral-vulcan.json`): `draftLocksCount` is true, and `withRemoved` throws `std::invalid_argument`.
  - Review Focus 4: a 1000-LED draft, `withShape(..., Matrix, 1000)` then compare against a stored copy. The copy equals the original (value semantics; this is what makes snapshot undo valid).
- [ ] **Step 2: Run, expect FAIL** (does not compile: `ComponentDraft` undefined). Command: `cmake --build build/redesign && build\redesign\tests\test_editor.exe componentDraftOperations` (in a VS dev shell: `Launch-VsDevShell.ps1 -Arch amd64`).
- [ ] **Step 3: Implement** in `ComponentDraft.cpp`. Port the algorithms verbatim from `ComponentEditor.cpp` (shapes :440-490, mirror :492-510, presets :350-382, assign/compress :383-401, renumber :306-314), operating on the vectors instead of table cells. Templates keep their current geometry: Ring/Fan radius 1.3 at center (1.3,1.3); Strip spacing `3.0*i/(n-1)`.
- [ ] **Step 4: Run, expect PASS.**
- [ ] **Step 5: Commit** `feat(editor): add ComponentDraft model with pure LED edit operations`.

### Task 2: Library delete, in-place update, usage lookup

**Files:** Modify `src/editor/LightingDocument.h` (after `registerComponent`), `src/editor/LightingDocument.cpp`; Test: new slot `componentLibraryEditing`.

**Interfaces — Produces:**
```cpp
// Placements of the open document that reference the definition, in placement order.
QVector<QUuid> componentUsers(const QString& id) const;
// Removes a library entry. Throws std::invalid_argument when id is installed or used by the open document.
void removeCustomComponent(const QString& id);
// Overwrites the library entry with the same id (no variant fork). Updates the document copy in one
// undoable edit when placements use it; throws (and writes nothing) if channel validation fails.
void updateCustomComponent(const ComponentDefinition& definition);
// Registers a copy of an installed or library definition under "custom.<slug>.<8 hex>"; returns the new id.
QString saveComponentCopy(const ComponentDefinition& definition);
```

- [ ] **Step 1: Write the failing test** `componentLibraryEditing`. Use a `QTemporaryDir` library path, constructed the same way as the `componentLibraryMigration` test (`test_editor.cpp:4107`).
  - Register `custom.a`; `removeCustomComponent("custom.a")` → `customComponents()` no longer contains it.
  - `removeCustomComponent(<installed id>)` throws.
  - Add a placement of `custom.b`, then `removeCustomComponent("custom.b")` throws and the message contains the placement name; `componentUsers("custom.b").size()==1`.
  - `updateCustomComponent(custom.b with moved position 0)` → the library keeps exactly one entry with id `custom.b` (no `custom.b.<hash>`), and `catalog()["custom.b"].positions[0]` is the moved point. One `history()->undo()` restores the document copy.
  - Review Focus 2: `saveComponentCopy(installed)` returns an id starting with `custom.`, and `installedCatalog()` is unchanged.
  - Review Focus 1: removing an unused entry succeeds even though a *saved* layout file in the temp dir references it; loading that file afterwards raises `MissingComponents`.
- [ ] **Step 2: Run, expect FAIL.**
- [ ] **Step 3: Implement.** All three writers use the same `QLockFile(libraryPath_ + ".lock")` + `readLibrary`/`writeLibrary` pattern as `registerComponent` (`LightingDocument.cpp:358-373`). `updateCustomComponent` validates inside `edit()` (as `applyComponentVariant` does at :392-405) and writes the library only after validation passes. Without a library path (tests/portable), operate on `state.components` only. Add a one-line comment on `updateCustomComponent` explaining that it bypasses `reconcileDefinition` on purpose, to stop the variant forks seen in the owner's library.
- [ ] **Step 4: Run, expect PASS** (`test_editor.exe componentLibraryEditing componentLibraryMigration`).
- [ ] **Step 5: Commit** `feat(editor): delete and update custom component definitions in place`.

### Task 3: `ComponentCanvas` interactions

**Files:** Create `src/editor/ComponentCanvas.h/.cpp`; Modify `CMakeLists.txt:107`; Test: new slot `componentCanvasInteractions`.

**Interfaces — Consumes:** `ComponentDraft`, `withAdded`, `withRemoved` (Task 1); `SnapGrid` (`src/editor/SnapGrid.h`); `drawHardwareComponentArtwork`.
**Produces:**
```cpp
class ComponentCanvas : public QGraphicsView {
    Q_OBJECT
public:
    enum class Tool { Select, Wire };
    explicit ComponentCanvas(QWidget* parent = nullptr);
    void setDraft(const ComponentDraft&);          // redraw only, no signal
    void setTool(Tool); Tool tool() const;
    void setLocked(bool countAndWiringLocked);     // instance mode: no add/delete, no Wire tool
    QSet<int> selectedRows() const; void selectRows(const QSet<int>&);
    void setGridVisible(bool); void setPathVisible(bool);
    void zoomBy(double factor); void fit(); double zoom() const;  // 1.0 == 100 %
signals:
    void draftEdited(const ComponentDraft& after, const QString& label); // one per finished gesture
    void wireOrderFinished(const QVector<int>& addressOrder);           // all addresses clicked
    void cursorMoved(std::optional<QPointF> componentPosition);
    void zoomChanged(double);
};
```
objectName `componentCanvas`. Each LED item carries `data(0)=row`, `data(1)=address`.

- [ ] **Step 1: Write the failing test** `componentCanvasInteractions`. Show a canvas with a 3-LED strip draft, connect `draftEdited` to a lambda that stores `after` and calls `setDraft(after)`.
  - Left-click empty scene point → one emission, 4 positions.
  - Right-click the LED at row 0 → 3 positions.
  - Drag LED row 1 by 24 px → exactly one emission on release, row 1 x moved by +1.0 unit (grid snapping disabled via `SnapGrid` save in the test setup).
  - Rubber-band over all → `selectedRows().size()==3`.
  - Wheel up over a point → `zoom()` > 1 and that scene point stays under the cursor within 1 px.
  - Middle-drag → horizontal scrollbar value changes, no emission.
  - Wire tool: click rows 2,1,0 → `wireOrderFinished({2,1,0})`.
  - `setLocked(true)`: left-click empty and right-click LED emit nothing, and `setTool(Wire)` keeps `Select`.
  - Hover over LED → `toolTip()` of the item is `"LED 1 · x, y"`.
- [ ] **Step 2: Run, expect FAIL.**
- [ ] **Step 3: Implement.** Start from `GeometryView` (`ComponentEditor.cpp:51-113`): keep the snap logic and background/artwork drawing; replace commit-to-table with `draftEdited`. Zoom at the cursor uses `setTransformationAnchor(AnchorUnderMouse)`. The pan uses a middle button or Space held (`ScrollHandDrag` toggled temporarily). Pen colors come from `studioThemeColor`.
- [ ] **Step 4: Run, expect PASS.**
- [ ] **Step 5: Commit** `feat(editor): add ComponentCanvas with direct LED editing`.

### Task 4: `ComponentEditorPage` shell: list, top bar, save/delete

**Files:** Create `src/editor/ComponentEditorPage.h/.cpp`; Modify `CMakeLists.txt:107`; Test: new slot `componentEditorPageLibrary`.

**Interfaces — Consumes:** Task 2 document API, Task 3 canvas.
**Produces:**
```cpp
class ComponentEditorPage : public QWidget {
    Q_OBJECT
public:
    ComponentEditorPage(LightingDocument& document, const ComponentCatalog& builtIn, QWidget* parent = nullptr);
    void openLibrary(const QString& selectId = {});                 // library mode
    void openInstance(const QUuid& placement, bool arrangementOnly); // instance mode (Task 6)
    bool confirmLeave();   // Save/Discard/Cancel when dirty; true when leaving is allowed
    bool isDirty() const;
signals:
    void closeRequested();
};
```
objectNames: `componentEditorPage`, `componentList`, `componentSearch`, `componentBack`, `componentNew`, `componentImport`, `componentExport`, `componentDelete`, `componentSave`, `componentName`, `componentManufacturer`, `componentCategory`.
`builtIn` = `document.installedCatalog()` plus the hardware-picker products. Expose them from `HardwarePicker.cpp` as `ComponentCatalog hardwareProducts(const ComponentCatalog& catalog)`, extracted from the constructor lines that add generic fans/strips, the M25, the NV5 strip and `installedProducts`. This is the only `HardwarePicker` change, a pure extraction.

- [ ] **Step 1: Write the failing test** `componentEditorPageLibrary` (library in a `QTemporaryDir`):
  - The list shows groups "My components", "Device copies (1)" and "Built-in"; an `instance.x` entry appears only under Device copies.
  - Selecting a built-in → Save text is "Save as copy", Delete is disabled.
  - Selecting `custom.a`, editing the name, Save → library has `custom.a` with the new name and no extra entry.
  - Delete on an unused `custom.a`, with an auto-accepted confirmation (inject via a `std::function<bool(QString)> confirm` member set by the test) → gone from the list and the library.
  - Delete on a used entry → nothing removed, the status label names the placement.
  - Dirty + select another entry with confirm returning Cancel → selection stays, draft unchanged.
- [ ] **Step 2: Run, expect FAIL.**
- [ ] **Step 3: Implement** the layout from the UX spec (`QSplitter` is not needed; use fixed-width side columns in a `QHBoxLayout`). New = an empty draft named "New component", category "Other", 0 LEDs. Import/Export reuse `importComponent` and the export block at `LightingWindow.cpp:492-504` (move it, don't copy it). Device copies show "used by <componentName(placement)>" or "unused". Confirmation dialogs go through the injectable `confirm` hook (default: `QMessageBox::question`).
- [ ] **Step 4: Run, expect PASS.**
- [ ] **Step 5: Commit** `feat(editor): add Component Editor page with component list, save and delete`.

### Task 5: Tool rail, properties sections, undo, shortcuts, status bar, help

**Files:** Modify `src/editor/ComponentEditorPage.cpp`; Test: new slot `componentEditorPageEditing`.

**Interfaces — Consumes:** all Task 1 operations; canvas signals.
objectNames: tool rail `componentToolSelect`, `componentToolWire`, `componentDuplicate`, `componentMirrorH`, `componentMirrorV`, `componentRotate`, `componentDeleteLeds`, `componentGridToggle`, `componentPathToggle`, `componentZoomIn`, `componentZoomOut`, `componentFit`, `componentHelp`; shapes `componentShape<Matrix|Strip|Ring|Fan|LShape|UShape>`, `componentShapeCount`; presets `componentPreset<RowsLeftToRight|RowsSerpentine|ColumnsTopToBottom|Clockwise>`; `componentAddress`, `componentAssignAddress`, `componentClearAll`; status labels `componentStatusLeds`, `componentStatusGrid`, `componentStatusZoom`, `componentStatusHint`.

- [ ] **Step 1: Write the failing test** `componentEditorPageEditing`:
  - Click `componentShapeRing` with count 12 → 12 LEDs; Ctrl+Z → previous draft; Ctrl+Y → 12 again.
  - `componentPresetClockwise` → mapping changes; one undo restores it.
  - Select rows {0,1}, set `componentAddress` = 1, Assign → both address 0, `componentStatusLeds` text `"12 LEDs · 11 addresses"`.
  - Ctrl+A then Del → 0 LEDs; Ctrl+Z → 12.
  - Keys V/W switch the checked tool button; Esc in Wire mode returns to Select.
  - Key G toggles `componentGridToggle` checked state.
  - Review Focus 5: dirty page, `LightingWindow` workspace switch to Effects (`workspaceEffects` action) with confirm returning Cancel → the page stays current.
  - `componentHelp` opens a child with objectName `componentHelpOverlay` listing every shortcut in the UX spec.
- [ ] **Step 2: Run, expect FAIL.**
- [ ] **Step 3: Implement.** Use a page-local `QUndoStack` with one `QUndoCommand` subclass holding `before`/`after` `ComponentDraft` snapshots; `redo()`/`undo()` call `setDraft`. Use `QShortcut` with `Qt::WidgetWithChildrenShortcut` on the page so the shortcuts do not leak into Layout/Effects. Property fields push a command on `editingFinished`. The hint line text per tool is copied from the "Mouse" bullet of the UX spec.
- [ ] **Step 4: Run, expect PASS.**
- [ ] **Step 5: Commit** `feat(editor): Component Editor tools, wiring panel, undo and shortcuts`.

### Task 6: Integrate into `LightingWindow`, instance mode, remove the old editor

**Files:** Modify `src/editor/LightingWindow.h/.cpp` (lines 440-517 library dialog and buttons, 535-552 `editDeviceLeds`), `CMakeLists.txt:107`; Delete `src/editor/ComponentEditor.h/.cpp`; Test: migrate `componentEditor()` (`test_editor.cpp:1318`), `componentAuthoring()` (:1215), and every remaining `findChild` on the old objectNames (grep `componentLab|componentLeds|componentGeometry|startWireOrder|assignLedAddress|customComponentLibrary|ledArrangementShapes`); new slot `componentEditorInstanceMode`.

- [ ] **Step 1: Write the failing test** `componentEditorInstanceMode`:
  - Devices button with objectName `openComponentEditor` and text "Component Editor" exists; clicking it makes `applicationPages_` current widget `componentEditorPage`; `componentBack` returns to the workspace tabs.
  - Place an M25 on a direct channel, trigger `editHardwareLeds` → page in instance mode: `componentList` hidden, the Save button reads "Apply", `componentShapeRing` enabled. Drag one LED, Apply → `applyComponentVariant` result: the placement's componentId changed, its mapping is unchanged.
  - Review Focus 3: a Vulcan placement (fixed channel) → shapes, `componentToolWire`, `componentDeleteLeds` and `componentShapeCount` disabled; after Apply, `catalog()[componentId].mapping` and `nonPhysicalSlots` equal the originals.
  - No widget named `customComponentLibrary` or `componentLab` exists.
- [ ] **Step 2: Run, expect FAIL.**
- [ ] **Step 3: Implement.** Add the page to `applicationPages_`. Replace the `layoutControls`→`QDialog` reparenting (`LightingWindow.cpp:484-490`) and the Create / Edit geometry / Import / Export buttons with the single **Component Editor** button. Keep `catalog_` and "Add component" in `layoutControls` only if a test or caller still needs them; otherwise delete them, since Devices "Add component" (HardwarePicker) is the add path. `editDeviceLeds` calls `openInstance(id, arrangementOnly)` and switches pages. Workspace switches (`workspaceHardware`/`Layout`/`Effects` actions) and `closeEvent` call `confirmLeave()` while the page is current. Delete `ComponentEditor.*` and migrate the old tests to the new objectNames. Behavior assertions stay; table-cell assertions become `ComponentDraft` assertions.
- [ ] **Step 4: Run the full suite**: `ctest --test-dir build/redesign` → expect `100% tests passed` (22/22).
- [ ] **Step 5: Docs.** Rewrite `docs/LIGHTING_EDITOR.md` "Component authoring and import" (from line ~505) for the page, the shortcuts, delete-blocking and Save-in-place. Update the Component Lab row in the `docs/UX_REDESIGN_PLAN.md` status table. Add a one-line `CONTINUE.md` note.
- [ ] **Step 6: Commit** `feat(editor): replace Custom components dialog with the Component Editor page`. Then run `graphify update .`.

## Open decisions (defaults used by this plan)

1. **Deleting an entry used in the open layout:** blocked, and the message names the placements (alternative: delete and also remove the placements).
2. **Device copies (`instance.*`, `restored.*`):** shown in a collapsed group with usage and deletable when unused (alternative: hidden entirely).
3. **"Use this Component" from the reference:** not added. In library mode Save is enough, and adding hardware stays in Devices "Add component". In instance mode the button is "Apply".
4. **Display name / image URL fields from the reference:** not added. `ComponentDefinition` has no such fields, and the artwork comes from the category photos.

## Out of scope

The fixed 176×110 Devices preview and the stretched strip photo (`LightingWindow.cpp:2293`, `:2306`) are a separate small fix.

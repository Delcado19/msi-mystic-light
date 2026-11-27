# Attach Flow and Component Library Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** A first-time user attaches a new or existing component to a header in about 5-6 actions, from one place, with every component coming from one file-per-component source.

**Architecture:** Built-in components become one JSON file each in `components/` (read-only, shipped). User components become one JSON file each in a user-chosen folder (QSettings key), replacing `custom-components.json`. The Devices tree groups headers under their board; the Attach dialog is rebuilt around a header row, a segmented search, two catalogs and an inline quick-create form.

**Tech Stack:** C++20, Qt 6 Widgets, QTest (`tests/editor/test_editor.cpp`, CTest `lighting_editor`), build `build\r36-build.cmd`, tests `build\r36-ctest.cmd`.

**Spec:** Owner decisions in chat 2026-11-27 (summarized under Global Constraints); critique `.impeccable/critique/2026-11-27T09-32-27Z__src-editor-componenteditorpage-cpp.md` (P1 bridge, P1 terminology).

## Global Constraints

- UI copy stays English for now (owner, 2026-11-27).
- Count and show **addresses only** in the Attach dialog, never LEDs (owner: "nur adressen, das muss reichen").
- Header row: header name left, `NNN addresses remaining` right. Component row: component name left, `N addresses` right. Selecting a component updates the remaining count live.
- Two catalogs: built-in (shipped, not editable) and user components (user folder). Both sorted by manufacturer. Search works in both.
- Search bar is recognizably a search field (frame, magnifier); type and manufacturer are two separate criteria combined with it (Amazon-style segmented bar).
- Empty search result offers `+ Create "<search text>"`, the search text becomes the name. Permanent tile label: `+ New component`.
- New component from the dialog: quick-create inline (Option A) plus `Open in editor…` for complex shapes.
- JRGB1 is listed but disabled with `12 V RGB · not supported yet` (no 12 V hardware to test, owner 2026-11-27).
- Board label shown as printed on the board: `JARGB_V2_1` (manual MSI MPG B850 EDGE TI WIFI p. 48), voltage `5 V ARGB` / `12 V RGB`.
- User folder: path in a settings variable; empty -> dialog at the **first save of a user component** (pre-filled default `Documents/Lighting Studio/Components`), existing folder content is read; path changeable in Edit > Settings; folder content is read automatically.
- Save shows `"<name>" saved to <folder>`; failure shows the reason. Nothing is skipped silently (invalid files are named with the reason).
- Editor: Export -> `Show in folder`; Import -> `Add from file…` (validates, copies into the user folder).
- No AI attribution lines in commits. Commits stay local until the owner approves a push.

## Review Focus

1. User folder missing/unwritable (USB removed, Dropbox offline) -> built-ins still work, clear message pointing to Settings; Save fails visibly, draft not lost. Test in Task 3.
2. Two files with the same component ID in the user folder, or a user file reusing a built-in ID -> later file reported and skipped, built-in wins. Test in Task 3.
3. Existing owner data: `custom-components.json` and projects referencing custom IDs -> migrated once, all placements still resolve. Test in Task 3.
4. Service (runs as SYSTEM, cannot read the user folder) plays a project using a user component -> definition travels inside the project. Test in Task 3.
5. Quantity × addresses exceeding the header, or a fixed-count header -> Attach disabled, reason shown in the header row (`exceeds by N`). Test in Task 6.

---

### Task 1: Board labels and connector kind in device profiles

**Files:**
- Modify: `src/modules/DeviceProfile.h:18-25` (ChannelProfile), `src/modules/DeviceProfile.cpp` (fromJson)
- Modify: `profiles/msi-mpg-b850-edge-ti-wifi.json`
- Test: `tests/modules/` driver_modules test file (where DeviceProfile parsing is tested)

**Interfaces:**
- Produces: `ChannelProfile::label` (QString, empty -> use `id`), `ChannelProfile::kind` (QString: `"argb-5v"`, `"rgb-12v"` or empty), free function `QString channelDisplayName(const ChannelProfile&)` returning `label` or `id`, `QString channelKindText(const ChannelProfile&)` returning `"5 V ARGB"`, `"12 V RGB"` or empty.

- [ ] **Step 1:** Test `channelLabelAndKind`: parsing `{"id":"JARGB1","label":"JARGB_V2_1","kind":"argb-5v",...}` gives `channelDisplayName()=="JARGB_V2_1"`, `channelKindText()=="5 V ARGB"`; a channel without the fields gives its `id` and empty kind; unknown `kind` value throws `std::invalid_argument`.
- [ ] **Step 2:** Run, expect FAIL.
- [ ] **Step 3:** Add the fields, parsing and the two functions. Profile: JARGB1-3 get `label` `JARGB_V2_1..3`, `kind` `argb-5v`; JRGB1 gets `kind` `rgb-12v`. Do **not** touch `maxLeds` (120 vs manual 240 is an open question).
- [ ] **Step 4:** Run, expect PASS; full CTest green.
- [ ] **Step 5:** Commit `feat(profiles): printed connector labels and connector kind`.

### Task 2: Devices tree grouped by board

**Files:**
- Modify: `src/editor/LightingWindow.cpp:2160-2200` (tree build), `:676-680` (button text)
- Test: `tests/editor/test_editor.cpp` (new slot `devicesTreeGroupsByBoard`)

**Interfaces:**
- Consumes: Task 1 `channelDisplayName`, `channelKindText`.
- Produces: tree levels board -> connector -> component. Connector items keep `Qt::UserRole+1` header data (selection logic unchanged); board items carry none. Button object name `addHeaderComponents` keeps its name, text becomes `Attach component…`.

- [ ] **Step 1:** Test: document with the B850 profile and two assignments. Assert one top-level item with text `MSI MPG B850 EDGE TI WIFI (MS-7E62)`, children texts start with `JARGB_V2_1`, column 1 contains `5 V ARGB`; the JRGB1 child is disabled (`!(flags & Qt::ItemIsEnabled)`) with tooltip `12 V RGB · not supported yet`; selecting a board item leaves `selectedHeader_` unchanged.
- [ ] **Step 2:** Run, expect FAIL.
- [ ] **Step 3:** Build the board parent per `profileId+path`; connector rows `label · kind`, LEDs column unchanged; non-direct channels without fixed component shown disabled. Peripherals (`lighting.peripherals`/`lighting.auxiliary`) keep their standalone rows under their own device parent.
- [ ] **Step 4:** Run, expect PASS; full CTest green.
- [ ] **Step 5:** Commit `feat(devices): group connectors under their board with printed labels`.

### Task 3: Component library folder (one JSON per component)

**Files:**
- Create: `src/components/ComponentLibrary.h`, `src/components/ComponentLibrary.cpp` (add to the CMake source list next to `ComponentCatalog.cpp`)
- Modify: `src/editor/LightingDocument.cpp:56-90,321-440` (replace `readLibrary`/`writeLibrary` file use), `src/main.cpp:188` (pass the folder instead of `custom-components.json`)
- Test: `tests/editor/test_editor.cpp` slot `componentLibraryFolder` (and adapt `hardwareLibrary`)

**Interfaces:**
- Produces:
  - `struct LibraryIssue { QString file, reason; };`
  - `struct LibraryRead { ComponentCatalog components; QVector<LibraryIssue> issues; };`
  - `LibraryRead readComponentFolder(const QString& folder, const ComponentCatalog& builtIn);` never throws for bad files; reports them.
  - `QString writeComponentFile(const QString& folder, const ComponentDefinition&);` writes `<id>.json` via QSaveFile, returns the path, throws `std::runtime_error("Cannot write <path>: <reason>")`.
  - `int migrateLibraryFile(const QString& legacyJson, const QString& folder);` splits the old array into files, renames the old file to `custom-components.migrated.json`, returns the count.
  - `LightingDocument` constructor parameter `libraryPath` now means a **folder**; `customComponents()` returns `readComponentFolder(...).components`.

- [ ] **Step 1:** Test `componentLibraryFolder` (QTemporaryDir):
  - write two definitions -> files `<id>.json` exist, read back equal (`toJson()` compare);
  - a file with invalid JSON and a file whose ID equals a built-in ID -> both in `issues` with their file names, not in `components`;
  - two files with the same ID -> second reported, first kept;
  - folder missing -> empty result, one issue `folder not found`;
  - `migrateLibraryFile` on a 3-entry array -> returns 3, three files, old file renamed;
  - project saved with a user component, loaded into `LightingDocument(builtInOnly)` -> placement's `componentName()` resolves (service case).
- [ ] **Step 2:** Run, expect FAIL.
- [ ] **Step 3:** Implement; keep the existing 32 MiB per-file limit and the `QLockFile` around writes; keep `reconcileDefinition` semantics.
- [ ] **Step 4:** Run, expect PASS; full CTest green.
- [ ] **Step 5:** Commit `feat(components): one JSON file per user component in a library folder`.

### Task 4: Built-in catalog as files only

**Files:**
- Create: `components/generic.fan.{6,8,12,16}.json`, `components/generic.strip.{8,30,60}.json`, `components/phanteks.m25.paired.json`, `components/phanteks.nv5mk2.internal-strip.json` (exported from the current code, values unchanged)
- Modify: `src/editor/HardwarePicker.cpp:115-150` (`hardwareProducts` loses the hard-coded definitions), `CMakeLists.txt:76-79,93-94` (copy **all** `components/*.json` via `file(GLOB)`), `src/components/ComponentCatalog.h` (optional `QString verification` field: `verified`, `manufacturer`, `unverified`; empty = unverified)
- Test: `tests/editor/test_editor.cpp` slot `builtInCatalogFromFiles`

**Interfaces:**
- Produces: `ComponentDefinition::verification`; built-in catalog = `loadComponentCatalog(<appdir>/components)` only.

- [ ] **Step 1:** Test: loading the source `components/` folder yields `phanteks.m25.paired` with 12 positions, `addresses()==6`, mapping pairs mirrored (position 2k and 2k+1 share address k), `verification=="verified"`; generic IDs present; `hardwareProducts(catalog)` adds nothing beyond `catalog`.
- [ ] **Step 2:** Run, expect FAIL.
- [ ] **Step 3:** Export definitions with a one-off test helper or by hand, delete the code, mark M25, NV5 strip and AK35i V4 `verified`.
- [ ] **Step 4:** Run, expect PASS; full CTest green; packaged ZIP contains all component files.
- [ ] **Step 5:** Commit `refactor(components): built-in catalog only from shipped files`.

### Task 5: Folder setup, settings, messages, editor buttons

**Files:**
- Modify: `src/editor/ComponentEditorPage.cpp` (save path, Import/Export buttons at `:82` area, messages), `src/editor/LightingWindow.cpp:354-380` (Settings dialog row), `src/main.cpp:188`
- Test: `tests/editor/test_editor.cpp` slot `componentFolderSetup`

**Interfaces:**
- Consumes: Task 3 functions.
- Produces: QSettings key `components/userFolder`; `LightingWindow::setComponentFolder(const QString&)` (reloads, emits status); `QFileSystemWatcher` on the folder triggers reload. Buttons: object names `componentShowInFolder`, `componentAddFromFile`.

- [ ] **Step 1:** Test (dialogs stubbed the way existing tests stub `confirm`/`askUnsaved`):
  - empty setting + first Save of a new component -> folder prompt called once with default `<Documents>/Lighting Studio/Components`; after accept the file exists and message equals `"New component" saved to <folder>`;
  - unwritable folder -> message starts `Could not save "New component":`, draft stays dirty;
  - changing the folder in Settings with 2 existing components -> move prompt; Yes -> files in new folder;
  - a file copied into the folder appears in `customComponents()` without restart (watcher);
  - `Add from file…` with a duplicate ID -> asks Replace / Save as copy; invalid file -> message names file and reason.
- [ ] **Step 2:** Run, expect FAIL.
- [ ] **Step 3:** Implement; on first folder setup call `migrateLibraryFile` if `custom-components.json` exists. `Show in folder` opens Explorer with the file selected (`explorer /select,<path>` on Windows, `QDesktopServices::openUrl(folder)` elsewhere).
- [ ] **Step 4:** Run, expect PASS; full CTest green.
- [ ] **Step 5:** Commit `feat(editor): user component folder with first-save setup and settings`.

### Task 6: Attach component dialog

**Files:**
- Modify: `src/editor/HardwarePicker.h`, `src/editor/HardwarePicker.cpp:175-240`, `src/editor/LightingWindow.cpp:2032-2060` (`addHeaderComponents`)
- Test: `tests/editor/test_editor.cpp` slot `attachComponentDialog` (adapt existing picker tests)

**Interfaces:**
- Consumes: Task 1 label/kind, Task 3 user catalog, Task 4 `verification`.
- Produces: `HeaderChoice` gains `QString kind; bool supported=true; QString reason;`. `HardwarePicker(const ComponentCatalog& builtIn, const ComponentCatalog& user, const QVector<HeaderChoice>&, const QVariantMap& selected, QWidget*)`. Object names: `attachHeader` (combo), `attachRemaining` (label), `attachType`, `attachManufacturer`, `attachSearch`, `attachSections` (list), `attachSelectionName`, `attachSelectionAddresses`, `attachQuantity`, `confirmAttach`. Window title `Attach component`.

- [ ] **Step 1:** Test:
  - header combo item for JRGB1 disabled; `attachRemaining` text `114 addresses remaining`;
  - selecting a 6-address component with quantity 2 -> `attachRemaining` `114 → 102 addresses remaining`, `attachSelectionAddresses` `6 addresses`;
  - quantity making 130 on 120 -> text contains `exceeds by`, `confirmAttach` disabled;
  - list shows section `My components` before `Built-in`, items inside sorted by manufacturer then name;
  - search `nucleus` with type `AiO` finds the EK item in Built-in; search `zzz` shows only `+ Create "zzz"`;
  - no `LEDs per component` widget exists.
- [ ] **Step 2:** Run, expect FAIL.
- [ ] **Step 3:** Implement; segmented search row = two compact combos + line edit with magnifier action in one framed widget. Remove `leds_` (generic counts come from quick-create now).
- [ ] **Step 4:** Run, expect PASS; full CTest green.
- [ ] **Step 5:** Commit `feat(attach): rebuilt Attach component dialog`.

### Task 7: Quick-create and editor round trip

**Files:**
- Modify: `src/editor/HardwarePicker.cpp`, `src/editor/ComponentEditorPage.cpp/.h`, `src/editor/LightingWindow.cpp`
- Test: `tests/editor/test_editor.cpp` slot `attachQuickCreate`

**Interfaces:**
- Consumes: Task 5 save path, Task 6 dialog.
- Produces: quick-create form object names `quickType` (tiles: Fans, Strips, Ring, RAM, Other), `quickManufacturer` (editable combo, suggestions from both catalogs), `quickName`, `quickAddresses`, buttons `quickCreateAttach`, `quickOpenEditor`. `ComponentEditorPage::openNewFor(const QVariantMap& header, const QString& name)`; signal `ComponentEditorPage::savedForAttach(QString id)`. Shapes: Fans -> ring, Strips -> line, others -> line.

- [ ] **Step 1:** Test:
  - `+ Create "Test strip"` opens the form with `quickName` = `Test strip`;
  - type Strips, 16 addresses, `quickCreateAttach` -> file in user folder, new item selected in `My components`, remaining reduced by 16, dialog accepted with that component;
  - `quickOpenEditor` -> editor page shown with header banner, Save text `Save & attach`; after save the dialog reopens with the component selected.
- [ ] **Step 2:** Run, expect FAIL.
- [ ] **Step 3:** Implement with `withShape` from `ComponentDraft` (same helpers the editor's shape buttons use).
- [ ] **Step 4:** Run, expect PASS; full CTest green.
- [ ] **Step 5:** Commit `feat(attach): quick-create a component and attach it in one step`.

### Task 8: LED strip from fixed square tiles

Owner idea 2026-11-27: a strip is drawn from two graphic elements, a connector and a square LED tile;
every LED is one tile of fixed size. Moving a single LED away (option b) fills the gap with plain strip
band without an LED; tiles never scale. Replaces the per-neighbour scaling in `drawHardwareEditorArtwork`
(the cause of the oversized segment in the owner's screenshot).

**Files:**
- Create: `assets/hardware/photos/led_strip_tile_fv.png` (256x256), `led_strip_band_fv.png` (256x256), `led_strip_connector_fv.png` (272x256), owner-supplied 2026-11-27; register in `assets/hardware/hardware.qrc`
- Modify: `src/editor/HardwareArtwork.cpp` (strip branch of `drawHardwareEditorArtwork`), `src/editor/ComponentEditorPage.cpp` (add-LED on a strip)
- Test: `tests/editor/test_editor.cpp` slot `stripFixedTiles`

**Interfaces:**
- Produces: constant `kStripTile` = the Strip shape's LED spacing from `withShape` (component units). Sources are the three tile images, loaded without the alpha-bbox auto-crop. The owner's second export (2026-11-27 15:49) is near-opaque (alpha >= 230, edge columns ~246); draw from the inner source rect `[2, 254)` on both axes anyway so no residual edge shows as a seam; the connector's right edge joins the tile's left edge. The LED colour stays the image's neutral warm-white; live colour is out of scope.

- [ ] **Step 1:** Test: 16-LED strip at the Strip-shape spacing renders 16 tiles of identical pixel width (± 1 px) with no light seam column between neighbours; moving LED 9 by 3 tiles keeps every tile width unchanged and the gap pixels are non-transparent (band); adding an LED on a strip puts it at `last + kStripTile` along the last segment's direction, not at the click point.
- [ ] **Step 2:** Run, expect FAIL.
- [ ] **Step 3:** Draw connector before LED 0, one fixed tile centred on each LED, band slices along each gap longer than `kStripTile`, rotated per segment direction.
- [ ] **Step 4:** Run, expect PASS; full CTest green.
- [ ] **Step 5:** Commit `feat(editor): LED strip drawn from fixed square tiles`.

### Task 9: Component editor cleanup

Owner requests 2026-11-27: hover coordinates are useless, Stop/Play has no function in the Component Editor,
a switchable Align is essential (as in the Layout editor), the AK35i LED arrangement should follow the Roccat Vulcan.

**Files:**
- Modify: `src/editor/ComponentCanvas.cpp:190` (LED tooltip), `src/editor/ComponentCanvas.h/.cpp` (new `alignSelection`), `src/editor/LightingWindow.cpp:1627-1633` (route the `Align / Snap` menu to the component canvas while the Component Editor page is open), `:1697` (hide `playPreview` there), `src/editor/ComponentEditorPage.h` (accessor for the canvas if none exists)
- Modify: `components/peripheral-ajazz-ak35i-v4.json` (positions only; `mapping` and wire IDs unchanged)
- Test: `tests/editor/test_editor.cpp` slot `componentEditorCleanup`

**Interfaces:**
- Produces: `void ComponentCanvas::alignSelection(const QString& alignment)` with the same keys as `LightingCanvas::alignSelection` (`left`, `center`, `right`, `top`, `middle`, `bottom`, `distribute-horizontal`, `distribute-vertical`), acting on selected LEDs of the active layer, one undo step `Align LEDs`; fewer than 2 (align) / 3 (distribute) selected -> the same error texts as the Layout canvas. Snapping keeps using the shared `SnapGrid` switch in the same menu.
- AK35i positions: group LEDs into 6 rows (the two LEDs at y .7164 belong to the row above for Num+ (x 5.3731) and the row below for Enter (x 3.459); y 1.209 Num Enter joins y 1.0821), sort each row by x, assign the Vulcan's physical positions of the same row sorted by x (row sizes 16/21/20/17/18/13 match).

- [ ] **Step 1:** Test: LED tooltip is `LED 3` (no coordinates; layer suffix kept); with the Component Editor page open `playPreview` is hidden and visible again after leaving; selecting 3 LEDs and triggering `align_top` sets their y to the smallest y in one undo step, `distribute-horizontal` spaces x evenly; AK35i positions as a set equal the Vulcan's physical positions and the `mapping` array is unchanged.
- [ ] **Step 2:** Run, expect FAIL.
- [ ] **Step 3:** Implement.
- [ ] **Step 4:** Run, expect PASS; full CTest green.
- [ ] **Step 5:** Commit `feat(editor): component editor align, no coordinate tooltips, no preview transport; AK35i layout from Vulcan grid`.

### Task 10: Docs, build, delivery

**Files:** `docs/LIGHTING_EDITOR.md`, `docs/HARDWARE_MODULES.md` (catalog schema: `verification`, folder layout), `CONTINUE.md`, `src/editor/LightingWindow.cpp:81` (`studioBuildId`).

- [ ] **Step 1:** Update docs for tree, Attach dialog, user folder, migration.
- [ ] **Step 2:** Bump build id, build, full CTest green, `graphify update .`.
- [ ] **Step 3:** Package `Downloads/LightingStudio-local-rNN.zip` (`scripts/package_release.py --runtime build/r36/Release --folder LightingStudio-rNN --no-checksum`).
- [ ] **Step 4:** Commit `docs: attach flow and component library`. No push without owner approval.

## Not in this plan (noted for later)

- `maxLeds` 120 vs manual 240 for JARGB_V2 (check protocol source first).
- Live JRGB control (needs 12 V test hardware).
- Motherboard front-view photo (owner provides `motherboard_rgb_fv.png`), photos on the main canvas.
- Power warning per header (needs hub/controller power info, e.g. NV5 MKII controller 5 V / 7 A).

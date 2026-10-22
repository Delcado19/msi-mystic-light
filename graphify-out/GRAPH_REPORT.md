# Graph Report - lighting-studio  (2026-10-22)

## Corpus Check
- 115 files · ~100,580 words
- Verdict: corpus is large enough that graph structure adds value.
- Unclassified: 4 file(s) not represented in the graph (top: (none) 3, .qrc 1)

## Summary
- 1805 nodes · 4121 edges · 111 communities (81 shown, 30 thin omitted)
- Extraction: 81% EXTRACTED · 19% INFERRED · 0% AMBIGUOUS · INFERRED: 796 edges (avg confidence: 0.85)
- Token cost: 0 input · 0 output

## Graph Freshness
- Built from commit: `c414db83`
- Run `git rev-parse HEAD` and compare to check if the graph is stale.
- Run `graphify update .` after code changes (no API cost).

## Community Hubs (Navigation)
- LayoutEditor
- MsiColor
- Lighting Studio — Design Doc (v2)
- TrayApp
- MSI mainboard LED protocol, "800-series" generation (VID `0x0DB0`/PID `0x0076`)
- test_canvas.cpp
- IDriver
- RazerBasiliskDriver
- test_settings.cpp
- CustomEffectDefinition
- DeviceLedWidget
- test_effects.cpp
- test_fake_hid_device.cpp
- AutostartWin
- MsiLightingMode
- .open
- LightingStudioWindow.cpp
- test_steelseries_apex3_driver.cpp
- MsiGen1AdvArea
- ComponentTransformHandle
- cmath
- MsiMotherboardDriver
- Channel
- LightingCanvasWidget
- GigabyteAeroDriver
- .makeGenericPlaceholder
- Settings.cpp
- Settings
- BuildReport0x50
- test_lighting_canvas_widget.cpp
- ChannelSettings
- NativeModeSettings
- test_lighting_studio_window.cpp
- MsiZoneSettings
- IHidDevice
- .EffectEditor
- SwirlCirclesEffect
- effectTimeMs
- LightingStudioWindow
- RoccatVulcanDriver
- test_razer_basilisk_driver.cpp
- Standalone RGB Lighting App — Design
- CustomGradientEffect
- BuildVulcanColorPackets
- Lighting Studio Implementation Plan
- SpiralEffect
- GradientStops
- HidApiDevice
- LightingStudioWindow.h
- Component
- Mystic Light — local network interfaces
- vcpkg.json
- algorithm
- DragTarget
- Lighting Studio UI redesign
- OpenRgbRamDriver.h
- README.md
- Mystic Light — Overall Architecture
- MSI Mystic Light — Reverse Engineering
- test_roccat_vulcan_driver.cpp
- `Mystic Light Online Data.dat` — decryption and content
- ChannelMode
- RoccatKoneDriver
- Component chains, device regions, and effects
- SteelSeriesApex3Driver
- EffectEditor.h
- .saveToFile
- test_roccat_kone_driver.cpp
- CanvasPlacement
- I2cInfo
- FakeDriver
- main.cpp
- EffectFactory.h
- qttest
- HardwareWorker
- EffectPreview
- ComponentHit
- .fromJson
- DeviceArtwork.h
- EffectEditor
- Compact controls and direct zone editing (2026-10-22)
- HardwarePicker.h
- QPushButton
- DeviceLedWidget.h
- ChannelAssignment
- snappedLayoutPosition
- Self-Review
- Installed lighting hardware
- ChannelInfo
- CometEffect
- LightingCanvasWidget.h
- .colorAt
- IPositionableEffect

## God Nodes (most connected - your core abstractions)
1. `LightingStudioWindow` - 115 edges
2. `MsiColor` - 81 edges
3. `Component` - 74 edges
4. `LightingCanvasWidget` - 71 edges
5. `EffectEditor` - 68 edges
6. `IEffect` - 51 edges
7. `LightingStudioWindowTest` - 51 edges
8. `CustomEffectDefinition` - 48 edges
9. `ChannelSettings` - 41 edges
10. `LightingStudioWindow::LightingStudioWindow()` - 41 edges

## Surprising Connections (you probably didn't know these)
- `Decisions already made (do not re-derive)` --references--> `Component`  [INFERRED]
  docs/superpowers/specs/2026-10-19-lighting-studio-design.md → src/device/Component.h
- `Effect library (staged)` --references--> `IEffect`  [INFERRED]
  docs/superpowers/specs/2026-10-19-lighting-studio-design.md → src/effects/IEffect.h
- `Report `0x51` ("Gen1_Adv"), 727 bytes — per-LED addressing of individual segments` --references--> `MsiZoneSettings`  [INFERRED]
  docs/MB_800_HID_PROTOCOL.md → src/protocol/MsiProtocol.h
- `What's left to the builder` --references--> `Settings`  [INFERRED]
  docs/superpowers/specs/2026-10-19-lighting-studio-design.md → src/settings/Settings.h
- `Task 13: Extend `Settings` — placement, source, global effect, profiles` --references--> `CanvasSource`  [INFERRED]
  docs/superpowers/plans/2026-10-19-lighting-studio.md → src/canvas/Canvas.h

## Import Cycles
- None detected.

## Communities (111 total, 30 thin omitted)

### Community 0 - "LayoutEditor"
Cohesion: 0.11
Nodes (7): LayoutEditor, bodies_, components_, points_, scene_, selection_, view_

### Community 1 - "MsiColor"
Cohesion: 0.08
Nodes (22): Task 7: Effect interface and built-in effects, BreathingEffect, BreathingEffect::BreathingEffect(), color_, periodMs_, IEffect, MarqueeEffect, color_ (+14 more)

### Community 2 - "Lighting Studio — Design Doc (v2)"
Cohesion: 0.14
Nodes (13): Anti-goals, Canvas architecture, Decisions already made (do not re-derive), Device roster (confirmed), Effect library (staged), Effect reference points, Goals, Intent and why (+5 more)

### Community 3 - "TrayApp"
Cohesion: 0.08
Nodes (14): Task 10: Qt tray app shell, Task 16: Tray integration — open Lighting Studio, profile quick-switch, refreshTrayProfiles(), QActionGroup, TrayApp, menu_, profileGroup_, profileMenu_ (+6 more)

### Community 4 - "MSI mainboard LED protocol, "800-series" generation (VID `0x0DB0`/PID `0x0076`)"
Cohesion: 0.18
Nodes (11): MSI mainboard LED protocol, "800-series" generation (VID `0x0DB0`/PID `0x0076`), Record layout (16 bytes, per zone/"area"), Report `0x50` ("Gen1"), 290 bytes — status and default configuration, Report `0x51` ("Gen1_Adv"), 727 bytes — per-LED addressing of individual segments, Report `0x90`+port / `0xB0`+port ("Gen2"/"Gen2_Adv") — newer strip model, ⚠️ Safety-relevant finding: the `Store` flag is set on every write, Standalone app capability correction (2026-10-21), Upstream OpenRGB bug found while cross-checking: `FeaturePacket_PerLED_761` is 727 bytes, not 761 (+3 more)

### Community 5 - "test_canvas.cpp"
Cohesion: 0.20
Nodes (6): Task 3: Canvas-global placement and "follow global effect", allChannelsTo(), CanvasTest, private, CanvasTest::testResizedRotatedGeometryAndRawCalibration(), PositionColorEffect

### Community 6 - "IDriver"
Cohesion: 0.15
Nodes (11): IDriver, applyChannelModeToDriver(), applyChannelSettingsToCanvas(), applyGlobalEffectToCanvas(), main(), reapplyChannelsForDriver(), ReconnectiblePeripheral, driver (+3 more)

### Community 7 - "RazerBasiliskDriver"
Cohesion: 0.09
Nodes (11): Task 11: Razer Basilisk V3 driver, RazerBasiliskDriver, RazerBasiliskDriver::RazerBasiliskDriver(), BuildBasiliskColorsReport(), BuildBasiliskModeCustomReport(), CalculateCrc(), RazerBasiliskDriverTest::testPushCanvasColors_sendsBothReportsInOrder(), toVec() (+3 more)

### Community 8 - "test_settings.cpp"
Cohesion: 0.13
Nodes (4): Task 13: Extend `Settings` — placement, source, global effect, profiles, SettingsTest, private, SettingsTest::testComponentsOverridesAndCustomEffectsRoundTrip()

### Community 9 - "CustomEffectDefinition"
Cohesion: 0.07
Nodes (23): CustomEffectDefinition, angle, blend, centerX, centerY, colors, enabled, layers (+15 more)

### Community 10 - "DeviceLedWidget"
Cohesion: 0.09
Nodes (12): DeviceLedWidget, components_, editable_, effect_, global_, hits_, overrides_, paused_ (+4 more)

### Community 11 - "test_effects.cpp"
Cohesion: 0.13
Nodes (4): Task 5: `BreathingEffect`, Task 7: `CometEffect` and `MarqueeEffect`, EffectsTest, private

### Community 12 - "test_fake_hid_device.cpp"
Cohesion: 0.18
Nodes (3): FakeHidDeviceTest, private, FakeHidDeviceTest::testOpen_defaultInterfaceNumberIsNegativeOne()

### Community 13 - "AutostartWin"
Cohesion: 0.11
Nodes (9): Task 11: Windows autostart registration, AutostartWin, AutostartWin::AutostartWin(), exePath_, valueName_, IAutostart, AutostartTest, private (+1 more)

### Community 14 - "MsiLightingMode"
Cohesion: 0.17
Nodes (12): MsiLightingMode, Advanced, Breathing, ColorRing, Flame, GodLike, Lightning, Meteor (+4 more)

### Community 15 - ".open"
Cohesion: 0.20
Nodes (3): MsiMotherboardDriverTest, private, MsiMotherboardDriverTest::testSetNativeMode_sendsExpectedBytesAndPreservesOtherZones()

### Community 16 - "LightingStudioWindow.cpp"
Cohesion: 0.11
Nodes (12): effectDisplayName(), compactHardwareName(), effectUsesColor(), LightingStudioWindow::LightingStudioWindow(), makeCombo(), modeName(), QComboBox, toMsiColor() (+4 more)

### Community 18 - "MsiGen1AdvArea"
Cohesion: 0.11
Nodes (19): MsiGen1AdvArea, JAF, JARGB1, JARGB2, JARGB3, JPIPE1, JPIPE2, JPIPE3 (+11 more)

### Community 19 - "ComponentTransformHandle"
Cohesion: 0.15
Nodes (9): ComponentTransformHandle, angle_, center_, distance_, owner_, rotate_, rotation_, scale_ (+1 more)

### Community 20 - "cmath"
Cohesion: 0.18
Nodes (7): Task 4: `IPositionableEffect` + `RadialRainbowEffect`, Task 6: `SpiralEffect`, HsvFullToRgb(), RadialRainbowEffect, centerX_, centerY_, periodMs_

### Community 21 - "MsiMotherboardDriver"
Cohesion: 0.14
Nodes (7): MsiMotherboardDriver, channelModes_, MsiMotherboardDriver::MsiMotherboardDriver(), shieldColor_, shieldLastSend_, shieldSent_, zoneState_

### Community 22 - "Channel"
Cohesion: 0.17
Nodes (6): Spatial model (extends, doesn't replace, v1's effect model), Channel, Channel::Channel(), components_, name_, ChannelTest::testFanChainsAndAtomicReplacement()

### Community 23 - "LightingCanvasWidget"
Cohesion: 0.05
Nodes (25): LightingCanvasWidget, channels_, componentCenter_, componentGrab_, componentHits_, dragComponents_, draggedChannel_, dragKind_ (+17 more)

### Community 24 - "GigabyteAeroDriver"
Cohesion: 0.11
Nodes (14): aeroColorPacket(), GigabyteAeroDriver, attempted_, failed_, handle_, kChannel, lastSend_, library_ (+6 more)

### Community 25 - ".makeGenericPlaceholder"
Cohesion: 0.29
Nodes (4): colors_, makeGridComponent(), ChannelTest, private

### Community 26 - "Settings.cpp"
Cohesion: 0.26
Nodes (11): channelSettingsFromJson(), clampLightingMode(), clampPlacementCoord(), colorEquals(), colorFromJson(), globalEffectSettingsFromJson(), nativeSettingsEquals(), nativeSettingsFromJson() (+3 more)

### Community 27 - "Settings"
Cohesion: 0.13
Nodes (15): Application structure, GlobalEffectSettings, color, effectName, referenceX, referenceY, Profile, channelSettings (+7 more)

### Community 28 - "BuildReport0x50"
Cohesion: 0.17
Nodes (7): Task 12: Application wire-up and first real-hardware verification, Task 6: MSI motherboard driver, BuildReport0x50(), BuildReport0x51(), MsiProtocolTest, private, MsiProtocolTest::testBuildReport0x50_defaultZonesAllZero()

### Community 29 - "test_lighting_canvas_widget.cpp"
Cohesion: 0.10
Nodes (7): Task 14: `LightingCanvasWidget`, FakeEffect, FakePositionableEffect, x_, y_, LightingCanvasWidgetTest, private

### Community 30 - "ChannelSettings"
Cohesion: 0.14
Nodes (13): channelSettingsNeedFullReapply(), ChannelSettings, channelName, components, componentsConfigured, effectName, layoutVisible, ledOverrides (+5 more)

### Community 31 - "NativeModeSettings"
Cohesion: 0.14
Nodes (14): Task 1: Extract shared device value types, add `IDriver`, NativeModeSettings, brightness, color1, color2, color3, color4, colorCount (+6 more)

### Community 32 - "test_lighting_studio_window.cpp"
Cohesion: 0.09
Nodes (5): Task 15: `LightingStudioWindow`, dragBoxAt(), LightingStudioWindowTest, private, makeSettingsWithOneProfile()

### Community 33 - "MsiZoneSettings"
Cohesion: 0.15
Nodes (13): MsiZoneSettings, brightness, color1, color2, color3, color4, colorCount, colorSelectionUser (+5 more)

### Community 34 - "IHidDevice"
Cohesion: 0.10
Nodes (11): Task 2: `IHidDevice` gains `write()` and interface-aware `open()`, IHidDevice, FakeHidDevice, opened_, openInterfaceNumber, openSucceeds, openUsagePage, sendSucceeds (+3 more)

### Community 36 - "SwirlCirclesEffect"
Cohesion: 0.16
Nodes (6): Task 8: `SwirlCirclesEffect`, SwirlCirclesEffect, centerX_, centerY_, periodMs_, ringWidthPx_

### Community 37 - "effectTimeMs"
Cohesion: 0.13
Nodes (6): effectTimeMs(), Hit, index, name, rect, LightingCanvasWidget::LightingCanvasWidget()

### Community 38 - "LightingStudioWindow"
Cohesion: 0.05
Nodes (26): Task 18: Final wire-up and real-hardware verification, LightingStudioWindow, availableModesByChannel_, canvasWidget_, channelNames_, customEffectCombo_, defaultComponents_, deleteProfileButton_ (+18 more)

### Community 42 - "Standalone RGB Lighting App — Design"
Cohesion: 0.20
Nodes (9): Anti-goals, Constraints, Goals, How it gets made (decided), Intent and why, Non-goals (v1), Settled during review (builder-check pass), Standalone RGB Lighting App — Design (+1 more)

### Community 43 - "CustomGradientEffect"
Cohesion: 0.18
Nodes (4): CustomGradientEffect, definition_, layers_, beforePaint

### Community 44 - "BuildVulcanColorPackets"
Cohesion: 0.18
Nodes (5): BuildVulcanColorPackets(), BuildVulcanEnableDirectReport(), RoccatVulcanProtocolTest, private, RoccatVulcanProtocolTest::testBuildVulcanColorPackets_packetCountAndHeader()

### Community 45 - "Lighting Studio Implementation Plan"
Cohesion: 0.33
Nodes (5): Global Constraints, Lighting Studio Implementation Plan, Review Focus, Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces), Task 17: `LICENSE` and README update

### Community 47 - "SpiralEffect"
Cohesion: 0.18
Nodes (5): SpiralEffect, centerX_, centerY_, periodMs_, tightnessDegPerPx_

### Community 48 - "GradientStops"
Cohesion: 0.21
Nodes (5): GradientStops, changed, dragging_, recipe, selected

### Community 49 - "HidApiDevice"
Cohesion: 0.21
Nodes (4): hid_device_, HidApiDevice, closeOnIoError_, handle_

### Community 50 - "LightingStudioWindow.h"
Cohesion: 0.09
Nodes (18): PatternLibrary, ChannelRow, colorButton, components, componentSignature, effectCombo, form, hardwarePreview (+10 more)

### Community 53 - "Component"
Cohesion: 0.09
Nodes (15): Task 4: Component data model, Component, brand, displayName, height, imageUrl, layoutDefaults, ledCoordinates (+7 more)

### Community 54 - "Mystic Light — local network interfaces"
Cohesion: 0.25
Nodes (8): 1. TCP 26820 — local HTTP "Web API" (`MLModule.dll:CWebAPI.cs`), 2. UDP 26822 — mobile app pairing (`CBaseServer.cs`/`CBaseFN.cs`), 3. UDP 26823 — "party sync" between multiple PCs (`CSyncServer.cs`/`CSyncFN.cs`), 4. UDP 61337 — SignalRGB bridge ("LEDKeeper 2" service), Live verification (2026-10-17): the whole MLModule network layer is dead code, Mystic Light — local network interfaces, Protocol (confirmed via code, extends the `Msi.js` reconstruction), Still open

### Community 55 - "vcpkg.json"
Cohesion: 0.40
Nodes (4): builtin-baseline, dependencies, name, version

### Community 56 - "algorithm"
Cohesion: 0.10
Nodes (8): Canvas::Canvas(), LayoutBounds, bottom, left, right, top, layoutSamplePoint(), instantiateHardware()

### Community 57 - "DragTarget"
Cohesion: 0.22
Nodes (9): DragTarget, Channel, ComponentMove, ComponentResize, ComponentRotate, GlobalReferencePoint, None, Resize (+1 more)

### Community 58 - "Lighting Studio UI redesign"
Cohesion: 0.11
Nodes (18): Audit and resulting changes, Circular centers, numbered previews and effect labels (2026-10-22), Component/region follow-up, Contracts preserved, Corrected layout intent, Design thesis, Effect semantics and individual rotation correction (2026-10-22), Layered effects and readable hardware (2026-10-22) (+10 more)

### Community 59 - "OpenRgbRamDriver.h"
Cohesion: 0.07
Nodes (16): Device, id, info, OpenRgbRamDriver, devices_, socket_, parseRamSdkDescriptor(), ramSdkColorPayload() (+8 more)

### Community 61 - "Mystic Light — Overall Architecture"
Cohesion: 0.29
Nodes (7): Components, HID password mechanism (`HID_Basic.init`), LEDKeeper2.exe, MLModule.dll — network core, Mystic Light — Overall Architecture, Mystic_Light_Service.exe, SyncData.dll

### Community 62 - "MSI Mystic Light — Reverse Engineering"
Cohesion: 0.22
Nodes (9): Board lighting limitations and defaults, Components, keys and custom effects, Contents, Installed hardware and layout grid, License, Lighting Studio interface, MSI Mystic Light — Reverse Engineering, Related repos (+1 more)

### Community 63 - "test_roccat_vulcan_driver.cpp"
Cohesion: 0.18
Nodes (4): RoccatVulcanDriverTest, private, RoccatVulcanDriverTest::testPushCanvasColors_sendsPacketsToLedHidNotControlHid(), toVec()

### Community 64 - "`Mystic Light Online Data.dat` — decryption and content"
Cohesion: 0.40
Nodes (5): Content, Decryption recipe (read-only, nothing sent to any device), `Mystic Light Online Data.dat` — decryption and content, Our board (MS-7E62, i.e. the MSI MPG B850 Edge TI WiFi), Scope: how many other boards share this exact protocol

### Community 65 - "ChannelMode"
Cohesion: 0.36
Nodes (3): ChannelMode, Canvas, Native

### Community 66 - "RoccatKoneDriver"
Cohesion: 0.11
Nodes (8): Task 10: Roccat Kone Aimo driver, RoccatKoneDriver, RoccatKoneDriver::RoccatKoneDriver(), BuildKoneColorsReport(), BuildKoneInitReport(), RoccatKoneProtocolTest, private, RoccatKoneProtocolTest::testBuildKoneInitReport_layout()

### Community 67 - "Component chains, device regions, and effects"
Cohesion: 0.18
Nodes (11): Component chains, device regions, and effects, Configure a motherboard header, Effect persistence and chain workflow correction (2026-10-21), Evidence and remaining checks, Focused workspace and artwork revision, Keys and LED regions, Smooth and custom effects, Spatial canvas correction and channel picker (+3 more)

### Community 69 - "SteelSeriesApex3Driver"
Cohesion: 0.10
Nodes (10): Task 9: SteelSeries Apex 3 driver (T-Zone protocol), SteelSeriesApex3Driver, SteelSeriesApex3Driver::SteelSeriesApex3Driver(), BuildApex3BrightnessPacket(), BuildApex3ColorsPacket(), SteelSeriesApex3DriverTest::testPushCanvasColors_sendsBothPacketsInOrder(), toVec(), SteelSeriesApex3ProtocolTest (+2 more)

### Community 70 - "EffectEditor.h"
Cohesion: 0.16
Nodes (3): LayoutView, LedPoint, QCheckBox

### Community 72 - ".saveToFile"
Cohesion: 0.30
Nodes (8): channelSettingsToJson(), colorToJson(), globalEffectSettingsToJson(), nativeSettingsToJson(), placementToJson(), profileToJson(), sourceToJson(), writeEffectLayers()

### Community 73 - "test_roccat_kone_driver.cpp"
Cohesion: 0.17
Nodes (4): RoccatKoneDriverTest, private, RoccatKoneDriverTest::testPushCanvasColors_sendsBothReportsInOrder(), toVec()

### Community 74 - "CanvasPlacement"
Cohesion: 0.18
Nodes (10): CanvasPlacement, height, rotation, width, x, y, ChannelBox, ownEffect (+2 more)

### Community 75 - "I2cInfo"
Cohesion: 0.18
Nodes (11): I2cInfo, address, ddc, displayMask, port, portSet, reg, regSize (+3 more)

### Community 76 - "FakeDriver"
Cohesion: 0.36
Nodes (3): FakeDriver, nativeModeChannels, pushedChannels

### Community 78 - "EffectFactory.h"
Cohesion: 0.24
Nodes (5): buildEffect(), effectDefinition(), isBuiltInEffect(), StaticColorEffect::StaticColorEffect(), readEffectLayers()

### Community 81 - "EffectPreview"
Cohesion: 0.08
Nodes (15): EffectPreview, caption, centerDragged, draggingCenter_, drawingZone_, drawZoneMode, patternDropped, paused_ (+7 more)

### Community 82 - "ComponentHit"
Cohesion: 0.40
Nodes (4): ComponentHit, channel, index, rect

### Community 83 - ".fromJson"
Cohesion: 0.22
Nodes (4): truncateOrThrow(), ComponentTest, private, ComponentTest::testShapesResetAndMappingValidation()

### Community 89 - "DeviceArtwork.h"
Cohesion: 0.12
Nodes (10): deviceArtworkKind(), drawDeviceArtwork(), hardwareCategory(), hardwareLightingDescription(), hardwareIcon(), HardwarePicker, count_, customRequested (+2 more)

### Community 91 - "EffectEditor"
Cohesion: 0.05
Nodes (35): EffectEditor, angle_, colors_, direction_, durationRow_, editing_, error_, form_ (+27 more)

### Community 92 - "Compact controls and direct zone editing (2026-10-22)"
Cohesion: 0.33
Nodes (6): Canvas drag regression, Compact controls and direct zone editing (2026-10-22), Device eyes, complete single-zone artwork and RAM output, Gaming visual hierarchy and action sizing, Layer identity and layout scroll bounds, Maximized layout, grid and installed hardware

### Community 93 - "HardwarePicker.h"
Cohesion: 0.15
Nodes (7): ColorPicker, hex_, rgb_, syncing_, value_, wheel_, QLabel

### Community 94 - "QPushButton"
Cohesion: 0.33
Nodes (3): QPushButton, compactStudioControlsStyle(), sizeStudioAction()

### Community 95 - "DeviceLedWidget.h"
Cohesion: 0.16
Nodes (3): ColorWheel, changed, color

### Community 96 - "ChannelAssignment"
Cohesion: 0.12
Nodes (18): Self-Review Notes, Canvas, assignments_, driversByChannel_, globalEffect_, nativeModePending_, CanvasSource, GlobalFollow (+10 more)

### Community 102 - "Self-Review"
Cohesion: 0.17
Nodes (11): File Structure, Global Constraints, Review Focus, Self-Review, Standalone Lighting App — v1 Skeleton Implementation Plan, Task 1: Project scaffolding and test harness, Task 2: MSI wire-protocol encoders, Task 3: HID transport interface and hidapi backend (+3 more)

### Community 103 - "Installed lighting hardware"
Cohesion: 0.40
Nodes (5): Installed lighting hardware, Layout editor, Layout visibility and model labels, Sources, Validation limits

### Community 104 - "ChannelInfo"
Cohesion: 0.40
Nodes (4): ChannelInfo, gen1AdvArea, ledLimit, zoneIndex

### Community 105 - "CometEffect"
Cohesion: 0.20
Nodes (6): CometEffect, color_, CometEffect::CometEffect(), speedPxPerMs_, tailLengthPx_, trackWidthPx_

## Knowledge Gaps
- **490 isolated node(s):** `Independent`, `GlobalFollow`, `y`, `width`, `height` (+485 more)
  These have ≤1 connection - possible missing edges or undocumented components. (Counts symbols only; 757 node(s) total have ≤1 connection when file, concept and rationale nodes are included.)
- **30 thin communities (<3 nodes) omitted from report** — run `graphify query` to explore isolated nodes.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **Why does `MsiColor` connect `MsiColor` to `test_canvas.cpp`, `RazerBasiliskDriver`, `test_settings.cpp`, `CustomEffectDefinition`, `DeviceLedWidget`, `test_effects.cpp`, `LightingStudioWindow.cpp`, `cmath`, `MsiMotherboardDriver`, `Channel`, `LightingCanvasWidget`, `GigabyteAeroDriver`, `.makeGenericPlaceholder`, `Settings.cpp`, `Settings`, `BuildReport0x50`, `test_lighting_canvas_widget.cpp`, `ChannelSettings`, `NativeModeSettings`, `test_lighting_studio_window.cpp`, `MsiZoneSettings`, `SwirlCirclesEffect`, `RoccatVulcanDriver`, `CustomGradientEffect`, `BuildVulcanColorPackets`, `vector`, `SpiralEffect`, `OpenRgbRamDriver.h`, `RoccatKoneDriver`, `SteelSeriesApex3Driver`, `.saveToFile`, `FakeDriver`, `EffectFactory.h`, `ChannelAssignment`, `Self-Review`, `CometEffect`, `.colorAt`?**
  _High betweenness centrality (0.210) - this node is a cross-community bridge._
- **Why does `Component` connect `Component` to `LayoutEditor`, `Lighting Studio — Design Doc (v2)`, `test_canvas.cpp`, `DeviceLedWidget`, `LightingStudioWindow.cpp`, `Channel`, `LightingCanvasWidget`, `.makeGenericPlaceholder`, `Settings`, `ChannelSettings`, `LightingStudioWindow`, `Lighting Studio Implementation Plan`, `vector`, `algorithm`, `EffectEditor.h`, `main.cpp`, `.fromJson`, `DeviceArtwork.h`, `HardwarePicker.h`, `DeviceLedWidget.h`, `Self-Review`?**
  _High betweenness centrality (0.104) - this node is a cross-community bridge._
- **Why does `CustomEffectDefinition` connect `CustomEffectDefinition` to `MsiColor`, `.EffectEditor`, `LightingStudioWindow`, `.saveToFile`, `EffectEditor`, `CustomGradientEffect`, `.colorAt`, `EffectFactory.h`, `vector`, `GradientStops`, `EffectPreview`, `LightingStudioWindow.cpp`, `Settings`?**
  _High betweenness centrality (0.076) - this node is a cross-community bridge._
- **Are the 33 inferred relationships involving `LightingStudioWindow` (e.g. with `Self-Review Notes` and `Task 13: Extend `Settings` — placement, source, global effect, profiles`) actually correct?**
  _`LightingStudioWindow` has 33 INFERRED edges - model-reasoned connections that need verification._
- **Are the 9 inferred relationships involving `MsiColor` (e.g. with `Self-Review` and `Task 2: MSI wire-protocol encoders`) actually correct?**
  _`MsiColor` has 9 INFERRED edges - model-reasoned connections that need verification._
- **Are the 13 inferred relationships involving `Component` (e.g. with `Task 4: Component data model` and `Task 5: Channel model`) actually correct?**
  _`Component` has 13 INFERRED edges - model-reasoned connections that need verification._
- **What connects `Independent`, `GlobalFollow`, `y` to the rest of the system?**
  _490 weakly-connected nodes found - possible documentation gaps or missing edges._
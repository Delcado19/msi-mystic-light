# Lighting Studio Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Replace the settings.json-only v1 app with a real UI ("Lighting Studio") built around a SignalRGB-style canvas, and extend control from the motherboard alone to four peripherals already owned (Roccat Vulcan 100/120 Aimo keyboard, Roccat Kone Aimo/16K mouse, Razer Basilisk V3 mouse, SteelSeries Apex 3 keyboard).

**Architecture:** Generalizes v1's existing `Channel`/`Component`/`IEffect`/`Canvas` model (which already does local per-LED coordinate sampling) with two additions: an `IDriver` abstraction so `Canvas` can drive five independent devices instead of one, and a canvas-global placement + "follow the global effect" mode layered on top of the existing per-channel Canvas/Native split. The new UI is plain Qt Widgets (`QPainter`-based canvas widget), not Qt Quick.

**Tech Stack:** C++17, Qt6 (Core, Widgets, Test), hidapi (vcpkg), same toolchain as v1.

**Spec:** `docs/superpowers/specs/2026-10-19-lighting-studio-design.md`

## Global Constraints

- Combined binary license becomes **GPL-2.0-or-later** once the OpenRGB-derived peripheral drivers land (Qt continues under its own LGPL dynamic-linking terms unchanged).
- No vendor software dependency: no Razer Synapse/Chroma Connect, no SteelSeries GG/Prism. Every device is driven directly over HID, the same way v1 already drives the motherboard.
- Peripherals are **direct-mode only**: lighting is shown only while this app is running; no flash-write/persistence protocol to any peripheral is implemented in this pass.
- A peripheral region in "follow global effect" mode samples **per individually addressable LED/zone**, not one averaged color for the whole device.
- `JRGB1` stays Native-mode-only (unchanged from v1); it never participates in "follow global effect".
- Out of scope for this pass: audio-reactive effects, GIF/image playback, layer/compositing effects, SignalRGB-style game integrations, any device/controller beyond the five already named.
- Real-hardware safety gate (carried from v1, unchanged): no unconfirmed byte sequence reaches real hardware without first showing the exact bytes and getting explicit plain-language human confirmation.
- Device identifiers (from OpenRGB's source, confirmed by reading it, not assumed):

  | Device | VID | PID | OpenRGB source (GPL-2.0-or-later, this app's reference) |
  |---|---|---|---|
  | Roccat Vulcan 100 Aimo | `0x1E7D` | `0x307A` | `Controllers/RoccatController/RoccatVulcanKeyboardController/RoccatVulcanKeyboardController.cpp` |
  | Roccat Vulcan 120 Aimo | `0x1E7D` | `0x3098` | same file |
  | Roccat Kone Aimo | `0x1E7D` | `0x2E27` | `Controllers/RoccatController/RoccatKoneAimoController/RoccatKoneAimoController.cpp` |
  | Roccat Kone Aimo 16K | `0x1E7D` | `0x2E2C` | same file |
  | Razer Basilisk V3 | `0x1532` | `0x0099` | `Controllers/RazerController/RazerController/RazerController.cpp` |
  | SteelSeries Apex 3 | `0x1038` | `0x161A` | `Controllers/SteelSeriesController/SteelSeriesApexTZoneController/SteelSeriesApexTZoneController.cpp` |

  Exact PID must still be confirmed against the real connected device (e.g. via Device Manager/`hid_enumerate`) before the final wire-up task sends anything to it — the table above is OpenRGB's documented values, not yet re-verified against this user's specific units.

## Review Focus

- A peripheral not connected, or not opening successfully, at startup must not crash or block the app — the tray and whatever devices *are* present must still come up (same spirit as v1's "board not connected" case, now across five independent devices). Test owned by Task 18.
- A canvas placement or an effect's draggable reference point with an extreme/out-of-range coordinate (e.g. dragged far outside the window, or loaded from a hand-edited settings file) must not crash `colorAt()` or corrupt persistence. Test owned by Task 3.
- A settings file whose `activeProfile` name doesn't match any entry in `profiles` (hand-edited, or a deleted profile) must not crash; it must fall back to a defined profile rather than leaving no active profile. Test owned by Task 14.
- A channel in `CanvasSource::GlobalFollow` while no global effect has been assigned yet (fresh profile) must not crash `Canvas::tick()` — it must simply skip pushing that channel this frame. Test owned by Task 3.
- Two channels placed at the exact same canvas coordinates must not crash angle/distance-based effect math (division by zero / `NaN` at zero distance from a reference point). Test owned by Task 6 (Spiral, which divides by distance).

---

## Task 1: Extract shared device value types, add `IDriver`

**Files:**
- Create: `src/device/DeviceTypes.h`
- Modify: `src/device/MsiMotherboardDriver.h` (remove `ChannelMode`/`NativeModeSettings` definitions, `#include "device/DeviceTypes.h"` instead, add `: public IDriver` and `override` on the four interface methods)
- Create: `src/device/IDriver.h`
- Test: `tests/device/test_msi_motherboard_driver.cpp` (extend existing file)

**Interfaces:**
- Produces: `DeviceTypes.h` defines `enum class ChannelMode { Canvas, Native }` and `struct NativeModeSettings { ... }` (moved verbatim from `MsiMotherboardDriver.h`, no field changes). `IDriver` defines the four abstract methods below, for every later driver task to implement.

```cpp
// src/device/IDriver.h
class IDriver {
public:
    virtual ~IDriver() = default;
    virtual bool setChannelMode(const std::string& channelName, ChannelMode mode) = 0;
    virtual ChannelMode channelMode(const std::string& channelName) const = 0;
    virtual bool setNativeMode(const std::string& channelName, const NativeModeSettings& settings) = 0;
    virtual bool pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors) = 0;
};
```

`MsiMotherboardDriver`'s moving these types out and implementing `IDriver` must not change any of its existing behavior — this task is a pure refactor plus one new base class, not a behavior change, so **no existing `MsiMotherboardDriver` test should need its assertions changed**, only recompiled.

- [ ] **Step 1: Create `src/device/DeviceTypes.h`** containing exactly the `ChannelMode` enum and `NativeModeSettings` struct currently in `src/device/MsiMotherboardDriver.h`, moved verbatim (same field names, types, defaults).
- [ ] **Step 2: Update `src/device/MsiMotherboardDriver.h`** to `#include "device/DeviceTypes.h"` instead of defining those types itself.
- [ ] **Step 3: Create `src/device/IDriver.h`** with the interface shown above (`#include "device/DeviceTypes.h"` and `#include "protocol/MsiProtocol.h"` for `MsiColor`).
- [ ] **Step 4: Update `src/device/MsiMotherboardDriver.h`** to `#include "device/IDriver.h"`, change the class declaration to `class MsiMotherboardDriver : public IDriver`, and add `override` to its four matching methods' declarations.
- [ ] **Step 5: Build and run the full existing test suite** (`ctest` from the build dir) to confirm the refactor changed no behavior.

Run: `ctest --output-on-failure`
Expected: all existing tests (10/10 as of v1) still PASS, unchanged.

- [ ] **Step 6: Commit**

```bash
git add src/device/DeviceTypes.h src/device/IDriver.h src/device/MsiMotherboardDriver.h
git commit -m "refactor: extract ChannelMode/NativeModeSettings, add IDriver interface"
```

---

## Task 2: `IHidDevice` gains `write()` and interface-aware `open()`

**Files:**
- Modify: `src/hid/IHidDevice.h`
- Modify: `src/hid/HidApiDevice.h`, `src/hid/HidApiDevice.cpp`
- Modify: `tests/hid/FakeHidDevice.h`

**Interfaces:**
- Produces: `IHidDevice::open(vendorId, productId, interfaceNumber = -1)` (default preserves v1's existing call sites unchanged) and a new `IHidDevice::write(report, length)` for output reports (`hid_write`), alongside the existing `sendFeatureReport` (`hid_send_feature_report`). Some peripherals (the Roccat Vulcan's LED interface) send color data via output report, not feature report — both must exist side by side.

```cpp
// src/hid/IHidDevice.h
class IHidDevice {
public:
    virtual ~IHidDevice() = default;
    // interfaceNumber == -1 (default) opens the first match for
    // vendorId/productId, same as v1's behavior. A device exposing
    // several HID interfaces under one VID/PID (e.g. Roccat Vulcan's
    // separate control and LED interfaces) is opened once per interface
    // by constructing a separate IHidDevice per interfaceNumber.
    virtual bool open(uint16_t vendorId, uint16_t productId, int interfaceNumber = -1) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;
    virtual bool sendFeatureReport(const uint8_t* report, size_t length) = 0;
    virtual bool write(const uint8_t* report, size_t length) = 0;
};
```

`HidApiDevice::open` must use `hid_enumerate(vendorId, productId)` + match `interface_number` (when not -1) + `hid_open_path`, falling back to plain `hid_open(vendorId, productId, nullptr)` when `interfaceNumber == -1`, to keep v1's exact existing behavior for the motherboard.

- [ ] **Step 1: Write the failing tests** in a new `FakeHidDevice` exercised by a small new test target (mirrors how `test_msi_motherboard_driver` already uses `FakeHidDevice`): add `writtenReports` tracking and an `openInterfaceNumber` capture field to `FakeHidDevice`, then add a `tests/hid/test_fake_hid_device.cpp` asserting:
  - `open(0x1234, 0x5678)` with no third argument sets `openInterfaceNumber == -1`.
  - `open(0x1234, 0x5678, 2)` sets `openInterfaceNumber == 2`.
  - `write(...)` appends to `writtenReports`, independently of `sentReports` (which only `sendFeatureReport` appends to).
- [ ] **Step 2: Run test to verify it fails** (interface doesn't exist yet).

Run: `ctest -R test_fake_hid_device --output-on-failure`
Expected: FAIL to build (`write`/third `open` arg not declared).

- [ ] **Step 3: Implement** the `IHidDevice.h` change above, update `FakeHidDevice` to match, update `HidApiDevice::open`/`HidApiDevice::write` using `hid_enumerate`/`hid_open_path`/`hid_write` as described.
- [ ] **Step 4: Run test to verify it passes.**

Run: `ctest -R test_fake_hid_device --output-on-failure`
Expected: PASS

- [ ] **Step 5: Run the full suite to confirm the default-argument change didn't break any existing `IHidDevice`/`MsiMotherboardDriver` test.**

Run: `ctest --output-on-failure`
Expected: all PASS

- [ ] **Step 6: Commit**

```bash
git add src/hid/IHidDevice.h src/hid/HidApiDevice.h src/hid/HidApiDevice.cpp tests/hid/FakeHidDevice.h tests/hid/test_fake_hid_device.cpp tests/CMakeLists.txt
git commit -m "feat: add write() and interface-aware open() to IHidDevice"
```

---

## Task 3: Canvas-global placement and "follow global effect"

**Files:**
- Modify: `src/canvas/Canvas.h`, `src/canvas/Canvas.cpp`
- Modify: `tests/canvas/test_canvas.cpp`

**Interfaces:**
- Consumes: `IDriver` (Task 1), existing `Channel`/`IEffect`.
- Produces: `CanvasSource`, `CanvasPlacement`, the extended `ChannelAssignment`, and `Canvas::setGlobalEffect`/new multi-driver constructor — every later device/UI task that touches per-region assignment uses these exact names.

```cpp
// src/canvas/Canvas.h
enum class CanvasSource { Independent, GlobalFollow };
struct CanvasPlacement { double x = 0.0, y = 0.0; };

struct ChannelAssignment {
    ChannelMode mode = ChannelMode::Canvas;
    CanvasSource source = CanvasSource::Independent; // meaningful only when mode == Canvas
    std::shared_ptr<IEffect> effect;        // used when source == Independent
    NativeModeSettings nativeSettings;      // used when mode == Native
    CanvasPlacement placement{};             // this channel's offset in global canvas space
};

class Canvas {
public:
    // channelName -> the IDriver responsible for it. Several channel names
    // may map to the same IDriver* (e.g. all 6 motherboard channels share
    // one MsiMotherboardDriver instance); each peripheral channel maps to
    // its own driver instance.
    Canvas(std::map<std::string, IDriver*> driversByChannel, std::map<std::string, Channel>& channels);

    void setAssignment(const std::string& channelName, ChannelAssignment assignment); // unchanged signature
    void setGlobalEffect(std::shared_ptr<IEffect> effect); // new
    void tick(int64_t elapsedMs); // unchanged signature, extended behavior below

private:
    std::map<std::string, IDriver*> driversByChannel_;
    std::map<std::string, Channel>& channels_;
    std::map<std::string, ChannelAssignment> assignments_;
    std::map<std::string, bool> nativeModePending_;
    std::shared_ptr<IEffect> globalEffect_;
};
```

`tick()`'s per-channel Canvas-mode branch changes from "sample `assignment.effect` at the LED's local `(x,y)`" to: if `source == Independent`, unchanged (sample `assignment.effect`, skip the channel this frame if `assignment.effect` is null — matches existing v1 tolerance); if `source == GlobalFollow`, sample `globalEffect_` at `(placement.x + localX, placement.y + localY)` for each LED, or skip the whole channel this frame if `globalEffect_` is null (Review Focus item). Native-mode handling and the "continue to next channel on any driver-call failure" tolerance are unchanged from v1.

- [ ] **Step 1: Write the failing tests** in `tests/canvas/test_canvas.cpp`, reusing the existing `FakeEffect`/`FakeHidDevice`-based fixtures already in that file:
  - `testTick_globalFollowSamplesGlobalEffectAtPlacementOffset`: one channel, `source = GlobalFollow`, `placement = {10, 0}`, a `FakeEffect` set via `setGlobalEffect` that returns a distinct color per `(x,y)`; assert the pushed colors match what the global effect returns at `(10 + localX, localY)`, not at local `(localX, localY)`.
  - `testTick_globalFollowWithNoGlobalEffectDoesNotCrash`: channel in `GlobalFollow`, `setGlobalEffect` never called; `tick()` must not throw/crash and must not call `pushCanvasColors` for that channel.
  - `testTick_extremePlacementDoesNotCrash`: `placement = {1e9, -1e9}`; `tick()` must not throw/crash (Review Focus item).
  - `testTick_multipleDriversEachReceiveOnlyTheirOwnChannel`: two channels mapped to two different fake `IDriver`s; assert each fake driver only recorded pushes for its own channel name.
- [ ] **Step 2: Run tests to verify they fail** (new types/signature don't exist yet).

Run: `ctest -R test_canvas --output-on-failure`
Expected: FAIL to build.

- [ ] **Step 3: Implement** the header changes and `tick()` logic described above.
- [ ] **Step 4: Run tests to verify they pass.**

Run: `ctest -R test_canvas --output-on-failure`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/canvas/Canvas.h src/canvas/Canvas.cpp tests/canvas/test_canvas.cpp
git commit -m "feat: add canvas-global placement and follow-global-effect mode"
```

---

## Task 4: `IPositionableEffect` + `RadialRainbowEffect`

**Files:**
- Create: `src/effects/IPositionableEffect.h`
- Create: `src/effects/RadialRainbowEffect.h`, `src/effects/RadialRainbowEffect.cpp`
- Modify: `tests/effects/test_effects.cpp`, `tests/CMakeLists.txt`

**Interfaces:**
- Produces: `IPositionableEffect` (any later effect task with a draggable reference point implements this in addition to `IEffect`); `RadialRainbowEffect` constructor and `colorAt` formula, for the UI task (Task 16) to construct and reposition.

```cpp
// src/effects/IPositionableEffect.h
// An effect with one spatial reference point the canvas lets the user drag
// (e.g. a radial gradient's center). Effects without such a parameter
// (Breathing, Marquee) implement only IEffect, not this.
class IPositionableEffect {
public:
    virtual ~IPositionableEffect() = default;
    virtual void setReferencePoint(int x, int y) = 0;
    virtual std::pair<int,int> referencePoint() const = 0;
};

// src/effects/RadialRainbowEffect.h
// hue = (angleDegrees(x,y around center) + elapsedMs*360/periodMs_) mod 360,
// full-saturation/value HSV->RGB (same conversion as RainbowWaveEffect).
// angleDegrees uses atan2(y-cy, x-cx) converted to [0,360).
class RadialRainbowEffect : public IEffect, public IPositionableEffect {
public:
    explicit RadialRainbowEffect(int centerX = 0, int centerY = 0, int64_t periodMs = 3000);
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;
    void setReferencePoint(int x, int y) override;
    std::pair<int,int> referencePoint() const override;
private:
    int centerX_, centerY_;
    int64_t periodMs_; // clamped >=1 in the constructor, same reason as RainbowWaveEffect
};
```

- [ ] **Step 1: Write the failing tests** in `tests/effects/test_effects.cpp`:
  - `testRadialRainbowEffect_sameAngleSameHueRegardlessOfDistance`: `colorAt(cx+10, cy, 0)` and `colorAt(cx+100, cy, 0)` (both due east of center, angle 0) return the same RGB.
  - `testRadialRainbowEffect_angle90DiffersFromAngle0`: `colorAt(cx, cy-10, 0)` (north, angle 90) differs from `colorAt(cx+10, cy, 0)` (east, angle 0).
  - `testRadialRainbowEffect_zeroDistanceDoesNotCrash` (Review Focus item: `atan2(0,0)` is well-defined in C++, but pin it anyway): `colorAt(cx, cy, 0)` returns without throwing.
  - `testRadialRainbowEffect_setReferencePointMovesTheCenter`: construct at `(0,0)`, call `setReferencePoint(50,50)`, then `colorAt(60,50,0)` must equal what the original constructed-at-`(50,50)` instance would return at the same point (same angle/time formula, new center).
- [ ] **Step 2: Run tests to verify they fail.**

Run: `ctest -R test_effects --output-on-failure`
Expected: FAIL to build.

- [ ] **Step 3: Implement** `IPositionableEffect.h` and `RadialRainbowEffect` per the formula above, reusing the HSV→RGB conversion already written in `RainbowWaveEffect.cpp` (extract it to a shared free function `src/effects/HsvToRgb.h` if duplicating it a sixth time — by Task 9 — would otherwise violate DRY; a single header-only `inline MsiColor HsvFullToRgb(int hueDegrees)` is enough).
- [ ] **Step 4: Run tests to verify they pass.**

Run: `ctest -R test_effects --output-on-failure`
Expected: PASS

- [ ] **Step 5: Commit**

```bash
git add src/effects/IPositionableEffect.h src/effects/RadialRainbowEffect.h src/effects/RadialRainbowEffect.cpp src/effects/HsvToRgb.h tests/effects/test_effects.cpp tests/CMakeLists.txt
git commit -m "feat: add RadialRainbowEffect and IPositionableEffect"
```

---

## Task 5: `BreathingEffect`

**Files:**
- Create: `src/effects/BreathingEffect.h`, `src/effects/BreathingEffect.cpp`
- Modify: `tests/effects/test_effects.cpp`

**Interfaces:**
- Produces: `BreathingEffect`, used standalone (no `IPositionableEffect` — no spatial reference point).

```cpp
// src/effects/BreathingEffect.h
// brightness(t) = (sin(2*pi*elapsedMs/periodMs_) + 1) / 2, applied
// uniformly to `color` (ignores x,y -- same color everywhere, pulsing).
class BreathingEffect : public IEffect {
public:
    explicit BreathingEffect(MsiColor color, int64_t periodMs = 3000);
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;
private:
    MsiColor color_;
    int64_t periodMs_; // clamped >=1, same reason as RainbowWaveEffect
};
```

- [ ] **Step 1: Write the failing tests:**
  - `testBreathingEffect_quarterPeriodIsFullBrightness`: `periodMs=4000`, `colorAt(0,0,1000)` (quarter period, sine peak) returns `color` unchanged (brightness ≈ 1.0).
  - `testBreathingEffect_threeQuarterPeriodIsOff`: `colorAt(0,0,3000)` (three-quarter period, sine trough) returns black (brightness ≈ 0.0).
  - `testBreathingEffect_zeroOrNegativePeriodDoesNotCrash`.
- [ ] **Step 2: Run to verify failure.**
- [ ] **Step 3: Implement** per the formula above.
- [ ] **Step 4: Run to verify pass.**
- [ ] **Step 5: Commit**

```bash
git add src/effects/BreathingEffect.h src/effects/BreathingEffect.cpp tests/effects/test_effects.cpp
git commit -m "feat: add BreathingEffect"
```

---

## Task 6: `SpiralEffect`

**Files:**
- Create: `src/effects/SpiralEffect.h`, `src/effects/SpiralEffect.cpp`
- Modify: `tests/effects/test_effects.cpp`

**Interfaces:**
- Produces: `SpiralEffect`, implements `IPositionableEffect` like `RadialRainbowEffect`.

```cpp
// src/effects/SpiralEffect.h
// hue = (angleDegrees(x,y around center) + distance(x,y,center)*tightnessDegPerPx
//        + elapsedMs*360/periodMs_) mod 360. distance = 0 is well-defined
// (contributes 0 to hue) -- Review Focus item: two channels placed at the
// same point both still get a valid, non-NaN color, just from angle+time
// alone.
class SpiralEffect : public IEffect, public IPositionableEffect {
public:
    explicit SpiralEffect(int centerX = 0, int centerY = 0, double tightnessDegPerPx = 4.0, int64_t periodMs = 3000);
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;
    void setReferencePoint(int x, int y) override;
    std::pair<int,int> referencePoint() const override;
private:
    int centerX_, centerY_;
    double tightnessDegPerPx_;
    int64_t periodMs_;
};
```

- [ ] **Step 1: Write the failing tests:**
  - `testSpiralEffect_zeroDistanceDoesNotCrashAndIsFinite` (Review Focus item): `colorAt(cx, cy, 0)` returns a value where `r,g,b` are each a normal `uint8_t` (not reachable as NaN given the type, but assert the call completes and returns deterministically across repeated calls).
  - `testSpiralEffect_sameAngleDifferentDistanceDiffersInHue`: `colorAt(cx+10, cy, 0)` vs `colorAt(cx+100, cy, 0)` (same angle, different distance) must differ, unlike `RadialRainbowEffect`.
  - `testSpiralEffect_setReferencePointMovesTheCenter` (mirrors Task 4's test).
- [ ] **Step 2: Run to verify failure.**
- [ ] **Step 3: Implement**, reusing `HsvToRgb.h` from Task 4.
- [ ] **Step 4: Run to verify pass.**
- [ ] **Step 5: Commit**

```bash
git add src/effects/SpiralEffect.h src/effects/SpiralEffect.cpp tests/effects/test_effects.cpp
git commit -m "feat: add SpiralEffect"
```

---

## Task 7: `CometEffect` and `MarqueeEffect`

**Files:**
- Create: `src/effects/CometEffect.h`, `src/effects/CometEffect.cpp`
- Create: `src/effects/MarqueeEffect.h`, `src/effects/MarqueeEffect.cpp`
- Modify: `tests/effects/test_effects.cpp`

**Interfaces:**
- Produces: both effects, neither implements `IPositionableEffect` (animated motion, not a user-dragged point).

```cpp
// src/effects/CometEffect.h
// headX(t) = (elapsedMs * speedPxPerMs) mod trackWidthPx.
// dist = ((headX - x) mod trackWidthPx), wrapped into [0, trackWidthPx).
// brightness = max(0, 1 - dist/tailLengthPx). output = color * brightness.
// y is ignored (horizontal track, same convention as RainbowWaveEffect).
class CometEffect : public IEffect {
public:
    CometEffect(MsiColor color, int trackWidthPx, double speedPxPerMs = 0.05, int tailLengthPx = 20);
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;
private:
    MsiColor color_;
    int trackWidthPx_; // clamped >=1
    double speedPxPerMs_;
    int tailLengthPx_; // clamped >=1
};

// src/effects/MarqueeEffect.h
// period = dashLengthPx + gapLengthPx (clamped >=1). pos = ((x - elapsedMs*speedPxPerMs)
// mod period), wrapped into [0, period). on = pos < dashLengthPx.
// output = on ? color : MsiColor{0,0,0}. y is ignored.
class MarqueeEffect : public IEffect {
public:
    MarqueeEffect(MsiColor color, int dashLengthPx = 5, int gapLengthPx = 5, double speedPxPerMs = 0.05);
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;
private:
    MsiColor color_;
    int dashLengthPx_, gapLengthPx_;
    double speedPxPerMs_;
};
```

- [ ] **Step 1: Write the failing tests:**
  - `testCometEffect_headPositionIsBrightest`: at `elapsedMs` where `headX == 0` exactly (e.g. `elapsedMs=0`), `colorAt(0,0,0)` equals `color` unchanged (brightness 1.0).
  - `testCometEffect_farFromHeadIsDark`: `colorAt(trackWidthPx/2, 0, 0)` (half the track away from the head, beyond `tailLengthPx`) returns black.
  - `testCometEffect_tinyTrackOrTailDoesNotCrash`: `trackWidthPx=0`, `tailLengthPx=0` must not divide by zero (clamp verified).
  - `testMarqueeEffect_onInsideDashOffInsideGap`: `dashLengthPx=5, gapLengthPx=5`, `colorAt(2,0,0)` (inside the dash) equals `color`; `colorAt(7,0,0)` (inside the gap) equals black.
  - `testMarqueeEffect_scrollsWithTime`: same `x`, two different `elapsedMs` values a half-period apart toggle on/off.
- [ ] **Step 2: Run to verify failure.**
- [ ] **Step 3: Implement** both per the formulas above.
- [ ] **Step 4: Run to verify pass.**
- [ ] **Step 5: Commit**

```bash
git add src/effects/CometEffect.h src/effects/CometEffect.cpp src/effects/MarqueeEffect.h src/effects/MarqueeEffect.cpp tests/effects/test_effects.cpp
git commit -m "feat: add CometEffect and MarqueeEffect"
```

---

## Task 8: `SwirlCirclesEffect`

**Files:**
- Create: `src/effects/SwirlCirclesEffect.h`, `src/effects/SwirlCirclesEffect.cpp`
- Modify: `tests/effects/test_effects.cpp`

**Interfaces:**
- Produces: `SwirlCirclesEffect`, implements `IPositionableEffect`.

```cpp
// src/effects/SwirlCirclesEffect.h
// ringIndex = floor(distance(x,y,center) / ringWidthPx). hue = (ringIndex*60
// + elapsedMs*360/periodMs_) mod 360, full-saturation/value HSV->RGB.
class SwirlCirclesEffect : public IEffect, public IPositionableEffect {
public:
    explicit SwirlCirclesEffect(int centerX = 0, int centerY = 0, int ringWidthPx = 15, int64_t periodMs = 3000);
    MsiColor colorAt(int x, int y, int64_t elapsedMs) const override;
    void setReferencePoint(int x, int y) override;
    std::pair<int,int> referencePoint() const override;
private:
    int centerX_, centerY_;
    int ringWidthPx_; // clamped >=1
    int64_t periodMs_;
};
```

- [ ] **Step 1: Write the failing tests:**
  - `testSwirlCirclesEffect_sameRingSameHue`: two points at different angles but the same distance band from center return the same hue/RGB at the same `elapsedMs`.
  - `testSwirlCirclesEffect_adjacentRingsDiffer`: a point just inside ring 0 and a point just inside ring 1 differ.
  - `testSwirlCirclesEffect_zeroRingWidthDoesNotCrash`.
- [ ] **Step 2: Run to verify failure.**
- [ ] **Step 3: Implement**, reusing `HsvToRgb.h`.
- [ ] **Step 4: Run to verify pass.**
- [ ] **Step 5: Commit**

```bash
git add src/effects/SwirlCirclesEffect.h src/effects/SwirlCirclesEffect.cpp tests/effects/test_effects.cpp
git commit -m "feat: add SwirlCirclesEffect"
```

---

## Task 9: SteelSeries Apex 3 driver (T-Zone protocol)

This is the simplest of the four peripherals' protocols — fully pinned below from OpenRGB's `SteelSeriesApexTZoneController::SetColor` (`Controllers/SteelSeriesController/SteelSeriesApexTZoneController/SteelSeriesApexTZoneController.cpp:56-80`), which this task ports.

**Files:**
- Create: `src/protocol/SteelSeriesApex3Protocol.h`, `src/protocol/SteelSeriesApex3Protocol.cpp`
- Create: `src/device/SteelSeriesApex3Driver.h`, `src/device/SteelSeriesApex3Driver.cpp`
- Test: `tests/protocol/test_steelseries_apex3_protocol.cpp`, `tests/device/test_steelseries_apex3_driver.cpp`

**Interfaces:**
- Consumes: `IHidDevice` (Task 2), `IDriver` (Task 1).
- Produces: `SteelSeriesApex3Driver`, used by Task 18's final wire-up and by Task 15 (UI) to know this device's channel name (`"SteelSeriesApex3"`) and zone count.

```cpp
// src/protocol/SteelSeriesApex3Protocol.h
// Ported from SteelSeriesApexTZoneController::SetColor. No report ID byte
// (buf[0] stays 0x00); two 33-byte output reports (hid_write, not a feature
// report) per update: a brightness packet (cmd 0x0A) then a colors packet
// (cmd 0x0B, up to 10 zones' RGB starting at offset 3).
constexpr size_t STEELSERIES_APEX3_PACKET_SIZE = 33;
constexpr int STEELSERIES_APEX3_MAX_ZONES = 10; // (33-3)/3

std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE> BuildApex3BrightnessPacket(uint8_t brightness);
std::array<uint8_t, STEELSERIES_APEX3_PACKET_SIZE> BuildApex3ColorsPacket(const std::vector<MsiColor>& zoneColors); // throws std::invalid_argument if zoneColors.size() > STEELSERIES_APEX3_MAX_ZONES

// src/device/SteelSeriesApex3Driver.h
class SteelSeriesApex3Driver : public IDriver {
public:
    explicit SteelSeriesApex3Driver(IHidDevice& hid);
    static Channel createDefaultChannel(); // one Channel named "SteelSeriesApex3", one Component with STEELSERIES_APEX3_MAX_ZONES zone-coordinates (see Step 3 note on placement)
    bool setChannelMode(const std::string& channelName, ChannelMode mode) override; // true only for Canvas on "SteelSeriesApex3"; Native always returns false (Global Constraints: direct-mode only)
    ChannelMode channelMode(const std::string& channelName) const override;
    bool setNativeMode(const std::string& channelName, const NativeModeSettings& settings) override; // always returns false, no-op
    bool pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>& colors) override; // sends both packets via hid.write(); returns false without sending if !hid.isOpen(), wrong channelName, or colors.size() > STEELSERIES_APEX3_MAX_ZONES
private:
    IHidDevice& hid_;
};
```

- [ ] **Step 1: Write the failing protocol tests** in `tests/protocol/test_steelseries_apex3_protocol.cpp`:
  - `testBuildApex3BrightnessPacket_layout`: `BuildApex3BrightnessPacket(3)` returns a 33-byte array with `[0]==0x00, [1]==0x0A, [3]==3`, all other bytes `0x00`.
  - `testBuildApex3ColorsPacket_layout`: `BuildApex3ColorsPacket({{1,2,3},{4,5,6}})` returns `[0]==0x00, [1]==0x0B, [3..5]=={1,2,3}, [6..8]=={4,5,6}`, remaining bytes `0x00`.
  - `testBuildApex3ColorsPacket_tooManyZonesThrows`: 11 zones throws `std::invalid_argument`.
- [ ] **Step 2: Run to verify failure.**
- [ ] **Step 3: Implement** `SteelSeriesApex3Protocol.cpp` per the cited OpenRGB function.
- [ ] **Step 4: Run to verify pass.**
- [ ] **Step 5: Write the failing driver tests** in `tests/device/test_steelseries_apex3_driver.cpp` (mirrors `test_msi_motherboard_driver.cpp`'s style with `FakeHidDevice`):
  - `testPushCanvasColors_sendsBothPacketsInOrder`: assert `fakeHid.writtenReports` has exactly 2 entries, matching `BuildApex3BrightnessPacket`/`BuildApex3ColorsPacket` output for the given input.
  - `testSetNativeMode_alwaysReturnsFalse`.
  - `testSetChannelMode_nativeReturnsFalseCanvasReturnsTrue`.
  - `testPushCanvasColors_returnsFalseWhenHidNotOpen`.
- [ ] **Step 6: Run to verify failure, then implement `SteelSeriesApex3Driver`, then run to verify pass.**
- [ ] **Step 7: Commit**

```bash
git add src/protocol/SteelSeriesApex3Protocol.h src/protocol/SteelSeriesApex3Protocol.cpp src/device/SteelSeriesApex3Driver.h src/device/SteelSeriesApex3Driver.cpp tests/protocol/test_steelseries_apex3_protocol.cpp tests/device/test_steelseries_apex3_driver.cpp tests/CMakeLists.txt
git commit -m "feat: add SteelSeries Apex 3 driver"
```

Note on zone placement (Step 3 of `createDefaultChannel`): the T-Zone layout's exact physical zone positions aren't needed for protocol correctness, only for how the canvas preview looks. `ponytail: approximate the Apex 3's zone coordinates as a simple left-to-right row for v1; replace with the real keyboard-zone layout if/when the canvas preview needs to look more accurate.`

---

## Task 10: Roccat Kone Aimo driver

**Files:**
- Create: `src/protocol/RoccatKoneProtocol.h`, `src/protocol/RoccatKoneProtocol.cpp`
- Create: `src/device/RoccatKoneDriver.h`, `src/device/RoccatKoneDriver.cpp`
- Test: `tests/protocol/test_roccat_kone_protocol.cpp`, `tests/device/test_roccat_kone_driver.cpp`

**Interfaces:**
- Same shape as Task 9's `IDriver` implementation, channel name `"RoccatKone"`.

- [ ] **Step 1: Read `Controllers/RoccatController/RoccatKoneAimoController/RoccatKoneAimoController.cpp`** (`SetChannelColors`/`SendUpdate`, around lines 60-112) and its header for the exact 46-byte feature-report buffer layout: report-ID/init bytes set in `SendInit`, the `0x02 + (colorIndex*4)` per-LED offset and the R/G/B sub-offsets (`R_OFFSET`/`G_OFFSET`/`B_OFFSET` constants in that header). Derive the exact expected byte layout for a small representative input (e.g. 2 LEDs, distinct colors) directly from that code — this is the test's source of truth, not a value to guess.
- [ ] **Step 2: Write the failing protocol test** `tests/protocol/test_roccat_kone_protocol.cpp`: `testBuildKoneColorsReport_layout`, asserting `BuildKoneColorsReport({...})` (signature: `std::array<uint8_t,46> BuildKoneColorsReport(const std::vector<MsiColor>& colors)`) matches the bytes derived in Step 1 exactly, including whatever fixed/init bytes `SendInit` establishes that persist across the buffer's life (if `SendInit`'s state can't be captured as a pure function of the color list alone, add a `BuildKoneInitReport()` counterpart and assert both are sent in `pushCanvasColors`, in the same order OpenRGB sends them).
- [ ] **Step 3: Run to verify failure.**
- [ ] **Step 4: Implement** `RoccatKoneProtocol.cpp`.
- [ ] **Step 5: Run to verify pass.**
- [ ] **Step 6: Write the failing driver tests** `tests/device/test_roccat_kone_driver.cpp` (same four test shapes as Task 9's Step 5, adapted to `pushCanvasColors` sending feature report(s) via `fakeHid.sentReports` instead of `write`).
- [ ] **Step 7: Run to verify failure, implement `RoccatKoneDriver`, run to verify pass.**
- [ ] **Step 8: Commit**

```bash
git add src/protocol/RoccatKoneProtocol.h src/protocol/RoccatKoneProtocol.cpp src/device/RoccatKoneDriver.h src/device/RoccatKoneDriver.cpp tests/protocol/test_roccat_kone_protocol.cpp tests/device/test_roccat_kone_driver.cpp tests/CMakeLists.txt
git commit -m "feat: add Roccat Kone Aimo driver"
```

---

## Task 11: Razer Basilisk V3 driver

**Files:**
- Create: `src/protocol/RazerBasiliskProtocol.h`, `src/protocol/RazerBasiliskProtocol.cpp`
- Create: `src/device/RazerBasiliskDriver.h`, `src/device/RazerBasiliskDriver.cpp`
- Test: `tests/protocol/test_razer_basilisk_protocol.cpp`, `tests/device/test_razer_basilisk_driver.cpp`

**Interfaces:**
- Same shape as Task 9, channel name `"RazerBasiliskV3"`.

- [ ] **Step 1: Read `Controllers/RazerController/RazerController/RazerController.cpp`**, specifically the `razer_argb_report` struct definition and `razer_usb_send_argb` (around line 2227) plus whatever caller builds the report for the Basilisk V3's ARGB zones (search the file for `RAZER_BASILISK_V3_PID` to find the model-specific zone count/call site). Derive the exact packed-struct byte layout from there.
- [ ] **Step 2: Write the failing protocol test** `tests/protocol/test_razer_basilisk_protocol.cpp`: `testBuildBasiliskArgbReport_layout`, signature `std::vector<uint8_t> BuildBasiliskArgbReport(const std::vector<MsiColor>& colors)` (or `std::array<uint8_t,N>` if the struct size read in Step 1 is fixed), asserting the exact byte layout derived in Step 1 for a representative input.
- [ ] **Step 3: Run to verify failure.**
- [ ] **Step 4: Implement** `RazerBasiliskProtocol.cpp`.
- [ ] **Step 5: Run to verify pass.**
- [ ] **Step 6: Write the failing driver tests** `tests/device/test_razer_basilisk_driver.cpp` (same four shapes as Task 9's Step 5).
- [ ] **Step 7: Run to verify failure, implement `RazerBasiliskDriver`, run to verify pass.**
- [ ] **Step 8: Commit**

```bash
git add src/protocol/RazerBasiliskProtocol.h src/protocol/RazerBasiliskProtocol.cpp src/device/RazerBasiliskDriver.h src/device/RazerBasiliskDriver.cpp tests/protocol/test_razer_basilisk_protocol.cpp tests/device/test_razer_basilisk_driver.cpp tests/CMakeLists.txt
git commit -m "feat: add Razer Basilisk V3 driver"
```

---

## Task 12: Roccat Vulcan 100/120 Aimo driver (two HID interfaces)

The most complex of the four: a per-key-addressable keyboard using **two separate HID interfaces** on the same VID/PID (a control interface for mode/brightness, a separate LED interface for color data via plain `hid_write`, not a feature report), and model-dependent packing constants.

**Files:**
- Create: `src/protocol/RoccatVulcanProtocol.h`, `src/protocol/RoccatVulcanProtocol.cpp`
- Create: `src/device/RoccatVulcanDriver.h`, `src/device/RoccatVulcanDriver.cpp`
- Test: `tests/protocol/test_roccat_vulcan_protocol.cpp`, `tests/device/test_roccat_vulcan_driver.cpp`

**Interfaces:**
- Consumes: `IHidDevice` (Task 2), `IDriver` (Task 1).
- Produces: `RoccatVulcanDriver`, channel name `"RoccatVulcan"`. Unlike the other three peripheral drivers, its constructor takes **two** `IHidDevice&` references.

```cpp
// src/device/RoccatVulcanDriver.h
class RoccatVulcanDriver : public IDriver {
public:
    RoccatVulcanDriver(IHidDevice& controlHid, IHidDevice& ledHid);
    static Channel createDefaultChannel(); // one Channel named "RoccatVulcan", one Component with a per-key coordinate grid (see Step 5 note)
    // ... same four IDriver methods as Task 9/10/11, Native always returns false
private:
    IHidDevice& controlHid_;
    IHidDevice& ledHid_;
};
```

- [ ] **Step 1: Read `Controllers/RoccatController/RoccatVulcanKeyboardController/RoccatVulcanKeyboardController.cpp` and its header** in full, specifically the constructor (to find which `column_length`/`header_length_first`/`data_length_packet`/`protocol_version`/`packet_num` values apply to `ROCCAT_VULCAN_100_AIMO_PID` and `ROCCAT_VULCAN_120_AIMO_PID` specifically — these two models may use different constants, check both), and the `SendColors`-equivalent function containing the `hid_write(dev_led, &bufs[p][0], 65)` loop (around line 388). This function is the exact byte-packing algorithm to port; there is no shortcut to reading it.
- [ ] **Step 2: Write the failing protocol test** `tests/protocol/test_roccat_vulcan_protocol.cpp`: `testBuildVulcanColorPackets_layout`, signature `std::vector<std::array<uint8_t,65>> BuildVulcanColorPackets(const std::vector<MsiColor>& keyColors, int protocolVersion)` (adjust the signature to whatever Step 1 shows the real packing actually depends on — if it only ever varies by `protocolVersion` for these two specific models, a bare `bool protocolVersionTwo` flag is enough; don't carry parameters neither target model needs). Assert the exact packet count and per-key byte offsets for a small representative input (e.g. 3 keys), traced by hand from the cited function for this model's constants.
- [ ] **Step 3: Run to verify failure.**
- [ ] **Step 4: Implement** `RoccatVulcanProtocol.cpp`.
- [ ] **Step 5: Run to verify pass.**
- [ ] **Step 6: Write the failing driver tests** `tests/device/test_roccat_vulcan_driver.cpp`:
  - `testPushCanvasColors_sendsPacketsToLedHidNotControlHid`: assert `ledFakeHid.writtenReports` is populated and `controlFakeHid.writtenReports`/`sentReports` are not touched by a color push.
  - `testSetNativeMode_alwaysReturnsFalse`, `testSetChannelMode_nativeReturnsFalseCanvasReturnsTrue`, `testPushCanvasColors_returnsFalseWhenEitherHidNotOpen` (both must be open).
- [ ] **Step 7: Run to verify failure, implement `RoccatVulcanDriver`, run to verify pass.**
- [ ] **Step 8: Commit**

```bash
git add src/protocol/RoccatVulcanProtocol.h src/protocol/RoccatVulcanProtocol.cpp src/device/RoccatVulcanDriver.h src/device/RoccatVulcanDriver.cpp tests/protocol/test_roccat_vulcan_protocol.cpp tests/device/test_roccat_vulcan_driver.cpp tests/CMakeLists.txt
git commit -m "feat: add Roccat Vulcan 100/120 Aimo driver"
```

Note on key placement (Step 5 of `createDefaultChannel`): `ponytail: approximate the Vulcan's per-key coordinates as a simple row/column grid matching its physical key-matrix dimensions for v1, not exact keycap pixel positions; refine later only if the canvas preview's visual accuracy demands it — protocol correctness doesn't depend on this.`

---

## Task 13: Extend `Settings` — placement, source, global effect, profiles

**Files:**
- Modify: `src/settings/Settings.h`, `src/settings/Settings.cpp`
- Modify: `tests/settings/test_settings.cpp`

**Interfaces:**
- Consumes: `CanvasSource`/`CanvasPlacement` (Task 3).
- Produces: the extended `ChannelSettings`, `GlobalEffectSettings`, `Profile`, and `Settings`'s new profile-list shape — Task 16 (UI) and Task 18 (wire-up) read/write exactly these.

```cpp
// src/settings/Settings.h
struct ChannelSettings {
    std::string channelName;
    ChannelMode mode = ChannelMode::Native;
    CanvasSource source = CanvasSource::Independent; // new; meaningful only when mode == Canvas
    CanvasPlacement placement{};                      // new
    std::string effectName;
    MsiColor staticColor;
    NativeModeSettings nativeSettings;
    bool operator==(const ChannelSettings&) const; // extended to compare the two new fields too
};

// New. periodMs/speed per effect are intentionally not yet configurable --
// ponytail: hardcode each effect's own default period for v1; add fields
// here when the UI actually exposes a speed control.
struct GlobalEffectSettings {
    std::string effectName;
    MsiColor color;
    int referenceX = 0, referenceY = 0; // meaningful only for effects implementing IPositionableEffect
};

// New.
struct Profile {
    std::string name;
    std::vector<ChannelSettings> channelSettings;
    GlobalEffectSettings globalEffect;
};

class Settings {
public:
    std::vector<Profile> profiles;
    std::string activeProfileName;

    bool saveToFile(const std::string& path) const;
    bool loadFromFile(const std::string& path); // see migration + fallback behavior below

    // Convenience accessor used by main.cpp/UI: returns a pointer to the
    // profile named activeProfileName, or to profiles.front() if that name
    // isn't found (Review Focus item), or nullptr if profiles is empty.
    Profile* activeProfile();
};
```

`loadFromFile` must handle two legacy/malformed cases in addition to v1's existing missing-file/corrupt-JSON/wrong-type tolerance (all of which stays unchanged, just operating one level deeper inside each profile's channel list now):
- **Migration**: a v1-style file (top-level `{"channels": [...]}`, no `"profiles"` key) loads as a single `Profile{name: "Default", channelSettings: <parsed channels>, globalEffect: {}}`, `activeProfileName = "Default"` — so existing users' `settings.json` keeps working unchanged.
- **Dangling active profile** (Review Focus item): `"activeProfile"` naming a profile not present in `"profiles"` must not crash; `activeProfile()` falls back to `profiles.front()`.

- [ ] **Step 1: Write the failing tests** in `tests/settings/test_settings.cpp`:
  - `testLoadFromFile_legacyChannelsOnlyFileMigratesToDefaultProfile`: write a v1-shaped file (reuse the existing round-trip test's fixture shape), load it, assert `profiles.size() == 1`, `profiles[0].name == "Default"`, `activeProfileName == "Default"`, and the channel settings match.
  - `testLoadFromFile_danglingActiveProfileFallsBackToFirst`: a file with two profiles and `"activeProfile": "DoesNotExist"`; assert `activeProfile()` returns a pointer to `&profiles[0]` (or an equal profile), not `nullptr`, and does not crash.
  - `testActiveProfile_emptyProfilesReturnsNullptr`.
  - `testSaveAndLoadRoundTrip_multipleProfilesWithPlacementAndSource`: extend the existing round-trip test to include 2 profiles, at least one `ChannelSettings` with `source = GlobalFollow` and a non-zero `placement`, and a non-default `GlobalEffectSettings`; assert everything round-trips exactly.
- [ ] **Step 2: Run to verify failure.**
- [ ] **Step 3: Implement** the extended types and migration/fallback logic.
- [ ] **Step 4: Run to verify pass**, including every pre-existing `test_settings` case (all must still pass against the new shape via the migration path).
- [ ] **Step 5: Commit**

```bash
git add src/settings/Settings.h src/settings/Settings.cpp tests/settings/test_settings.cpp
git commit -m "feat: add profiles, canvas placement/source to Settings"
```

---

## Task 14: `LightingCanvasWidget`

**Files:**
- Create: `src/ui/LightingCanvasWidget.h`, `src/ui/LightingCanvasWidget.cpp`
- Test: `tests/ui/test_lighting_canvas_widget.cpp`

**Interfaces:**
- Consumes: `ChannelSettings`/`CanvasPlacement` (Task 13), `IEffect`/`IPositionableEffect` (Tasks 4-8), reads the physical-mirror case-outline layout confirmed in the design doc's mockup (a fixed background outline drawn once; exact artwork is this task's own call, a simple tower-rectangle + keyboard/mouse-row outline per the mockup is enough — no new asset pipeline).
- Produces: the widget's public API, consumed by Task 15 (main window).

```cpp
// src/ui/LightingCanvasWidget.h
// QWidget showing every channel as a small draggable rectangle over a
// live-rendered background: the globally-assigned effect (if any) sampled
// across the widget's full area via QPainter, redrawn on a 60Hz QTimer
// (mirrors Canvas::tick's own rate). Devices in "Independent" mode are
// drawn with their own effect's color at their own position instead of the
// background's.
class LightingCanvasWidget : public QWidget {
    Q_OBJECT
public:
    explicit LightingCanvasWidget(QWidget* parent = nullptr);

    // Registers (or replaces) one channel's canvas box; `name` is shown as
    // the box's label. Placement/source come from the ChannelSettings the
    // caller already owns -- this widget does not read Settings directly.
    void setChannel(const std::string& channelName, CanvasPlacement placement, CanvasSource source, std::shared_ptr<IEffect> ownEffect);
    void removeChannel(const std::string& channelName);
    void setGlobalEffect(std::shared_ptr<IEffect> effect);

signals:
    // Emitted live while dragging a channel's box, and once more on release.
    void channelPlacementChanged(const std::string& channelName, CanvasPlacement newPlacement);
    // Emitted while dragging a positionable global effect's reference-point handle.
    void globalEffectReferencePointChanged(int x, int y);

protected:
    void paintEvent(QPaintEvent*) override;
    void mousePressEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;

private:
    struct ChannelBox { CanvasPlacement placement; CanvasSource source; std::shared_ptr<IEffect> ownEffect; };
    std::map<std::string, ChannelBox> channels_;
    std::shared_ptr<IEffect> globalEffect_;
    QTimer repaintTimer_;
    QElapsedTimer clock_;
    // drag state: which box (by name) or "the global effect's reference
    // point" is currently grabbed, and the press-time offset, set in
    // mousePressEvent and read in mouseMoveEvent/mouseReleaseEvent.
};
```

- [ ] **Step 1: Write the failing tests** in `tests/ui/test_lighting_canvas_widget.cpp` (same pattern as `test_tray_app.cpp`: construct off-screen, drive it programmatically, no real display needed for most assertions):
  - `testSetChannel_thenRemoveChannel_doesNotCrash`.
  - `testMousePressDragRelease_onAChannelBox_emitsChannelPlacementChanged`: synthesize a press/move/release sequence (via `QTest::mousePress`/`mouseMove`/`mouseRelease` on the widget) at the known screen position of a registered channel box; assert the signal fires with a placement different from the one passed to `setChannel`.
  - `testMousePressDragRelease_onGlobalEffectHandle_emitsGlobalEffectReferencePointChanged`: same shape, with `setGlobalEffect` given an `IPositionableEffect`-implementing fake.
  - `testPaintEvent_doesNotCrashWithNoChannelsRegistered` (empty-canvas edge case).
- [ ] **Step 2: Run to verify failure.**
- [ ] **Step 3: Implement** `LightingCanvasWidget`. The background render samples `globalEffect_->colorAt(x,y,elapsedMs)` per-pixel (or per small block, e.g. every 4px, for performance — implementer's call) via `QPainter::drawRect`/`fillRect`; each channel box is drawn as a labeled rounded rectangle at its placement, filled with `ownEffect->colorAt(0,0,elapsedMs)` when `source == Independent`, or left as an outline over the background when `source == GlobalFollow` (matches the mockup's visual language).
- [ ] **Step 4: Run to verify pass.**
- [ ] **Step 5: Commit**

```bash
git add src/ui/LightingCanvasWidget.h src/ui/LightingCanvasWidget.cpp tests/ui/test_lighting_canvas_widget.cpp tests/CMakeLists.txt
git commit -m "feat: add LightingCanvasWidget"
```

---

## Task 15: `LightingStudioWindow`

**Files:**
- Create: `src/ui/LightingStudioWindow.h`, `src/ui/LightingStudioWindow.cpp`
- Test: `tests/ui/test_lighting_studio_window.cpp`

**Interfaces:**
- Consumes: `LightingCanvasWidget` (Task 14), `Settings`/`Profile`/`ChannelSettings` (Task 13).
- Produces: the window's public API, consumed by Task 17 (tray wiring) and Task 18 (final wire-up).

```cpp
// src/ui/LightingStudioWindow.h
// Main window: LightingCanvasWidget on the left, a side panel on the right
// listing every known channel with a mode/source/effect picker per row,
// and a profile selector at the top. Writes back into the Settings
// instance it's given -- it does not own persistence (main.cpp still
// calls Settings::saveToFile on quit, unchanged from v1).
class LightingStudioWindow : public QWidget {
    Q_OBJECT
public:
    // channelNames: every channel this build knows about (6 motherboard +
    // 4 peripherals), used to populate the side panel and the canvas even
    // before any Settings data exists for a channel (fresh profile case).
    LightingStudioWindow(Settings& settings, std::vector<std::string> channelNames, QWidget* parent = nullptr);

    LightingCanvasWidget* canvas() const; // exposed for tests and for main.cpp to connect Canvas::setAssignment on changes

signals:
    // Fired whenever any row's mode/source/effect/placement changes, or the
    // active profile switches -- main.cpp listens to push the new
    // assignment into the running Canvas (Task 18), not just to Settings.
    void channelAssignmentChanged(const std::string& channelName);
    void globalEffectChanged();
    void activeProfileChanged(const std::string& profileName);

private:
    Settings& settings_;
    LightingCanvasWidget* canvasWidget_;
    // one row widget per channel name in the side panel, built from
    // channelNames in the constructor
};
```

- [ ] **Step 1: Write the failing tests** in `tests/ui/test_lighting_studio_window.cpp`:
  - `testConstruction_populatesCanvasForEveryChannelName`: construct with 3 fake channel names and a `Settings` with one matching profile; assert `canvas()` has a box registered for each (no direct getter needed — assert indirectly via `channelAssignmentChanged` not firing spuriously, or expose a minimal test-only accessor if needed).
  - `testChangingARowsModeInUi_emitsChannelAssignmentChanged`: drive whatever concrete widget Step 3 picks for the mode picker (e.g. a `QComboBox`) via `QTest`, assert the signal fires with the right channel name.
  - `testSwitchingActiveProfile_emitsActiveProfileChangedAndRepopulatesCanvas`.
- [ ] **Step 2: Run to verify failure.**
- [ ] **Step 3: Implement.** Row widget layout (which exact `QComboBox`/`QColorDialog`/slider combination per field) is this task's own call — the design doc explicitly leaves exact widget choice to the builder, guided at this step by the project's `emil-design-eng`/`apple-design`/`animate` skills for spacing, motion, and restraint rather than a default `QFormLayout` dump.
- [ ] **Step 4: Run to verify pass.**
- [ ] **Step 5: Commit**

```bash
git add src/ui/LightingStudioWindow.h src/ui/LightingStudioWindow.cpp tests/ui/test_lighting_studio_window.cpp tests/CMakeLists.txt
git commit -m "feat: add LightingStudioWindow"
```

---

## Task 16: Tray integration — open Lighting Studio, profile quick-switch

**Files:**
- Modify: `src/ui/TrayApp.h`, `src/ui/TrayApp.cpp`
- Modify: `tests/ui/test_tray_app.cpp`

**Interfaces:**
- Produces: two new signals, consumed by Task 18's wire-up.

```cpp
// src/ui/TrayApp.h additions
signals:
    void openStudioRequested();            // new: "Open Lighting Studio" menu entry
    void profileSelected(const std::string& profileName); // new: a profile submenu entry was clicked

public:
    // Replaces the profile submenu's contents; called by main.cpp whenever
    // Settings.profiles changes (e.g. after a save in the Studio window).
    void setAvailableProfiles(const std::vector<std::string>& profileNames, const std::string& activeProfileName);
```

- [ ] **Step 1: Write the failing tests** in `tests/ui/test_tray_app.cpp` (same pattern as v1's existing autostart-toggle test, driving `contextMenu()`'s actions directly, no visible tray needed):
  - `testOpenStudioMenuEntry_triggersOpenStudioRequested`.
  - `testSetAvailableProfiles_thenClickingAnEntry_emitsProfileSelectedWithThatName`.
  - `testSetAvailableProfiles_marksTheActiveProfileChecked`.
- [ ] **Step 2: Run to verify failure.**
- [ ] **Step 3: Implement.**
- [ ] **Step 4: Run to verify pass**, and confirm existing `test_tray_app` cases (Quit, autostart toggle) still pass unchanged.
- [ ] **Step 5: Commit**

```bash
git add src/ui/TrayApp.h src/ui/TrayApp.cpp tests/ui/test_tray_app.cpp
git commit -m "feat: add Open Lighting Studio and profile quick-switch to the tray menu"
```

---

## Task 17: `LICENSE` and README update

**Files:**
- Create: `LICENSE`
- Modify: `README.md`

**Interfaces:** none (no code).

- [ ] **Step 1: Add `LICENSE`** containing the standard GPL-2.0-or-later text (the exact, unmodified license text — copy it verbatim from `OpenRGB/LICENSE` in the user's local checkout, which already carries this exact license).
- [ ] **Step 2: Add a short README section** stating the project is licensed GPL-2.0-or-later as of this change, and that the Roccat/Razer/SteelSeries peripheral drivers are adapted from OpenRGB (GPL-2.0-or-later), linking OpenRGB's project as the upstream source.
- [ ] **Step 3: Commit**

```bash
git add LICENSE README.md
git commit -m "docs: add GPL-2.0-or-later LICENSE, credit OpenRGB for peripheral drivers"
```

---

## Task 18: Final wire-up and real-hardware verification

**Files:**
- Modify: `src/main.cpp`
- Modify: `CMakeLists.txt` (add every new `.cpp` from Tasks 1-17 to the `MsiMysticLightApp` target's source list)

**Interfaces:**
- Consumes every public interface produced by Tasks 1-17.

- [ ] **Step 1: Extend `main.cpp`'s device setup** (Step 2-3 region of the existing v1 wire-up): open each of the four peripherals' `IHidDevice`(s) (five opens total: motherboard, Kone, Basilisk V3, Apex 3, and Vulcan's two), each with its confirmed VID/PID from the Global Constraints table and, for Vulcan, the two distinct interface numbers found while implementing Task 12. Every `open()` call's return value is ignored at this point, same as v1's existing motherboard `hid.open()` call — a disconnected peripheral must not block startup (Review Focus item).
- [ ] **Step 2: Build the `driversByChannel` map** passed to `Canvas`'s new constructor (Task 3): the motherboard's 6 channel names all map to one `MsiMotherboardDriver` instance; `"RoccatVulcan"`, `"RoccatKone"`, `"RazerBasiliskV3"`, `"SteelSeriesApex3"` each map to their own driver instance.
- [ ] **Step 3: Load `Settings`**, resolve `activeProfile()`, apply each channel's `ChannelSettings` (mode, source, placement, effect-or-native-settings) to `Canvas` via `setAssignment` (extended per Task 3), and apply `GlobalEffectSettings` via `Canvas::setGlobalEffect` — same "fall back to Native/Steady/black defaults if the file is missing/corrupt" tolerance v1 already has, now for the full `Profile` shape.
- [ ] **Step 4: Construct `LightingStudioWindow`** with the 10 channel names (6 motherboard + 4 peripheral) and the loaded `Settings`; connect its `channelAssignmentChanged`/`globalEffectChanged`/`activeProfileChanged` signals to push updates into the running `Canvas` and to `TrayApp::setAvailableProfiles`.
- [ ] **Step 5: Wire `TrayApp::openStudioRequested`** to show/raise the `LightingStudioWindow`, and `TrayApp::profileSelected` to switch `Settings.activeProfileName`, reload that profile's assignments into `Canvas`, and refresh the `LightingStudioWindow`.
- [ ] **Step 6: Build the app** (`cmake --build build`) and confirm it still starts with the tray icon visible even with every peripheral physically disconnected (manual check — plug nothing in, launch, confirm tray appears and no crash).
- [ ] **Step 7: Real-hardware verification, one device at a time, following the project's standing safety rule.** For each of the five devices in turn: connect it, show the exact bytes the first `pushCanvasColors`/`setNativeMode` call for that device would send (same plain-language byte table format used in v1's Task 12), get explicit human confirmation, then run the app and confirm the device visibly lights up as expected. Do not proceed to the next device until the current one is confirmed working or explicitly deferred by the user.
- [ ] **Step 8: Run the full test suite one last time.**

Run: `ctest --output-on-failure`
Expected: all tests PASS.

- [ ] **Step 9: Commit**

```bash
git add src/main.cpp CMakeLists.txt
git commit -m "feat: wire up Lighting Studio with all five devices"
```

---

## Self-Review Notes

- **Spec coverage:** every "Decisions already made" item in the design doc has an owning task (device roster → Tasks 9-12; physical-mirror layout + draggable reference point → Task 14; both follow/independent modes → Task 3; JRGB1 unchanged → no task touches it, correctly; peripherals direct-mode-only → each driver's `setNativeMode` always returns `false`; per-key/zone sampling → each driver's `createDefaultChannel` placement note; licensing → Task 17). The two builder-check findings (direct-mode-only, per-key/zone granularity) are both reflected in Tasks 9-14.
- **Type consistency:** `ChannelMode`/`NativeModeSettings` (Task 1) → `IDriver` (Task 1) → every driver (Tasks 9-12) → `Canvas` (Task 3) → `Settings::ChannelSettings` (Task 13) → `LightingStudioWindow`/`main.cpp` (Tasks 15, 18) all reference the same names throughout; `CanvasSource`/`CanvasPlacement` (Task 3) flow unchanged into Settings (13), the canvas widget (14), and the window (15).
- **Proportion:** protocol-level byte tables for the three more complex peripherals (Kone, Basilisk V3, Vulcan) are deliberately not transcribed into this plan — each task instead cites the exact OpenRGB file/function to port from, since fabricating byte values without re-reading those files in full would risk an incorrect plan being treated as authoritative. The one protocol simple enough to verify by inspection (SteelSeries Apex 3) is fully pinned in Task 9.

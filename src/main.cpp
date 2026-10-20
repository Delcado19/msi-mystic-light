// Application entry point: wires together every device driver (the MSI
// motherboard plus the four peripherals added by the Lighting Studio plan),
// the Canvas render loop, Settings persistence, the Lighting Studio window,
// and the tray UI into the running app. This is Task 18 (the final task) of
// docs/superpowers/plans/2026-10-19-lighting-studio.md -- see that plan's
// Task 18 for the step list this file follows; .superpowers/sdd/
// 2026-10-19-lighting-studio/task-18-report.md records the exact byte
// tables derived for each device's first push (for Step 7's real-hardware
// confirmation, not yet run) and the implementation decisions noted inline
// below. No new public interface is produced; this file is consumed by
// nothing else.
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QMenu>
#include <QStandardPaths>
#include <QString>
#include <QTimer>

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "canvas/Canvas.h"
#include "device/Channel.h"
#include "device/IDriver.h"
#include "device/MsiMotherboardDriver.h"
#include "device/RazerBasiliskDriver.h"
#include "device/RoccatKoneDriver.h"
#include "device/RoccatVulcanDriver.h"
#include "device/SteelSeriesApex3Driver.h"
#include "effects/BreathingEffect.h"
#include "effects/CometEffect.h"
#include "effects/MarqueeEffect.h"
#include "effects/RadialRainbowEffect.h"
#include "effects/RainbowWaveEffect.h"
#include "effects/SpiralEffect.h"
#include "effects/StaticColorEffect.h"
#include "effects/SwirlCirclesEffect.h"
#include "hid/HidApiDevice.h"
#include "platform/win/AutostartWin.h"
#include "settings/Settings.h"
#include "ui/LightingCanvasWidget.h" // needed for the channelPlacementChanged signal connect below (Task 20, Bug B)
#include "ui/LightingStudioWindow.h"
#include "ui/TrayApp.h"

namespace {

// Minimal effectName -> IEffect factory. LightingStudioWindow.cpp carries
// its own copy of this exact mapping for its preview widget; there is still
// no shared factory anywhere in the codebase (see that file's "Minimal
// effectName -> IEffect factory" comment), so this is a second, independent
// instance of the same small if/else chain rather than an extracted shared
// header for two callers -- per the plan's Task 18, Step 3 and the
// established precedent from Task 15's own report. Both copies must be kept
// in sync with Settings' effectName contract if a new effect is ever added.
std::shared_ptr<IEffect> buildEffect(const std::string& effectName, const MsiColor& color, int referenceX, int referenceY)
{
    // ponytail: CometEffect's trackWidthPx has no stored Settings field to
    // read (no canvas-width concept persisted); fixed to a sane constant
    // for now, matching LightingStudioWindow.cpp's own copy of this
    // simplification. Add a real field if Comet's track width ever needs
    // tuning.
    constexpr int kCometTrackWidthPx = 300;
    if (effectName == "StaticColor") return std::make_shared<StaticColorEffect>(color);
    if (effectName == "RainbowWave") return std::make_shared<RainbowWaveEffect>();
    if (effectName == "Breathing") return std::make_shared<BreathingEffect>(color);
    if (effectName == "RadialRainbow") return std::make_shared<RadialRainbowEffect>(referenceX, referenceY);
    if (effectName == "Spiral") return std::make_shared<SpiralEffect>(referenceX, referenceY);
    if (effectName == "Comet") return std::make_shared<CometEffect>(color, kCometTrackWidthPx);
    if (effectName == "Marquee") return std::make_shared<MarqueeEffect>(color);
    if (effectName == "SwirlCircles") return std::make_shared<SwirlCirclesEffect>(referenceX, referenceY);
    return nullptr; // unknown/empty name -- Canvas tolerates a null effect by skipping the channel for the frame
}

// Builds the Canvas-ready assignment for one persisted ChannelSettings
// entry. Native mode carries its NativeModeSettings verbatim. Canvas mode
// with CanvasSource::Independent resolves effectName to a concrete IEffect;
// CanvasSource::GlobalFollow channels get no own effect at all -- Canvas::
// tick() (canvas/Canvas.cpp) samples the shared globalEffect_ for those
// instead of assignment.effect, exactly mirroring LightingStudioWindow::
// applyChannelToCanvas's same split for its own preview widget. An
// unrecognized effectName (e.g. from a hand-edited settings.json) leaves
// effect null; Canvas::tick already tolerates a null effect by skipping
// that channel for the frame rather than crashing.
ChannelAssignment buildAssignment(const ChannelSettings& cs)
{
    ChannelAssignment assignment;
    assignment.mode = cs.mode;
    assignment.source = cs.source;
    assignment.placement = cs.placement;
    if (cs.mode == ChannelMode::Native) {
        assignment.nativeSettings = cs.nativeSettings;
    } else if (cs.source == CanvasSource::Independent) {
        assignment.effect = buildEffect(cs.effectName, cs.staticColor, 0, 0);
    }
    return assignment;
}

// Pushes `cs.mode` into the channel's driver (setChannelMode -- needed so
// e.g. RoccatVulcanDriver sends its one-time EnableDirect report, and so
// MsiMotherboardDriver's internal channelMode_ state matches what Canvas is
// about to push/send). Split out from applyChannelSettingsToCanvas() below
// so the channelAssignmentChanged handler (Task 20, Bug A) can call *only*
// this -- gated on whether `cs.mode` itself actually changed, the only field
// this function reads -- separately from the Canvas-assignment push, which
// is gated on a wider set of fields (see channelSettingsNeedFullReapply()
// below). For MsiMotherboardDriver, setChannelMode(Canvas) resends a full
// 290-byte 0x50 report that arms the zone to Advanced mode and carries every
// other zone's last-sent state along with it (device/MsiMotherboardDriver.cpp,
// commit 2884a12); for RoccatVulcanDriver it resends the one-time
// EnableDirect report. Both are meant to run once per actual mode change,
// not on every mouse-move pixel of a drag, nor on every effect/color tweak
// that leaves the mode itself unchanged -- see
// .superpowers/sdd/2026-10-19-lighting-studio/progress.md line 111
// ("full resend on every canvas-drag pixel").
void applyChannelModeToDriver(const std::map<std::string, IDriver*>& driversByChannel, const ChannelSettings& cs)
{
    auto it = driversByChannel.find(cs.channelName);
    if (it != driversByChannel.end() && it->second) {
        it->second->setChannelMode(cs.channelName, cs.mode);
    }
}

// Pushes one channel's current ChannelSettings into both its driver
// (applyChannelModeToDriver, see above) and Canvas itself (setAssignment).
// Used for the initial per-channel apply at startup below, where both halves
// are always needed unconditionally (there is no "previous" to gate against
// yet). The channelAssignmentChanged handler further down calls
// applyChannelModeToDriver()/canvas.setAssignment() separately instead of
// through this combined function, since after startup the two halves are
// gated independently (Task 20, Bug A).
void applyChannelSettingsToCanvas(Canvas& canvas, const std::map<std::string, IDriver*>& driversByChannel, const ChannelSettings& cs)
{
    applyChannelModeToDriver(driversByChannel, cs);
    canvas.setAssignment(cs.channelName, buildAssignment(cs));
}

// Task 21 (USB peripheral reconnect/retry): re-applies every channel
// currently routed to `driver` using its CURRENT ChannelSettings, read live
// from `settings.activeProfile()` rather than the lastAppliedChannelSettings
// cache. The two happen to agree in practice (the startup loop and the
// channelAssignmentChanged handler both keep the cache in lockstep with
// settings), but they answer different questions: `settings` is the user's
// actual intent (what the lighting SHOULD be), while the cache is Task 20's
// own bookkeeping for its placement-drag gate (what was last PUSHED,
// regardless of whether the push reached real hardware). A reconnect needs
// to restore intent, not replay the gate's bookkeeping -- reading `settings`
// directly is correct by construction, not just correct because the two
// currently happen to match.
//
// Matches channels by `driversByChannel`'s driver pointer rather than a
// hardcoded channel name, so this works unchanged if a driver is ever
// routed to more than one channel. Calls applyChannelSettingsToCanvas() --
// the same unconditional helper Step 4b below uses for the initial
// startup apply -- which always calls through to both the driver's
// setChannelMode() and canvas.setAssignment(). This is deliberate: a
// reconnect is not a placement-only drag (Task 20, Bug A's gate, see
// channelSettingsNeedFullReapply() below) even though the ChannelSettings
// value itself hasn't changed since last time -- it's the device, not the
// settings, that changed, so the gated channelAssignmentChanged path (and
// its cache-driven no-op) must not be the one used here. The cache is
// still updated afterward so later gated comparisons stay correct.
void reapplyChannelsForDriver(
    Settings& settings,
    Canvas& canvas,
    const std::map<std::string, IDriver*>& driversByChannel,
    std::map<std::string, ChannelSettings>& lastAppliedChannelSettings,
    IDriver* driver)
{
    Profile* profile = settings.activeProfile();
    if (!profile) {
        return; // defensive; never null once LightingStudioWindow has constructed
    }
    for (const ChannelSettings& cs : profile->channelSettings) {
        auto driverIt = driversByChannel.find(cs.channelName);
        if (driverIt == driversByChannel.end() || driverIt->second != driver) {
            continue;
        }
        applyChannelSettingsToCanvas(canvas, driversByChannel, cs);
        lastAppliedChannelSettings[cs.channelName] = cs;
    }
}

// True unless `next` differs from `previous` only in `placement` -- i.e.
// true when the channel's mode/source/effect/color/native-settings actually
// changed and Canvas needs a fresh ChannelAssignment (canvas.setAssignment(),
// which rebuilds the channel's IEffect and/or re-arms Canvas's native-mode
// resend -- see canvas/Canvas.cpp's setAssignment()), false when only the
// on-canvas x/y position moved (a drag). Note this is a *wider* gate than
// applyChannelModeToDriver()'s (any of the above fields, not just `mode`) --
// Canvas needs the full rebuild for an effect/color/native-settings change
// even though the driver's setChannelMode() doesn't care about any of those.
// Task 20, Bug A: LightingCanvasWidget::channelPlacementChanged fires on
// every mouseMoveEvent during a drag (see LightingCanvasWidget.h), which
// LightingStudioWindow::onCanvasChannelPlacementChanged relays into
// channelAssignmentChanged once per pixel -- so without this gate, dragging
// a channel's box rebuilt its IEffect/resent native-mode settings on every
// pixel. Extracted as a free, pure function (no Canvas/IDriver/Settings-file
// dependency) so the gating decision itself is a small, independently-
// reasoned-about unit even though main.cpp has no existing test harness to
// host a regression test in (see task-20-report.md for why no test file was
// added for this).
bool channelSettingsNeedFullReapply(const ChannelSettings& previous, const ChannelSettings& next)
{
    ChannelSettings previousIgnoringPlacement = previous;
    ChannelSettings nextIgnoringPlacement = next;
    previousIgnoringPlacement.placement = CanvasPlacement{};
    nextIgnoringPlacement.placement = CanvasPlacement{};
    return !(previousIgnoringPlacement == nextIgnoringPlacement);
}

void applyGlobalEffectToCanvas(Canvas& canvas, const GlobalEffectSettings& g)
{
    canvas.setGlobalEffect(buildEffect(g.effectName, g.color, g.referenceX, g.referenceY));
}

// Rebuilds the tray's profile submenu from Settings.profiles. Called once
// after construction (TrayApp has no profiles yet at that point) and again
// every time LightingStudioWindow::activeProfileChanged fires.
void refreshTrayProfiles(TrayApp& tray, const Settings& settings)
{
    std::vector<std::string> names;
    for (const Profile& p : settings.profiles) names.push_back(p.name);
    tray.setAvailableProfiles(names, settings.activeProfileName);
}

// Task 21 (USB peripheral reconnect/retry): one entry per reconnectable USB
// peripheral (the 4 named in this task's brief; the motherboard's
// mbHid/mbDriver deliberately has no entry -- see this task's report for
// why). `isOpen`/`reopen` close over that device's own IHidDevice
// instance(s) -- `reopen` reuses the exact same open() lambda main()
// defines for that device's initial startup open (Step 2), so there is
// only ever one place that knows this device's vendorId/productId/
// interfaceNumber/usagePage. `reopen` returns whether the device ended up
// open, not whether the underlying open() call "succeeded" in isolation
// (relevant for the Vulcan, which needs two separate HID interfaces both
// open before it counts as reconnected).
struct ReconnectiblePeripheral {
    std::string name; // for the reconnect-attempt log line only; not a channel-name lookup key
    IDriver* driver;
    std::function<bool()> isOpen;
    std::function<bool()> reopen;
};

} // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv); // Step 1
    // The tray icon/menu are not QWidgets and don't count toward Qt's
    // "last window" tracking, so without this, closing LightingStudioWindow
    // (its own 'X' button, not the tray's Quit) would quit the whole app --
    // and skip the quitRequested-triggered settings save, silently losing
    // any edit the user just made in the Studio window. The tray's own
    // Quit action is the only intended way to exit.
    app.setQuitOnLastWindowClosed(false);

    // Step 2: attempt to open every device's HID handle(s), but proceed
    // regardless of success -- the tray must still come up with any subset
    // of devices unplugged (Review Focus, carried from v1: a disconnected
    // device must never crash/abort startup). Every open() call's return
    // value is intentionally unused except where noted below.
    HidApiDevice mbHid;
    mbHid.open(0x0DB0, 0x0076);
    // Task 21: the motherboard is a different, internal, permanently-
    // attached device (not a hot-pluggable USB peripheral) and is
    // explicitly out of scope for this task's reconnect/retry mechanism --
    // see the 4 peripherals' ReconnectiblePeripheral entries further down,
    // which mbHid deliberately has no equivalent of. Disable
    // close-on-I/O-error for mbHid specifically: without a retry timer
    // watching it, closing its handle on a transient I/O blip would strand
    // it disconnected for the rest of the app's lifetime (see
    // HidApiDevice::setCloseOnIoError()'s doc comment).
    mbHid.setCloseOnIoError(false);

    HidApiDevice koneHid;
    // Roccat Kone Aimo ships under two PIDs (the plain Aimo and the Aimo
    // 16K); RoccatKoneDriver is PID-agnostic (it only matches on channel
    // name), so try the plain Aimo PID first and fall back to the 16K PID
    // on the same handle before giving up, same as the Vulcan's two-PID
    // family below. interfaceNumber=0, usagePage=0x0B per
    // RoccatControllerDetect.cpp:414-415 (both PIDs' REGISTER_HID_DETECTOR_IPU:
    // interface_number=0, usage_page=0x0B, usage=0). The usagePage filter is
    // required, not optional: interface_number=0 alone splits into 5 separate
    // HID collections on this machine's Kone Aimo (confirmed via
    // Get-PnpDevice), and without usagePage, open() landed on the wrong one --
    // live-tested, both pure red and pure green writes produced the same
    // fixed orange (the device's own idle default, unaffected by our writes).
    // Confirmed fixed with usagePage=0x0B: red now shows as red.
    constexpr uint16_t kKoneVid = 0x1E7D;
    constexpr uint16_t kKonePidAimo = 0x2E27;
    constexpr uint16_t kKonePid16K = 0x2E2C;
    constexpr int kKoneIface = 0;
    constexpr int kKoneUsagePage = 0x0B;
    // Task 21: named so the reconnect timer's retry (built further down)
    // can reopen this device with the exact same two-PID fallback and
    // constants as this initial open, instead of a second, possibly
    // drifting copy of this logic.
    auto openKoneHid = [&koneHid]() {
        if (!koneHid.open(kKoneVid, kKonePidAimo, kKoneIface, kKoneUsagePage)) {
            koneHid.open(kKoneVid, kKonePid16K, kKoneIface, kKoneUsagePage);
        }
        return koneHid.isOpen();
    };
    openKoneHid();

    HidApiDevice basiliskHid;
    // interfaceNumber=3 per RazerControllerDetect.cpp:361
    // (REGISTER_HID_DETECTOR_IPU("Razer Basilisk V3", ..., 0x03, 0x0C, 0x01):
    // interface_number=3, usage_page=0x0C, usage=0x01). IHidDevice::open()
    // can filter on usage_page too (added for the Kone, see below), but a
    // read-only Get-PnpDevice enumeration on this machine (where this mouse
    // is actually connected) confirmed interface 3 is a single, unambiguous
    // collection for this device -- no other collection shares it -- so the
    // interface-only filter is sufficient here; usage_page isn't needed.
    // Task 21: named for the same reason as openKoneHid above.
    auto openBasiliskHid = [&basiliskHid]() {
        basiliskHid.open(0x1532, 0x0099, 3);
        return basiliskHid.isOpen();
    };
    openBasiliskHid();

    HidApiDevice apex3Hid;
    // interfaceNumber=3 per SteelSeriesControllerDetect.cpp:543
    // (REGISTER_HID_DETECTOR_I("SteelSeries Apex 3", ..., 3): this detector
    // variant only ever specifies interface_number, no usage_page -- there
    // is no more specific filter to apply even though IHidDevice::open()
    // now supports one. Confirmed via a read-only Get-PnpDevice enumeration
    // on this machine (where this keyboard is actually connected):
    // interface 3 is a single, unambiguous collection.
    // Task 21: named for the same reason as openKoneHid above.
    auto openApex3Hid = [&apex3Hid]() {
        apex3Hid.open(0x1038, 0x161A, 3);
        return apex3Hid.isOpen();
    };
    openApex3Hid();

    // Roccat Vulcan 100/120 Aimo: one physical device, two HID interfaces
    // under the same VID/PID -- control (mode/EnableDirect feature reports,
    // interface 1) and LED (per-key color output reports, interface 3), per
    // Task 12's confirmed RoccatControllerDetect.cpp wiring
    // (dev_ctrl_iface=1, dev_led_iface=3; see task-12-report.md's
    // "Control-interface wiring note"). Both target PIDs (100/120 Aimo) are
    // byte-for-byte identical on the wire (RoccatVulcanProtocol.h), so try
    // 100 Aimo first, then 120, opening both interfaces on whichever PID
    // matches.
    // usage_page is also pinned here (0x000B control, 0x0001 LED), confirmed
    // directly from RoccatControllerDetect.cpp's default-case switch (which
    // both target PIDs hit, same as task-12-report.md's citation) against
    // the same collection-ambiguity risk confirmed live on the Kone Aimo
    // above. This is strictly MORE restrictive than the pre-fix
    // interface-only match, not a free precaution: if these usage_page
    // values are wrong for a specific physical unit, open() now fails
    // outright instead of possibly working by luck. Unlike the Kone fix,
    // this has NOT been verified against a real Vulcan (none physically
    // available) -- treat it as plausible-but-unconfirmed until tested.
    HidApiDevice vulcanControlHid;
    HidApiDevice vulcanLedHid;
    constexpr uint16_t kVulcanVid = 0x1E7D;
    constexpr uint16_t kVulcanPids[] = {0x307A, 0x3098}; // 100 Aimo, 120 Aimo
    constexpr int kVulcanCtrlIface = 1;
    constexpr int kVulcanLedIface = 3;
    constexpr int kVulcanCtrlUsagePage = 0x000B;
    constexpr int kVulcanLedUsagePage = 0x0001;
    // Task 21: named for the same reason as openKoneHid above. Both
    // interfaces always belong to the one physical device actually
    // present, so they only ever pair under the SAME loop iteration's pid
    // -- there is no scenario where control ends up open under one PID
    // while led is open (or retried) under a different one. Explicitly
    // closes both first so every call (startup or a later retry) starts
    // from a clean slate rather than relying only on open()'s own
    // close-before-reopen; if control opens but led doesn't (the device
    // vanishing mid-attempt), control is closed again so this driver is
    // never left half-open.
    auto openVulcanHid = [&vulcanControlHid, &vulcanLedHid, kVulcanVid, &kVulcanPids, kVulcanCtrlIface, kVulcanLedIface, kVulcanCtrlUsagePage, kVulcanLedUsagePage]() {
        vulcanControlHid.close();
        vulcanLedHid.close();
        for (uint16_t pid : kVulcanPids) {
            if (vulcanControlHid.open(kVulcanVid, pid, kVulcanCtrlIface, kVulcanCtrlUsagePage)) {
                vulcanLedHid.open(kVulcanVid, pid, kVulcanLedIface, kVulcanLedUsagePage);
                break;
            }
        }
        if (vulcanControlHid.isOpen() && !vulcanLedHid.isOpen()) {
            vulcanControlHid.close();
        }
        return vulcanControlHid.isOpen() && vulcanLedHid.isOpen();
    };
    openVulcanHid();

    // Step 3: one driver instance per physical device; the motherboard's 6
    // channels all route to the same driver instance, each peripheral's one
    // channel routes to its own.
    MsiMotherboardDriver mbDriver(mbHid);
    RoccatKoneDriver koneDriver(koneHid);
    RazerBasiliskDriver basiliskDriver(basiliskHid);
    SteelSeriesApex3Driver apex3Driver(apex3Hid);
    RoccatVulcanDriver vulcanDriver(vulcanControlHid, vulcanLedHid);

    std::map<std::string, Channel> channels;
    std::map<std::string, IDriver*> driversByChannel;
    for (Channel& channel : MsiMotherboardDriver::createDefaultChannels()) {
        std::string name = channel.name();
        driversByChannel[name] = &mbDriver;
        channels.emplace(std::move(name), std::move(channel));
    }
    auto addPeripheralChannel = [&channels, &driversByChannel](Channel channel, IDriver* driver) {
        std::string name = channel.name();
        driversByChannel[name] = driver;
        channels.emplace(std::move(name), std::move(channel));
    };
    addPeripheralChannel(RoccatVulcanDriver::createDefaultChannel(), &vulcanDriver);
    addPeripheralChannel(RoccatKoneDriver::createDefaultChannel(), &koneDriver);
    addPeripheralChannel(RazerBasiliskDriver::createDefaultChannel(), &basiliskDriver);
    addPeripheralChannel(SteelSeriesApex3Driver::createDefaultChannel(), &apex3Driver);

    std::vector<std::string> channelNames;
    for (const auto& [name, channel] : channels) {
        (void)channel;
        channelNames.push_back(name);
    }

    // Task 23, Part 1: query each channel's own driver for which of
    // {Native, Canvas} it actually supports (IDriver::supportsMode -- a
    // non-mutating capability query mirroring each driver's own
    // setChannelMode accept/reject rule) and hand the result to
    // LightingStudioWindow, so its mode combo per row only ever offers a
    // mode that channel's hardware will really honor -- e.g. the 4
    // peripherals are Canvas-only, and the motherboard's JRGB1 is
    // Native-only. This is the one place that owns the query; the UI layer
    // never re-derives per-device-name rules of its own.
    std::map<std::string, std::vector<ChannelMode>> availableModesByChannel;
    for (const auto& [name, driver] : driversByChannel) {
        std::vector<ChannelMode> modes;
        if (driver && driver->supportsMode(name, ChannelMode::Native)) modes.push_back(ChannelMode::Native);
        if (driver && driver->supportsMode(name, ChannelMode::Canvas)) modes.push_back(ChannelMode::Canvas);
        availableModesByChannel[name] = std::move(modes);
    }

    // Step 4a: load persisted settings. loadFromFile returns false on a
    // missing/corrupt file and leaves settings untouched (default-
    // constructed: no profiles) per its documented contract -- the return
    // value is intentionally unused here, since the fallback below (Step
    // 4b, via LightingStudioWindow's construction) already covers that
    // case without needing a branch on it.
    const QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataDir); // first run: AppDataLocation may not exist yet
    const QString settingsPath = appDataDir + QStringLiteral("/settings.json");

    Settings settings;
    settings.loadFromFile(settingsPath.toStdString());

    Canvas canvas(driversByChannel, channels);

    // Step 4b: construct the Studio window before the initial Canvas push,
    // not strictly after as the plan's step numbering reads. Its
    // constructor already calls the private ensureProfileAndEntries()
    // (Task 15; Task 23 extended it), which pushes a "Default" profile if
    // settings.profiles was empty and fills in a default entry for any
    // channel missing one -- Native/Steady/black (ChannelSettings'/
    // NativeModeSettings' own default member values) for a motherboard
    // channel, but Canvas/StaticColor/white for a Canvas-only peripheral
    // channel (Task 23: Native is a confirmed permanent no-op for all 4
    // peripheral drivers, so the plain Native/black default would leave it
    // invisible forever) -- plus clamping any EXISTING entry whose persisted
    // mode this channel no longer supports. Reusing that existing,
    // already-tested fallback instead of duplicating an equivalent
    // default-filling loop here means settings.activeProfile() is
    // guaranteed non-null and complete for every line below.
    LightingStudioWindow studio(settings, channelNames, availableModesByChannel);

    // Task 20, Bug A: last ChannelSettings actually pushed to the driver/
    // Canvas per channel, so the channelAssignmentChanged handler below can
    // tell "placement moved" apart from "mode/settings changed" (see
    // channelSettingsNeedFullReapply() above) instead of reapplying on
    // every canvas-drag pixel.
    std::map<std::string, ChannelSettings> lastAppliedChannelSettings;

    for (const ChannelSettings& cs : settings.activeProfile()->channelSettings) {
        applyChannelSettingsToCanvas(canvas, driversByChannel, cs);
        lastAppliedChannelSettings[cs.channelName] = cs;
    }
    applyGlobalEffectToCanvas(canvas, settings.activeProfile()->globalEffect);

    // Task 21: USB peripheral reconnect/retry. Review finding: the 4 USB
    // peripherals below have their HID handle(s) opened exactly once,
    // above -- if one is unplugged and replugged, or the machine sleeps/
    // resumes (both of which can invalidate a USB HID handle on Windows),
    // it stayed permanently failed for the rest of the app's lifetime with
    // no retry/reopen anywhere. Each entry's `isOpen`/`reopen` reuse the
    // exact open() lambda (and vendorId/productId/interfaceNumber/
    // usagePage constants) already defined above for that device's initial
    // startup open -- see openKoneHid/openBasiliskHid/openApex3Hid/
    // openVulcanHid. The motherboard (mbHid/mbDriver) deliberately has no
    // entry here -- see mbHid.setCloseOnIoError(false) above and this
    // task's report for why.
    std::vector<ReconnectiblePeripheral> reconnectPeripherals = {
        {"RoccatVulcan", &vulcanDriver, [&vulcanControlHid, &vulcanLedHid]() { return vulcanControlHid.isOpen() && vulcanLedHid.isOpen(); }, openVulcanHid},
        {"RoccatKone", &koneDriver, [&koneHid]() { return koneHid.isOpen(); }, openKoneHid},
        {"RazerBasiliskV3", &basiliskDriver, [&basiliskHid]() { return basiliskHid.isOpen(); }, openBasiliskHid},
        {"SteelSeriesApex3", &apex3Driver, [&apex3Hid]() { return apex3Hid.isOpen(); }, openApex3Hid},
    };

    // Interval: 5s. A failed hid_open() on an absent device is a cheap
    // Windows device enumeration, not a blocking call, so this isn't about
    // protecting the OS from load -- there's just no reason to poll faster
    // than a human plug-in event needs to be noticed in.
    QTimer reconnectTimer;
    QObject::connect(&reconnectTimer, &QTimer::timeout,
        [&reconnectPeripherals, &settings, &canvas, &driversByChannel, &lastAppliedChannelSettings]() {
            for (ReconnectiblePeripheral& peripheral : reconnectPeripherals) {
                if (peripheral.isOpen()) {
                    continue; // already up, nothing to do
                }
                if (!peripheral.reopen()) {
                    continue; // still down (device not present); try again next tick
                }
                qInfo() << "Reconnect: reopened" << QString::fromStdString(peripheral.name) << "-- re-applying its current lighting settings";
                // Re-apply this device's channel(s) to their CURRENT
                // settings: a freshly (re)plugged device resets to its own
                // firmware-default lighting state. This intentionally
                // bypasses Task 20's lastAppliedChannelSettings gate (see
                // reapplyChannelsForDriver()'s doc comment above) rather
                // than going through the gated channelAssignmentChanged
                // handler further down, since the ChannelSettings value
                // itself hasn't changed -- only the device has.
                reapplyChannelsForDriver(settings, canvas, driversByChannel, lastAppliedChannelSettings, peripheral.driver);
            }
        });
    reconnectTimer.start(5000);

    // Step 5 (render loop): ~60Hz, elapsed time measured from launch.
    QElapsedTimer clock;
    clock.start();
    QTimer renderTimer;
    QObject::connect(&renderTimer, &QTimer::timeout, [&canvas, &clock]() {
        canvas.tick(clock.elapsed());
    });
    renderTimer.start(16);

    // Step 5 (tray shell). Quit both persists settings (first, so the save
    // happens before teardown) and exits the app.
    TrayApp tray;
    QObject::connect(&tray, &TrayApp::quitRequested, [&settings, &settingsPath]() {
        settings.saveToFile(settingsPath.toStdString());
    });
    QObject::connect(&tray, &TrayApp::quitRequested, &app, &QApplication::quit);

    // Task 20, Bug B: Settings was previously only ever persisted on the
    // tray's Quit action above, so an autostart tray app that the user logs
    // off or shuts down (rather than explicitly choosing Quit) silently
    // lost every edit made since the last Quit. Debounces a save behind a
    // short idle window instead of writing on every single change signal
    // (which would mean a disk write per canvas-drag pixel -- the same
    // problem class as Bug A above): 2 seconds is long enough to coalesce a
    // drag or a burst of row edits into one write, short enough that an
    // unplanned shutdown can only ever lose ~2s of edits. Restarted (not
    // just started) on every relevant change below, and reuses the exact
    // same saveToFile() call the Quit path already uses -- no new
    // persistence code path/semantics. The Quit-time save above stays as an
    // immediate, non-debounced final flush.
    QTimer saveDebounceTimer;
    saveDebounceTimer.setSingleShot(true);
    saveDebounceTimer.setInterval(2000);
    QObject::connect(&saveDebounceTimer, &QTimer::timeout, [&settings, &settingsPath]() {
        settings.saveToFile(settingsPath.toStdString());
    });

    // Step 4/5: Studio window <-> Canvas/tray wiring. Every mutation the
    // Studio window makes is written into `settings` before it emits, so
    // each handler just re-reads settings.activeProfile() rather than
    // needing the new value passed through the signal.
    QObject::connect(&studio, &LightingStudioWindow::channelAssignmentChanged,
        [&settings, &canvas, &driversByChannel, &lastAppliedChannelSettings](const std::string& channelName) {
            Profile* profile = settings.activeProfile();
            if (!profile) return; // defensive; never null once `studio` exists
            for (const ChannelSettings& cs : profile->channelSettings) {
                if (cs.channelName != channelName) continue;

                // Task 20, Bug A gate: a canvas drag re-fires this signal on
                // every mouse-move pixel with only `cs.placement` different
                // from last time. Two independent gates below, matching the
                // brief's wording precisely rather than one coarse "did
                // anything change" flag:
                //  - applyChannelModeToDriver() (IDriver::setChannelMode --
                //    see its comment above for the full-report resend it can
                //    trigger) only runs when `cs.mode` itself flipped, since
                //    that's the only field it actually reads -- an
                //    effect/source/color/native-settings-only change (e.g.
                //    picking a new effect in the combo) must not re-arm the
                //    driver.
                //  - canvas.setAssignment() runs whenever anything besides
                //    placement changed (it needs the new effect/
                //    nativeSettings/etc.), plus the Canvas+GlobalFollow
                //    placement-only case below so that mode keeps tracking a
                //    drag live. Canvas+Independent and Native-mode
                //    placement-only changes are skipped entirely: Canvas::
                //    tick() never reads `placement` for either of those
                //    (only Canvas+GlobalFollow does), and calling
                //    setAssignment() would otherwise needlessly rebuild the
                //    channel's IEffect (Independent) or re-arm
                //    nativeModePending_ and resend the full native-mode
                //    report on the next tick for no reason (Native) -- see
                //    canvas/Canvas.cpp's setAssignment().
                auto cached = lastAppliedChannelSettings.find(channelName);
                const bool firstApply = cached == lastAppliedChannelSettings.end();
                const bool modeChanged = firstApply || cached->second.mode != cs.mode;
                const bool nonPlacementChanged = firstApply || channelSettingsNeedFullReapply(cached->second, cs);

                if (modeChanged) {
                    applyChannelModeToDriver(driversByChannel, cs);
                }
                if (nonPlacementChanged
                    || (cs.mode == ChannelMode::Canvas && cs.source == CanvasSource::GlobalFollow)) {
                    canvas.setAssignment(channelName, buildAssignment(cs));
                }

                lastAppliedChannelSettings[channelName] = cs;
                break;
            }
        });
    QObject::connect(&studio, &LightingStudioWindow::globalEffectChanged, [&settings, &canvas]() {
        Profile* profile = settings.activeProfile();
        if (!profile) return; // defensive; never null once `studio` exists
        applyGlobalEffectToCanvas(canvas, profile->globalEffect);
    });
    // Updates the tray's profile submenu/checkmark whenever the active
    // profile changes from inside the Studio window -- not just the profile
    // combo box anymore (Task 23): LightingStudioWindow::refreshFromSettings()
    // fires this unconditionally after a profile switch AND after a
    // create/rename/delete via its own createProfile()/renameActiveProfile()/
    // deleteActiveProfile(), so a rename or delete of a non-last profile
    // (which may leave "the active profile" pointing at the same slot,
    // merely renamed, or reassign it to a neighbor) still reaches the tray.
    QObject::connect(&studio, &LightingStudioWindow::activeProfileChanged, [&tray, &settings](const std::string&) {
        refreshTrayProfiles(tray, settings);
    });

    // Task 20, Bug B: restart the debounced save (declared above, next to
    // the tray's Quit-time save) on every one of the Studio's three change
    // signals, plus the canvas widget's own placement signal directly --
    // belt-and-suspenders with channelAssignmentChanged, since
    // LightingStudioWindow::onCanvasChannelPlacementChanged already relays
    // every canvas drag into channelAssignmentChanged (LightingStudioWindow.cpp),
    // but restarting from both costs nothing extra (QTimer::start() on an
    // already-running single-shot timer just re-arms it) and keeps this
    // wiring resilient to that relay ever changing.
    QObject::connect(&studio, &LightingStudioWindow::channelAssignmentChanged,
        [&saveDebounceTimer](const std::string&) { saveDebounceTimer.start(); });
    QObject::connect(&studio, &LightingStudioWindow::globalEffectChanged,
        [&saveDebounceTimer]() { saveDebounceTimer.start(); });
    QObject::connect(&studio, &LightingStudioWindow::activeProfileChanged,
        [&saveDebounceTimer](const std::string&) { saveDebounceTimer.start(); });
    QObject::connect(studio.canvas(), &LightingCanvasWidget::channelPlacementChanged,
        [&saveDebounceTimer](const std::string&, CanvasPlacement) { saveDebounceTimer.start(); });

    QObject::connect(&tray, &TrayApp::openStudioRequested, [&studio]() {
        studio.show();
        studio.raise();
        studio.activateWindow();
    });
    // TrayApp's own doc comment names this exact wiring ("switch profile ->
    // setAvailableProfiles") as its expected caller; refreshFromSettings()
    // re-syncs the window's own UI/preview AND re-emits
    // channelAssignmentChanged (per channel) + globalEffectChanged, which
    // the handlers above already route into the running Canvas -- so no
    // separate "reload into Canvas" loop is needed here.
    QObject::connect(&tray, &TrayApp::profileSelected, [&settings, &studio](const std::string& name) {
        settings.activeProfileName = name;
        studio.refreshFromSettings();
    });

    // Initial tray profile list: activeProfileChanged only fires on a later
    // switch, not at `studio`'s own construction.
    refreshTrayProfiles(tray, settings);

    // Step 6 (autostart toggle), keyed to the real installed exe path. The
    // plan leaves the menu item's exact wording/placement as an
    // implementation detail; placed as a checkable "Start with Windows"
    // item above whatever the menu's current first action is (now "Open
    // Lighting Studio" rather than v1's "Quit", per Task 16's menu-order
    // note in task-16-report.md) -- kept as-is deliberately: autostart
    // first is still sensible UX (the setting you touch once, up top; the
    // actions you use every session, below it), and no other `menu->...`
    // call in this file assumes "Quit" is first/only.
    AutostartWin autostart(L"MsiMysticLightApp", QCoreApplication::applicationFilePath().toStdWString());
    QMenu* menu = tray.contextMenu();
    QAction* autostartAction = new QAction(QStringLiteral("Start with Windows"), menu);
    autostartAction->setCheckable(true);
    autostartAction->setChecked(autostart.isEnabled());
    QObject::connect(autostartAction, &QAction::toggled, [&autostart](bool enabled) {
        autostart.setEnabled(enabled);
    });
    menu->insertAction(menu->actions().isEmpty() ? nullptr : menu->actions().first(), autostartAction);

    tray.start();

    return app.exec();
}

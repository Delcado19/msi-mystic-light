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
#include <QDir>
#include <QElapsedTimer>
#include <QMenu>
#include <QStandardPaths>
#include <QString>
#include <QTimer>

#include <cstdint>
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

// Pushes one channel's current ChannelSettings into both its driver
// (setChannelMode -- needed so e.g. RoccatVulcanDriver sends its one-time
// EnableDirect report, and so MsiMotherboardDriver's internal channelMode_
// state matches what Canvas is about to push/send) and Canvas itself
// (setAssignment). Shared by the initial apply below and the
// channelAssignmentChanged handler, so a row edit in the Studio window and
// the app's own startup state go through exactly the same path.
void applyChannelSettingsToCanvas(Canvas& canvas, const std::map<std::string, IDriver*>& driversByChannel, const ChannelSettings& cs)
{
    auto it = driversByChannel.find(cs.channelName);
    if (it != driversByChannel.end() && it->second) {
        it->second->setChannelMode(cs.channelName, cs.mode);
    }
    canvas.setAssignment(cs.channelName, buildAssignment(cs));
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
    if (!koneHid.open(0x1E7D, 0x2E27, 0, 0x0B)) {
        koneHid.open(0x1E7D, 0x2E2C, 0, 0x0B);
    }

    HidApiDevice basiliskHid;
    // interfaceNumber=3 per RazerControllerDetect.cpp:361
    // (REGISTER_HID_DETECTOR_IPU("Razer Basilisk V3", ..., 0x03, 0x0C, 0x01):
    // interface_number=3, usage_page=0x0C, usage=0x01). IHidDevice::open()
    // only filters on interface_number, not usage_page/usage, but a
    // read-only Get-PnpDevice enumeration on this machine (where this mouse
    // is actually connected) confirmed interface 3 is a single, unambiguous
    // collection for this device -- no other collection shares it -- so the
    // interface-only filter is sufficient here.
    basiliskHid.open(0x1532, 0x0099, 3);

    HidApiDevice apex3Hid;
    // interfaceNumber=3 per SteelSeriesControllerDetect.cpp:543
    // (REGISTER_HID_DETECTOR_I("SteelSeries Apex 3", ..., 3): this detector
    // variant filters on interface_number only, no usage_page, matching
    // exactly what IHidDevice::open() supports. Confirmed via a read-only
    // Get-PnpDevice enumeration on this machine (where this keyboard is
    // actually connected): interface 3 is a single, unambiguous collection.
    apex3Hid.open(0x1038, 0x161A, 3);

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
    for (uint16_t pid : kVulcanPids) {
        if (vulcanControlHid.open(kVulcanVid, pid, kVulcanCtrlIface, kVulcanCtrlUsagePage)) {
            vulcanLedHid.open(kVulcanVid, pid, kVulcanLedIface, kVulcanLedUsagePage);
            break;
        }
    }

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
    // (Task 15), which pushes a "Default" profile if settings.profiles was
    // empty and fills in a default (Native/Steady/black, per
    // ChannelSettings'/NativeModeSettings' own default member values) entry
    // for any channel missing one -- exactly the "fall back to
    // Native/Steady/black defaults for every channel" tolerance this task's
    // brief asks main.cpp to implement. Reusing that existing, already-
    // tested fallback instead of duplicating an equivalent default-filling
    // loop here means settings.activeProfile() is guaranteed non-null and
    // complete for every line below.
    LightingStudioWindow studio(settings, channelNames);

    for (const ChannelSettings& cs : settings.activeProfile()->channelSettings) {
        applyChannelSettingsToCanvas(canvas, driversByChannel, cs);
    }
    applyGlobalEffectToCanvas(canvas, settings.activeProfile()->globalEffect);

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

    // Step 4/5: Studio window <-> Canvas/tray wiring. Every mutation the
    // Studio window makes is written into `settings` before it emits, so
    // each handler just re-reads settings.activeProfile() rather than
    // needing the new value passed through the signal.
    QObject::connect(&studio, &LightingStudioWindow::channelAssignmentChanged,
        [&settings, &canvas, &driversByChannel](const std::string& channelName) {
            Profile* profile = settings.activeProfile();
            if (!profile) return; // defensive; never null once `studio` exists
            for (const ChannelSettings& cs : profile->channelSettings) {
                if (cs.channelName == channelName) {
                    applyChannelSettingsToCanvas(canvas, driversByChannel, cs);
                    break;
                }
            }
        });
    QObject::connect(&studio, &LightingStudioWindow::globalEffectChanged, [&settings, &canvas]() {
        Profile* profile = settings.activeProfile();
        if (!profile) return; // defensive; never null once `studio` exists
        applyGlobalEffectToCanvas(canvas, profile->globalEffect);
    });
    // Updates the tray's profile submenu/checkmark whenever the active
    // profile changes from inside the Studio window (profile combo box).
    QObject::connect(&studio, &LightingStudioWindow::activeProfileChanged, [&tray, &settings](const std::string&) {
        refreshTrayProfiles(tray, settings);
    });

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

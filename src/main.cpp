// Application entry point: wires together the HID transport, MSI driver,
// settings persistence, render loop, tray UI, and Windows autostart into
// the running app. This is Task 12 of the v1 skeleton plan (the final
// task) -- see docs/superpowers/plans/2026-10-18-v1-skeleton.md ("Task 12")
// for the 9 responsibilities implemented in order below. No new public
// interface is produced; this file is consumed by nothing else.
#include <QAction>
#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QMenu>
#include <QStandardPaths>
#include <QString>
#include <QTimer>

#include <map>
#include <memory>
#include <string>
#include <utility>

#include "canvas/Canvas.h"
#include "device/Channel.h"
#include "device/MsiMotherboardDriver.h"
#include "effects/RainbowWaveEffect.h"
#include "effects/StaticColorEffect.h"
#include "hid/HidApiDevice.h"
#include "platform/win/AutostartWin.h"
#include "settings/Settings.h"
#include "ui/TrayApp.h"

namespace {

// Builds the Canvas-ready assignment for one persisted ChannelSettings
// entry, per the plan's Step 5 mapping: Native mode carries its
// NativeModeSettings verbatim; Canvas mode resolves effectName to a
// concrete IEffect ("RainbowWave"/"StaticColor"). An unrecognized
// effectName (e.g. from a hand-edited settings.json) leaves effect null;
// Canvas::tick already tolerates a null effect by skipping that channel
// for the frame rather than crashing.
ChannelAssignment buildAssignment(const ChannelSettings& cs)
{
    ChannelAssignment assignment;
    assignment.mode = cs.mode;
    if (cs.mode == ChannelMode::Native) {
        assignment.nativeSettings = cs.nativeSettings;
    } else if (cs.effectName == "RainbowWave") {
        assignment.effect = std::make_shared<RainbowWaveEffect>();
    } else if (cs.effectName == "StaticColor") {
        assignment.effect = std::make_shared<StaticColorEffect>(cs.staticColor);
    }
    return assignment;
}

} // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv); // Step 1

    // Step 2: attempt to open the real board, but proceed regardless of
    // success -- the tray must still come up with the board unplugged or
    // absent (Review Focus, carried from Task 6: board not connected must
    // never crash/abort startup). The return value is intentionally unused.
    HidApiDevice hid;
    hid.open(0x0DB0, 0x0076);

    // Step 3: driver + fixed channel topology for this board.
    MsiMotherboardDriver driver(hid);
    std::map<std::string, Channel> channels;
    for (Channel& channel : MsiMotherboardDriver::createDefaultChannels()) {
        std::string name = channel.name();
        channels.emplace(std::move(name), std::move(channel));
    }

    // Step 4: load persisted settings, falling back to hardcoded defaults
    // (every channel Native/Steady/black) on a missing/corrupt file, per
    // Settings::loadFromFile's documented contract (Task 9 Review Focus:
    // never crash on a missing/bad settings file). The defaults are built
    // from ChannelSettings' and NativeModeSettings' own default member
    // values, which already are Native/Steady/black -- no separate literal
    // list to keep in sync with the channel topology above.
    const QString appDataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(appDataDir); // first run: AppDataLocation may not exist yet
    const QString settingsPath = appDataDir + QStringLiteral("/settings.json");

    Settings settings;
    if (!settings.loadFromFile(settingsPath.toStdString())) {
        settings.channelSettings.clear();
        for (const auto& [name, channel] : channels) {
            (void)channel;
            ChannelSettings defaults;
            defaults.channelName = name;
            settings.channelSettings.push_back(std::move(defaults));
        }
    }

    // Step 5: canvas + per-channel assignments from the loaded/default settings.
    Canvas canvas(driver, channels);
    for (const auto& cs : settings.channelSettings) {
        canvas.setAssignment(cs.channelName, buildAssignment(cs));
    }

    // Step 6: ~60Hz render loop, elapsed time measured from launch.
    QElapsedTimer clock;
    clock.start();
    QTimer renderTimer;
    QObject::connect(&renderTimer, &QTimer::timeout, [&canvas, &clock]() {
        canvas.tick(clock.elapsed());
    });
    renderTimer.start(16);

    // Step 7: tray shell. Quit both persists settings (first, so the save
    // happens before teardown) and exits the app.
    TrayApp tray;
    QObject::connect(&tray, &TrayApp::quitRequested, [&settings, &settingsPath]() {
        settings.saveToFile(settingsPath.toStdString());
    });
    QObject::connect(&tray, &TrayApp::quitRequested, &app, &QApplication::quit);

    // Step 8: autostart toggle, keyed to the real installed exe path. The
    // plan leaves the menu item's exact wording/placement as an
    // implementation detail; placed as a checkable "Start with Windows"
    // item above the existing Quit action, initialized from the current
    // registry state so the checkbox reflects reality on launch.
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

    return app.exec(); // Step 9
}

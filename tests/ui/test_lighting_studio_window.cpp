// Unit test for LightingStudioWindow (Task 15): canvas + side panel (per-
// channel mode/source/effect rows, profile selector) over a Settings
// instance. Same off-screen-widget/QTest pattern as
// test_lighting_canvas_widget.cpp (Task 14) -- construct, show(), drive
// combos/mouse events programmatically, no interactive desktop required.
#include <QtTest/QtTest>

#include <QComboBox>
#include <QPushButton>

#include <algorithm>
#include <map>
#include <string>
#include <vector>

#include "settings/Settings.h"
#include "ui/LightingCanvasWidget.h"
#include "ui/LightingStudioWindow.h"

namespace {

const std::vector<std::string> kChannelNames = {"CPU", "GPU", "JARGB1"};

// One profile named "Default" with all three channels in Canvas/
// Independent/StaticColor mode, each at a distinct, well-separated
// placement so a mouse press/drag at a channel's own placement
// unambiguously hits that channel's box and no other.
Settings makeSettingsWithOneProfile()
{
    Settings settings;
    Profile profile;
    profile.name = "Default";
    const CanvasPlacement placements[] = {{10.0, 10.0}, {150.0, 10.0}, {290.0, 10.0}};
    for (size_t i = 0; i < kChannelNames.size(); ++i) {
        ChannelSettings cs;
        cs.channelName = kChannelNames[i];
        cs.mode = ChannelMode::Canvas;
        cs.source = CanvasSource::Independent;
        cs.effectName = "StaticColor";
        cs.placement = placements[i];
        profile.channelSettings.push_back(cs);
    }
    settings.profiles = {profile};
    settings.activeProfileName = "Default";
    return settings;
}

// Drags the box at `placement` by a few pixels and returns the channel name
// LightingCanvasWidget::channelPlacementChanged reports for that drag --
// the only way (per the brief) to confirm a box is really registered at a
// given spot without a new getter on Task 14's already-reviewed widget.
std::string dragBoxAt(LightingCanvasWidget* canvas, const CanvasPlacement& placement)
{
    const QPoint pressPos(static_cast<int>(placement.x) + LightingCanvasWidget::kBoxWidth / 2,
                           static_cast<int>(placement.y) + LightingCanvasWidget::kBoxHeight / 2);
    const QPoint movePos = pressPos + QPoint(6, 6);

    QSignalSpy spy(canvas, &LightingCanvasWidget::channelPlacementChanged);
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, pressPos);
    QTest::mouseMove(canvas, movePos);
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, movePos);

    if (spy.count() < 1) return {};
    return spy.last().at(0).value<std::string>();
}

} // namespace

class LightingStudioWindowTest : public QObject
{
    Q_OBJECT

private slots:
    void testConstruction_populatesCanvasForEveryChannelName();
    void testConstruction_withEmptyProfiles_doesNotCrashAndShowsDefaults();
    void testChangingARowsModeInUi_emitsChannelAssignmentChanged();
    void testSwitchingActiveProfile_emitsActiveProfileChangedAndRepopulatesCanvas();
    void testRefreshFromSettings_afterExternalProfileSwitch_repopulatesRowsAndCanvas();

    // Task 23, Part 1: mode-availability enforcement.
    void testModeCombo_onlySupportedModesAreOffered();
    void testFreshProfile_canvasOnlyChannelDefaultsToVisibleCanvasStaticColor();
    void testPersistedUnsupportedMode_nativeOnlyChannelClampedFromCanvasToNative();
    void testPersistedUnsupportedMode_canvasOnlyChannelClampedFromNativeFillsStaticColorDefault();

    // Task 23, Part 2: color editing (everything around the modal
    // QColorDialog -- the dialog itself cannot be driven from an automated
    // test, see this window's public setChannelColor/setChannelNativeColor/
    // setGlobalEffectColor).
    void testSetChannelColor_updatesSettingsAndEmitsChannelAssignmentChanged();
    void testColorButton_enabledOnlyWhenEffectUsesColor();
    void testGlobalColorButton_enabledOnlyWhenEffectUsesColor();
    void testSetGlobalEffectColor_updatesSettingsAndEmitsGlobalEffectChanged();
    void testSetChannelNativeColor_updatesSettingsAndEmitsChannelAssignmentChanged();
    void testNativeModeCombo_changingItUpdatesSettingsAndEmitsSignal();
    void testNativeModeCombo_excludesAdvanced();

    // Task 23, Part 3: profile create/rename/delete.
    void testCreateProfile_addsAndActivatesNewProfileWithChannelDefaults();
    void testCreateProfile_rejectsDuplicateNameAsNoOp();
    void testRenameActiveProfile_updatesNameAndEmitsActiveProfileChanged();
    void testDeleteActiveProfile_removesItAndActivatesNeighbor();
    void testDeleteProfileButton_disabledWhenOnlyOneProfileLeft();
};

void LightingStudioWindowTest::testConstruction_populatesCanvasForEveryChannelName()
{
    Settings settings = makeSettingsWithOneProfile();
    LightingStudioWindow window(settings, kChannelNames);
    window.resize(900, 600);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    // Attached right after construction, with no interaction yet -- proves
    // construction/repopulation never fires the window's own signals on
    // their own (only real row/canvas interaction should).
    QSignalSpy assignmentSpy(&window, &LightingStudioWindow::channelAssignmentChanged);
    QSignalSpy profileSpy(&window, &LightingStudioWindow::activeProfileChanged);
    QSignalSpy globalSpy(&window, &LightingStudioWindow::globalEffectChanged);
    QCOMPARE(assignmentSpy.count(), 0);
    QCOMPARE(profileSpy.count(), 0);
    QCOMPARE(globalSpy.count(), 0);

    // Real proof that canvas() has a box registered for every channel
    // name, at the exact placement Settings supplied (also exercises the
    // task's 1:1 CanvasPlacement<->widget-pixel coordinate ruling).
    for (const auto& cs : settings.activeProfile()->channelSettings) {
        QCOMPARE(dragBoxAt(window.canvas(), cs.placement), cs.channelName);
    }
}

void LightingStudioWindowTest::testConstruction_withEmptyProfiles_doesNotCrashAndShowsDefaults()
{
    Settings settings; // profiles empty: fresh install / corrupt-file fallback case
    LightingStudioWindow window(settings, kChannelNames);

    QCOMPARE(settings.profiles.size(), static_cast<size_t>(1));
    QCOMPARE(settings.activeProfileName, std::string("Default"));
    QVERIFY(window.canvas() != nullptr);
    for (const std::string& name : kChannelNames) {
        QVERIFY(window.findChild<QComboBox*>(QString::fromStdString("mode:" + name)) != nullptr);
    }
}

void LightingStudioWindowTest::testChangingARowsModeInUi_emitsChannelAssignmentChanged()
{
    Settings settings = makeSettingsWithOneProfile();
    LightingStudioWindow window(settings, kChannelNames);

    auto* modeCombo = window.findChild<QComboBox*>(QStringLiteral("mode:GPU"));
    QVERIFY(modeCombo != nullptr);
    modeCombo->setFocus();
    QCOMPARE(modeCombo->currentIndex(), 1); // Canvas, per makeSettingsWithOneProfile

    QSignalSpy spy(&window, &LightingStudioWindow::channelAssignmentChanged);
    QTest::keyClick(modeCombo, Qt::Key_Up); // Canvas (1) -> Native (0), driven via QTest per the brief

    QCOMPARE(modeCombo->currentIndex(), 0);
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.last().at(0).value<std::string>(), std::string("GPU"));

    const auto& channels = settings.activeProfile()->channelSettings;
    const auto it = std::find_if(channels.begin(), channels.end(), [](const ChannelSettings& cs) { return cs.channelName == "GPU"; });
    QVERIFY(it != channels.end());
    QCOMPARE(it->mode, ChannelMode::Native);
}

void LightingStudioWindowTest::testSwitchingActiveProfile_emitsActiveProfileChangedAndRepopulatesCanvas()
{
    Settings settings = makeSettingsWithOneProfile();

    // A second profile with GPU moved to a different placement and put
    // into Native mode -- distinct enough from "Default" that switching
    // to it is observable both in the row widgets and on the canvas.
    Profile nightProfile = *settings.activeProfile();
    nightProfile.name = "Night";
    for (auto& cs : nightProfile.channelSettings) {
        if (cs.channelName == "GPU") {
            cs.mode = ChannelMode::Native;
            cs.placement = CanvasPlacement{300.0, 200.0};
        }
    }
    settings.profiles.push_back(nightProfile);

    LightingStudioWindow window(settings, kChannelNames);
    window.resize(900, 600);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    auto* profileCombo = window.findChild<QComboBox*>(QStringLiteral("profileCombo"));
    QVERIFY(profileCombo != nullptr);
    const int nightIdx = profileCombo->findText(QStringLiteral("Night"));
    QVERIFY(nightIdx >= 0);

    QSignalSpy profileSpy(&window, &LightingStudioWindow::activeProfileChanged);
    QSignalSpy globalSpy(&window, &LightingStudioWindow::globalEffectChanged);
    QSignalSpy assignmentSpy(&window, &LightingStudioWindow::channelAssignmentChanged);
    profileCombo->setCurrentIndex(nightIdx);

    // Per the header's documented contract: one channelAssignmentChanged
    // per channel (exact count, not just >=1 -- this is what actually
    // proves the row QSignalBlockers in repopulateFromActiveProfile work;
    // without them, re-setting each row combo's index would add extra,
    // spurious emissions on top of these intentional ones), then
    // globalEffectChanged, then activeProfileChanged.
    QCOMPARE(assignmentSpy.count(), static_cast<int>(kChannelNames.size()));
    QCOMPARE(globalSpy.count(), 1);
    QCOMPARE(profileSpy.count(), 1);
    QCOMPARE(profileSpy.last().at(0).value<std::string>(), std::string("Night"));
    QCOMPARE(settings.activeProfileName, std::string("Night"));

    // Repopulation reached the row widgets...
    auto* gpuModeCombo = window.findChild<QComboBox*>(QStringLiteral("mode:GPU"));
    QVERIFY(gpuModeCombo != nullptr);
    QCOMPARE(gpuModeCombo->currentIndex(), 0); // Native, per nightProfile

    // ...and the canvas: GPU's box moved to Night's placement.
    QCOMPARE(dragBoxAt(window.canvas(), CanvasPlacement{300.0, 200.0}), std::string("GPU"));
}

// Same two-profile setup as the in-UI switch test above, but the switch is
// triggered the way Task 18's tray profile-switch wiring will: by mutating
// settings.activeProfileName directly (simulating an external caller that
// holds the same Settings& reference) and calling the new public
// refreshFromSettings(), not by touching profileCombo_ at all.
void LightingStudioWindowTest::testRefreshFromSettings_afterExternalProfileSwitch_repopulatesRowsAndCanvas()
{
    Settings settings = makeSettingsWithOneProfile();

    Profile nightProfile = *settings.activeProfile();
    nightProfile.name = "Night";
    for (auto& cs : nightProfile.channelSettings) {
        if (cs.channelName == "GPU") {
            cs.mode = ChannelMode::Native;
            cs.placement = CanvasPlacement{300.0, 200.0};
        }
    }
    settings.profiles.push_back(nightProfile);

    LightingStudioWindow window(settings, kChannelNames);
    window.resize(900, 600);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));

    QSignalSpy profileSpy(&window, &LightingStudioWindow::activeProfileChanged);
    QSignalSpy globalSpy(&window, &LightingStudioWindow::globalEffectChanged);
    QSignalSpy assignmentSpy(&window, &LightingStudioWindow::channelAssignmentChanged);

    // External mutation: nothing in this window's UI touched yet.
    settings.activeProfileName = "Night";
    window.refreshFromSettings();

    QCOMPARE(assignmentSpy.count(), static_cast<int>(kChannelNames.size()));
    QCOMPARE(globalSpy.count(), 1);
    QCOMPARE(profileSpy.count(), 1);
    QCOMPARE(profileSpy.last().at(0).value<std::string>(), std::string("Night"));

    // Row widgets and the profile selector itself caught up...
    auto* gpuModeCombo = window.findChild<QComboBox*>(QStringLiteral("mode:GPU"));
    QVERIFY(gpuModeCombo != nullptr);
    QCOMPARE(gpuModeCombo->currentIndex(), 0); // Native, per nightProfile

    auto* profileCombo = window.findChild<QComboBox*>(QStringLiteral("profileCombo"));
    QVERIFY(profileCombo != nullptr);
    QCOMPARE(profileCombo->currentText(), QStringLiteral("Night"));

    // ...and the canvas: GPU's box moved to Night's placement.
    QCOMPARE(dragBoxAt(window.canvas(), CanvasPlacement{300.0, 200.0}), std::string("GPU"));
}

// Part 1: a channel's mode combo must only ever offer a mode its driver
// actually supports (via the constructor's availableModesByChannel map),
// never a hardcoded Native/Canvas pair -- a channel with no map entry (here,
// JARGB1) still gets both, same as every pre-Task-23 test/call site.
void LightingStudioWindowTest::testModeCombo_onlySupportedModesAreOffered()
{
    Settings settings; // fresh/empty: avoids entangling this with clamp-on-load behavior
    const std::map<std::string, std::vector<ChannelMode>> availableModes = {
        {"CPU", {ChannelMode::Canvas}}, // like a Canvas-only peripheral
        {"GPU", {ChannelMode::Native}}, // like JRGB1
    };
    LightingStudioWindow window(settings, kChannelNames, availableModes);

    auto* cpuMode = window.findChild<QComboBox*>(QStringLiteral("mode:CPU"));
    QVERIFY(cpuMode != nullptr);
    QCOMPARE(cpuMode->count(), 1);
    QCOMPARE(cpuMode->itemText(0), QStringLiteral("Canvas"));

    auto* gpuMode = window.findChild<QComboBox*>(QStringLiteral("mode:GPU"));
    QVERIFY(gpuMode != nullptr);
    QCOMPARE(gpuMode->count(), 1);
    QCOMPARE(gpuMode->itemText(0), QStringLiteral("Native"));

    auto* jargbMode = window.findChild<QComboBox*>(QStringLiteral("mode:JARGB1"));
    QVERIFY(jargbMode != nullptr);
    QCOMPARE(jargbMode->count(), 2); // no map entry -- both offered
}

// A freshly-created entry for a Canvas-only channel must default to Canvas/
// StaticColor/a non-black color, not ChannelSettings' own Native/black
// default -- Native is a confirmed permanent no-op for peripheral hardware,
// so the plain default would leave the device dark until manually fixed.
void LightingStudioWindowTest::testFreshProfile_canvasOnlyChannelDefaultsToVisibleCanvasStaticColor()
{
    Settings settings; // empty profiles -- exercises ensureProfileAndEntries()'s fresh-entry path
    const std::map<std::string, std::vector<ChannelMode>> availableModes = {{"CPU", {ChannelMode::Canvas}}};
    LightingStudioWindow window(settings, kChannelNames, availableModes);

    const auto& channels = settings.activeProfile()->channelSettings;
    const auto it = std::find_if(channels.begin(), channels.end(), [](const ChannelSettings& cs) { return cs.channelName == "CPU"; });
    QVERIFY(it != channels.end());
    QCOMPARE(it->mode, ChannelMode::Canvas);
    QCOMPARE(it->effectName, std::string("StaticColor"));
    QVERIFY(it->staticColor.r != 0 || it->staticColor.g != 0 || it->staticColor.b != 0); // not black
}

// A persisted entry naming a mode this channel no longer supports (e.g. a
// hand-edited settings.json predating Task 23) must be clamped on
// construction, not left mismatched with the combo or silently a no-op
// forever.
void LightingStudioWindowTest::testPersistedUnsupportedMode_nativeOnlyChannelClampedFromCanvasToNative()
{
    Settings settings = makeSettingsWithOneProfile(); // GPU persisted as Canvas/Independent/StaticColor
    const std::map<std::string, std::vector<ChannelMode>> availableModes = {{"GPU", {ChannelMode::Native}}};
    LightingStudioWindow window(settings, kChannelNames, availableModes);

    const auto& channels = settings.activeProfile()->channelSettings;
    const auto it = std::find_if(channels.begin(), channels.end(), [](const ChannelSettings& cs) { return cs.channelName == "GPU"; });
    QVERIFY(it != channels.end());
    QCOMPARE(it->mode, ChannelMode::Native);

    auto* gpuMode = window.findChild<QComboBox*>(QStringLiteral("mode:GPU"));
    QVERIFY(gpuMode != nullptr);
    QCOMPARE(gpuMode->currentText(), QStringLiteral("Native"));
}

// The inverse clamp (Native -> Canvas) must not leave effectName empty --
// that's the exact "stays dark forever" bug class the brief calls out for
// per-channel defaults; the clamp must fill the same StaticColor/color
// default a fresh peripheral entry gets.
void LightingStudioWindowTest::testPersistedUnsupportedMode_canvasOnlyChannelClampedFromNativeFillsStaticColorDefault()
{
    Settings settings;
    Profile profile;
    profile.name = "Default";
    ChannelSettings cs;
    cs.channelName = "CPU";
    cs.mode = ChannelMode::Native; // persisted; effectName left empty per the "empty if Native" contract
    profile.channelSettings.push_back(cs);
    settings.profiles = {profile};
    settings.activeProfileName = "Default";

    const std::map<std::string, std::vector<ChannelMode>> availableModes = {{"CPU", {ChannelMode::Canvas}}};
    LightingStudioWindow window(settings, kChannelNames, availableModes);

    const auto& channels = settings.activeProfile()->channelSettings;
    const auto it = std::find_if(channels.begin(), channels.end(), [](const ChannelSettings& c) { return c.channelName == "CPU"; });
    QVERIFY(it != channels.end());
    QCOMPARE(it->mode, ChannelMode::Canvas);
    QCOMPARE(it->effectName, std::string("StaticColor"));
    QVERIFY(it->staticColor.r != 0 || it->staticColor.g != 0 || it->staticColor.b != 0);
}

void LightingStudioWindowTest::testSetChannelColor_updatesSettingsAndEmitsChannelAssignmentChanged()
{
    Settings settings = makeSettingsWithOneProfile();
    LightingStudioWindow window(settings, kChannelNames);

    QSignalSpy spy(&window, &LightingStudioWindow::channelAssignmentChanged);
    window.setChannelColor("GPU", MsiColor{10, 20, 30});

    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.last().at(0).value<std::string>(), std::string("GPU"));
    const auto& channels = settings.activeProfile()->channelSettings;
    const auto it = std::find_if(channels.begin(), channels.end(), [](const ChannelSettings& c) { return c.channelName == "GPU"; });
    QVERIFY(it != channels.end());
    QCOMPARE(it->staticColor.r, static_cast<uint8_t>(10));
    QCOMPARE(it->staticColor.g, static_cast<uint8_t>(20));
    QCOMPARE(it->staticColor.b, static_cast<uint8_t>(30));
}

// effectUsesColor() gates the row's color swatch: StaticColor uses it,
// RainbowWave doesn't (mirrors buildEffect()'s own constructor arguments).
void LightingStudioWindowTest::testColorButton_enabledOnlyWhenEffectUsesColor()
{
    Settings settings = makeSettingsWithOneProfile(); // GPU: Canvas/Independent/StaticColor
    LightingStudioWindow window(settings, kChannelNames);

    auto* colorButton = window.findChild<QPushButton*>(QStringLiteral("color:GPU"));
    QVERIFY(colorButton != nullptr);
    QVERIFY(colorButton->isEnabled());

    auto* effectCombo = window.findChild<QComboBox*>(QStringLiteral("effect:GPU"));
    QVERIFY(effectCombo != nullptr);
    const int rainbowIdx = effectCombo->findText(QStringLiteral("RainbowWave"));
    QVERIFY(rainbowIdx >= 0);
    effectCombo->setCurrentIndex(rainbowIdx);
    QVERIFY(!colorButton->isEnabled());
}

// Regression test for a real bug caught in review: globalEffectCombo_ is
// set via findText(profile->globalEffect.effectName), which falls back to
// displaying index 0 ("StaticColor") whenever the STORED effectName is
// empty or unrecognized -- true for every fresh profile, since
// GlobalEffectSettings{} default-constructs effectName to "". Gating the
// swatch's enabled state on the combo's displayed text (rather than the
// actual stored globalEffect.effectName) would enable it for a profile
// whose stored effect is still "" -- clicking it would call
// setGlobalEffectColor(), but applyGlobalEffectToCanvas(buildEffect(""))
// still resolves to nullptr, so nothing visible would change. This test
// pins the swatch disabled on a truly-empty stored name, and -- since
// selecting index 0 ("StaticColor") when the combo ALREADY merely displays
// index 0 is a Qt no-op that fires no signal and would mask this bug --
// deliberately selects RainbowWave first so the StaticColor selection after
// it is a genuine index change that actually writes Settings and emits.
void LightingStudioWindowTest::testGlobalColorButton_enabledOnlyWhenEffectUsesColor()
{
    Settings settings = makeSettingsWithOneProfile(); // globalEffect.effectName == "" (never set by this helper)
    LightingStudioWindow window(settings, kChannelNames);

    auto* globalEffectCombo = window.findChild<QComboBox*>(QStringLiteral("globalEffectCombo"));
    auto* globalColorButton = window.findChild<QPushButton*>(QStringLiteral("globalColorButton"));
    QVERIFY(globalEffectCombo != nullptr);
    QVERIFY(globalColorButton != nullptr);

    QCOMPARE(settings.activeProfile()->globalEffect.effectName, std::string());
    QVERIFY(!globalColorButton->isEnabled()); // stored name is "", regardless of what the combo displays

    const int rainbowIdx = globalEffectCombo->findText(QStringLiteral("RainbowWave"));
    QVERIFY(rainbowIdx >= 0);
    globalEffectCombo->setCurrentIndex(rainbowIdx); // a genuine change: fires currentIndexChanged, writes Settings
    QVERIFY(!globalColorButton->isEnabled());

    const int staticIdx = globalEffectCombo->findText(QStringLiteral("StaticColor"));
    QVERIFY(staticIdx >= 0);
    globalEffectCombo->setCurrentIndex(staticIdx); // also a genuine change now (from RainbowWave)
    QVERIFY(globalColorButton->isEnabled());
    QCOMPARE(settings.activeProfile()->globalEffect.effectName, std::string("StaticColor"));
}

void LightingStudioWindowTest::testSetGlobalEffectColor_updatesSettingsAndEmitsGlobalEffectChanged()
{
    Settings settings = makeSettingsWithOneProfile();
    LightingStudioWindow window(settings, kChannelNames);

    QSignalSpy spy(&window, &LightingStudioWindow::globalEffectChanged);
    window.setGlobalEffectColor(MsiColor{1, 2, 3});

    QCOMPARE(spy.count(), 1);
    QCOMPARE(settings.activeProfile()->globalEffect.color.r, static_cast<uint8_t>(1));
    QCOMPARE(settings.activeProfile()->globalEffect.color.g, static_cast<uint8_t>(2));
    QCOMPARE(settings.activeProfile()->globalEffect.color.b, static_cast<uint8_t>(3));
}

void LightingStudioWindowTest::testSetChannelNativeColor_updatesSettingsAndEmitsChannelAssignmentChanged()
{
    Settings settings = makeSettingsWithOneProfile();
    LightingStudioWindow window(settings, kChannelNames);

    QSignalSpy spy(&window, &LightingStudioWindow::channelAssignmentChanged);
    window.setChannelNativeColor("GPU", MsiColor{40, 50, 60});

    QCOMPARE(spy.count(), 1);
    const auto& channels = settings.activeProfile()->channelSettings;
    const auto it = std::find_if(channels.begin(), channels.end(), [](const ChannelSettings& c) { return c.channelName == "GPU"; });
    QVERIFY(it != channels.end());
    QCOMPARE(it->nativeSettings.color1.r, static_cast<uint8_t>(40));
    QCOMPARE(it->nativeSettings.color1.g, static_cast<uint8_t>(50));
    QCOMPARE(it->nativeSettings.color1.b, static_cast<uint8_t>(60));
}

void LightingStudioWindowTest::testNativeModeCombo_changingItUpdatesSettingsAndEmitsSignal()
{
    Settings settings = makeSettingsWithOneProfile();
    LightingStudioWindow window(settings, kChannelNames);

    auto* nativeModeCombo = window.findChild<QComboBox*>(QStringLiteral("nativeMode:GPU"));
    QVERIFY(nativeModeCombo != nullptr);
    const int waveIdx = nativeModeCombo->findData(static_cast<int>(MsiLightingMode::Wave));
    QVERIFY(waveIdx >= 0);

    QSignalSpy spy(&window, &LightingStudioWindow::channelAssignmentChanged);
    nativeModeCombo->setCurrentIndex(waveIdx);

    QCOMPARE(spy.count(), 1);
    const auto& channels = settings.activeProfile()->channelSettings;
    const auto it = std::find_if(channels.begin(), channels.end(), [](const ChannelSettings& c) { return c.channelName == "GPU"; });
    QVERIFY(it != channels.end());
    QCOMPARE(it->nativeSettings.lightingMode, MsiLightingMode::Wave);
}

// Advanced is the Canvas per-LED arming mode (MsiMotherboardDriver::
// setChannelMode), not a real user-chosen Native-mode value -- must not
// appear in this combo at all.
void LightingStudioWindowTest::testNativeModeCombo_excludesAdvanced()
{
    Settings settings = makeSettingsWithOneProfile();
    LightingStudioWindow window(settings, kChannelNames);

    auto* nativeModeCombo = window.findChild<QComboBox*>(QStringLiteral("nativeMode:GPU"));
    QVERIFY(nativeModeCombo != nullptr);
    QCOMPARE(nativeModeCombo->findData(static_cast<int>(MsiLightingMode::Advanced)), -1);
}

void LightingStudioWindowTest::testCreateProfile_addsAndActivatesNewProfileWithChannelDefaults()
{
    Settings settings = makeSettingsWithOneProfile(); // one profile, "Default"
    LightingStudioWindow window(settings, kChannelNames);

    QSignalSpy profileSpy(&window, &LightingStudioWindow::activeProfileChanged);
    window.createProfile("Night");

    QCOMPARE(static_cast<int>(settings.profiles.size()), 2);
    QCOMPARE(settings.activeProfileName, std::string("Night"));
    QCOMPARE(profileSpy.count(), 1);
    QCOMPARE(profileSpy.last().at(0).value<std::string>(), std::string("Night"));
    // ensureProfileAndEntries() filled in every known channel for it.
    QCOMPARE(static_cast<int>(settings.activeProfile()->channelSettings.size()), static_cast<int>(kChannelNames.size()));

    auto* profileCombo = window.findChild<QComboBox*>(QStringLiteral("profileCombo"));
    QVERIFY(profileCombo != nullptr);
    QVERIFY(profileCombo->findText(QStringLiteral("Night")) >= 0);
    QCOMPARE(profileCombo->currentText(), QStringLiteral("Night"));
}

void LightingStudioWindowTest::testCreateProfile_rejectsDuplicateNameAsNoOp()
{
    Settings settings = makeSettingsWithOneProfile(); // profile named "Default"
    LightingStudioWindow window(settings, kChannelNames);

    QSignalSpy profileSpy(&window, &LightingStudioWindow::activeProfileChanged);
    window.createProfile("Default"); // duplicate

    QCOMPARE(static_cast<int>(settings.profiles.size()), 1);
    QCOMPARE(profileSpy.count(), 0); // no-op: nothing re-synced/re-emitted
}

void LightingStudioWindowTest::testRenameActiveProfile_updatesNameAndEmitsActiveProfileChanged()
{
    Settings settings = makeSettingsWithOneProfile();
    LightingStudioWindow window(settings, kChannelNames);

    QSignalSpy profileSpy(&window, &LightingStudioWindow::activeProfileChanged);
    window.renameActiveProfile("Gaming");

    QCOMPARE(settings.activeProfileName, std::string("Gaming"));
    QCOMPARE(profileSpy.count(), 1);
    QCOMPARE(profileSpy.last().at(0).value<std::string>(), std::string("Gaming"));

    auto* profileCombo = window.findChild<QComboBox*>(QStringLiteral("profileCombo"));
    QVERIFY(profileCombo != nullptr);
    QCOMPARE(profileCombo->currentText(), QStringLiteral("Gaming"));
}

void LightingStudioWindowTest::testDeleteActiveProfile_removesItAndActivatesNeighbor()
{
    Settings settings = makeSettingsWithOneProfile(); // "Default"
    LightingStudioWindow window(settings, kChannelNames);
    window.createProfile("Night"); // 2 profiles now, "Night" active

    QSignalSpy profileSpy(&window, &LightingStudioWindow::activeProfileChanged);
    window.deleteActiveProfile();

    QCOMPARE(static_cast<int>(settings.profiles.size()), 1);
    QCOMPARE(settings.activeProfileName, std::string("Default"));
    QCOMPARE(profileSpy.count(), 1);
}

void LightingStudioWindowTest::testDeleteProfileButton_disabledWhenOnlyOneProfileLeft()
{
    Settings settings = makeSettingsWithOneProfile();
    LightingStudioWindow window(settings, kChannelNames);

    auto* deleteButton = window.findChild<QPushButton*>(QStringLiteral("deleteProfileButton"));
    QVERIFY(deleteButton != nullptr);
    QVERIFY(!deleteButton->isEnabled());

    window.createProfile("Night");
    QVERIFY(deleteButton->isEnabled());

    window.deleteActiveProfile();
    QVERIFY(!deleteButton->isEnabled());
}

QTEST_MAIN(LightingStudioWindowTest)
#include "test_lighting_studio_window.moc"

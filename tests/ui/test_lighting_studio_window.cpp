#include <QCheckBox>
#include "ui/LayoutEditor.h"
#include "ui/HardwareLibrary.h"
#include "ui/HardwarePicker.h"
#include "ui/DeviceArtwork.h"
#include "ui/EffectEditor.h"
#include "hid/HardwareWorker.h"
#include <atomic>
// Unit test for LightingStudioWindow (Task 15): canvas + side panel (per-
// channel mode/source/effect rows, profile selector) over a Settings
// instance. Same off-screen-widget/QTest pattern as
// test_lighting_canvas_widget.cpp (Task 14) -- construct, show(), drive
// combos/mouse events programmatically, no interactive desktop required.
#include <QtTest/QtTest>

#include <QComboBox>
#include <QPushButton>
#include <QListWidget>
#include <QStackedWidget>
#include <QTabWidget>
#include "device/RoccatVulcanDriver.h"
#include "device/RoccatKoneDriver.h"
#include "device/RazerBasiliskDriver.h"
#include "device/SteelSeriesApex3Driver.h"
#include <QDir>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QTimer>
#include <QApplication>

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
        if(cs.channelName=="JARGB1"){cs.componentsConfigured=true;cs.components={Component::makeGenericPlaceholder(1)};}
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
    QTest::mousePress(canvas, Qt::LeftButton, Qt::NoModifier, canvas->sceneToWidget(pressPos));
    QTest::mouseMove(canvas, canvas->sceneToWidget(movePos));
    QTest::mouseRelease(canvas, Qt::LeftButton, Qt::NoModifier, canvas->sceneToWidget(movePos));

    if (spy.count() < 1) return {};
    return spy.last().at(0).value<std::string>();
}

} // namespace

class LightingStudioWindowTest : public QObject
{
    Q_OBJECT

private slots:
    void testHardwareRemovalGeometryAndCalibration();
    void testLayoutEditorDragShapesAndReset();
    void testCustomHardwareDefinitionDialog();
    void testChannelPickerAddsMultipleAndRemovesInline();
    void testHardwarePickerFiltersAndRejectsOverCapacity();
    void testFocusedNavigationAndArtworkResources();
    void testDurationSliderHasUsefulQuarterRangeAndExactSeconds();
    void testCustomEffectEditorDialogSavesRecipe();
    void testEditPresetColorsSurviveSaveAndReopen();
    void testColorWheelRgbAndHexStaySynchronized();
    void testGradientDraggingAndPatternDrop();
    void testSlowHardwareDoesNotBlockGui();
    void testHardwareSelectionAndCustomEffectEditing();
    void testDeviceSelection_changesInspectorWithoutChangingLighting();
    void testPreviewPause_doesNotChangeHardwareSettings();
    void testDesignPreview_export();
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
    QCOMPARE(cpuMode->itemText(0), QStringLiteral("Software lighting"));

    auto* gpuMode = window.findChild<QComboBox*>(QStringLiteral("mode:GPU"));
    QVERIFY(gpuMode != nullptr);
    QCOMPARE(gpuMode->count(), 1);
    QCOMPARE(gpuMode->itemText(0), QStringLiteral("Device effects"));
    const auto& entries = settings.activeProfile()->channelSettings;
    const auto gpu = std::find_if(entries.begin(), entries.end(), [](const ChannelSettings& cs) { return cs.channelName == "GPU"; });
    QVERIFY(gpu != entries.end());
    QCOMPARE(gpu->nativeSettings.color1.r, uint8_t(255));
    QCOMPARE(gpu->nativeSettings.color1.g, uint8_t(255));
    QCOMPARE(gpu->nativeSettings.color1.b, uint8_t(255));


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
    QCOMPARE(gpuMode->currentText(), QStringLiteral("Device effects"));
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
// pins the swatch disabled on a truly-empty stored name. Follow-up: the
// combo itself now shows NO selection (index -1) for an empty stored name,
// so picking "StaticColor" directly -- the user's obvious first action -- is
// a genuine index change that writes Settings and enables the swatch
// (previously it was a same-index no-op, forcing a detour via another effect).
void LightingStudioWindowTest::testGlobalColorButton_enabledOnlyWhenEffectUsesColor()
{
    Settings settings = makeSettingsWithOneProfile(); // globalEffect.effectName == "" (never set by this helper)
    LightingStudioWindow window(settings, kChannelNames);

    auto* globalEffectCombo = window.findChild<QComboBox*>(QStringLiteral("globalEffectCombo"));
    auto* globalColorButton = window.findChild<QPushButton*>(QStringLiteral("globalColorButton"));
    QVERIFY(globalEffectCombo != nullptr);
    QVERIFY(globalColorButton != nullptr);

    QCOMPARE(settings.activeProfile()->globalEffect.effectName, std::string());
    QCOMPARE(globalEffectCombo->currentIndex(), -1); // display matches the stored "" -- no fake "StaticColor"
    QVERIFY(!globalColorButton->isEnabled());

    const int staticIdx = globalEffectCombo->findText(QStringLiteral("StaticColor"));
    QVERIFY(staticIdx >= 0);
    globalEffectCombo->setCurrentIndex(staticIdx); // picked directly from the fresh state
    QVERIFY(globalColorButton->isEnabled());
    QCOMPARE(settings.activeProfile()->globalEffect.effectName, std::string("StaticColor"));

    const int rainbowIdx = globalEffectCombo->findText(QStringLiteral("RainbowWave"));
    QVERIFY(rainbowIdx >= 0);
    globalEffectCombo->setCurrentIndex(rainbowIdx);
    QVERIFY(!globalColorButton->isEnabled());

    // Item 3 round trip: a color picked via the swatch lands in Settings and
    // is shown again after a profile reload (refreshFromSettings).
    globalEffectCombo->setCurrentIndex(staticIdx);
    window.setGlobalEffectColor(MsiColor{9, 8, 7});
    window.refreshFromSettings();
    QCOMPARE(settings.activeProfile()->globalEffect.color.b, static_cast<uint8_t>(7));
    QCOMPARE(globalEffectCombo->currentText(), QStringLiteral("StaticColor"));
    QVERIFY(globalColorButton->isEnabled());
    QVERIFY(globalColorButton->styleSheet().contains(QStringLiteral("#090807")));
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

void LightingStudioWindowTest::testDeviceSelection_changesInspectorWithoutChangingLighting()
{
    Settings settings = makeSettingsWithOneProfile();
    LightingStudioWindow window(settings, kChannelNames);
    const auto before = settings.profiles;
    auto* list = window.findChild<QListWidget*>(QStringLiteral("deviceList"));
    auto* pages = window.findChild<QStackedWidget*>(QStringLiteral("deviceInspector"));
    QVERIFY(list && pages);
    QCOMPARE(list->count(), 3);
    QSignalSpy changed(&window, &LightingStudioWindow::channelAssignmentChanged);
    list->setCurrentRow(1);
    QCOMPARE(pages->currentIndex(), 1);
    QVERIFY(settings.profiles == before);
    QCOMPARE(changed.count(), 0);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    dragBoxAt(window.canvas(), {10, 10});
    QCOMPARE(list->currentRow(), 0);
    QCOMPARE(pages->currentIndex(), 0);
}

void LightingStudioWindowTest::testPreviewPause_doesNotChangeHardwareSettings()
{
    Settings settings = makeSettingsWithOneProfile();
    LightingStudioWindow window(settings, kChannelNames);
    const auto before = settings.profiles;
    auto* pause = window.findChild<QPushButton*>(QStringLiteral("previewToggle"));
    QVERIFY(pause);
    pause->setChecked(true);
    QVERIFY(window.canvas()->previewPaused());
    pause->setChecked(false);
    QVERIFY(!window.canvas()->previewPaused());
    QVERIFY(settings.profiles == before);
}

void LightingStudioWindowTest::testDesignPreview_export()
{
    const QString output = qEnvironmentVariable("LIGHTING_STUDIO_SCREENSHOT");
    if (output.isEmpty()) QSKIP("Set LIGHTING_STUDIO_SCREENSHOT to export a UI-only preview.");
    Settings settings;
    Profile profile;
    profile.name = "Evening studio";
    profile.globalEffect.effectName = "RadialRainbow";
    profile.globalEffect.referenceX = 550;
    profile.globalEffect.referenceY = 350;
    const std::vector<std::string> names = {"JARGB1", "JARGB2", "JARGB3", "EZConnector", "IOShield", "JRGB1", "RoccatVulcan", "RoccatKone", "RazerBasiliskV3", "SteelSeriesApex3"};
    const CanvasPlacement positions[] = {{345,190},{630,190},{630,300},{345,300},{345,405},{630,405},{320,550},{760,550},{760,615},{535,550}};
    for (size_t i = 0; i < names.size(); ++i) {
        ChannelSettings channel;
        channel.channelName = names[i];
        channel.mode = ChannelMode::Canvas;
        channel.effectName = "StaticColor";
        channel.staticColor = {113, 226, 197};
        channel.placement = positions[i];
        profile.channelSettings.push_back(channel);
    }
    settings.profiles = {profile};
    settings.activeProfileName = profile.name;
    LightingStudioWindow window(settings, names, {{"IOShield", {ChannelMode::Native}}, {"JRGB1", {ChannelMode::Native}}});
    window.setDeviceComponents("RoccatVulcan",RoccatVulcanDriver::createDefaultChannel().components());
    window.setDeviceComponents("RoccatKone",RoccatKoneDriver::createDefaultChannel().components());
    window.configureComponents("JARGB1",{Component::makeFan(6,true),Component::makeFan(6,true,1),Component::makeFan(6,true,2)});
    window.setDeviceComponents("JARGB2",{Component::makeGenericPlaceholder(120)});
    window.setDeviceComponents("RazerBasiliskV3",RazerBasiliskDriver::createDefaultChannel().components());
    window.setDeviceComponents("SteelSeriesApex3",SteelSeriesApex3Driver::createDefaultChannel().components());
    const CanvasPlacement arranged[]={{80,80,440,125},{630,190},{630,300},{345,300},{160,300,140,35},{420,300,140,35},{100,430,520,145},{760,410,80,135},{900,410,80,135},{630,580,370,85}};
    for(size_t i=0;i<names.size();++i)settings.activeProfile()->channelSettings[i].placement=arranged[i];
    window.refreshFromSettings();
    window.resize(1440, 880);
    window.findChild<QListWidget*>(QStringLiteral("deviceList"))->setCurrentRow(6);
    window.canvas()->setPreviewPaused(true);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    const auto capture=[&](const QString& path) {
        auto image=window.grab();
        if(window.canvas()->isVisible()) {
            QPainter painter(&image);
            painter.drawImage(QRect(window.canvas()->mapTo(&window,QPoint()),window.canvas()->size()),window.canvas()->grabFramebuffer());
        }
        return image.save(path);
    };
    QVERIFY(capture(output));
    QVERIFY(window.canvas()->grabFramebuffer().save(output+".canvas-framebuffer.png"));
    window.resize(1080, 700);
    QCoreApplication::processEvents();
    QVERIFY(capture(output + QStringLiteral(".compact.png")));
    window.resize(1440,880);
    auto* regions=window.findChild<QDialog*>("ledRegionsDialog");QVERIFY(regions);regions->show();
    window.findChild<QListWidget*>(QStringLiteral("deviceList"))->setCurrentRow(6);
    QCoreApplication::processEvents();QVERIFY(regions->grab().save(output+QStringLiteral(".keyboard.png")));
    window.findChild<QListWidget*>(QStringLiteral("deviceList"))->setCurrentRow(7);
    QCoreApplication::processEvents();QVERIFY(regions->grab().save(output+QStringLiteral(".mouse.png")));
    window.findChild<QListWidget*>(QStringLiteral("deviceList"))->setCurrentRow(0);
    QCoreApplication::processEvents();QVERIFY(regions->grab().save(output+QStringLiteral(".fans.png")));
    regions->close();window.findChild<QTabWidget*>("workspaceViews")->setCurrentIndex(2);
    QCoreApplication::processEvents();QVERIFY(window.grab().save(output+".hardware.png"));
    window.resize(1080,700);QCoreApplication::processEvents();QVERIFY(window.grab().save(output+".hardware-compact.png"));
    window.resize(1440,880);
    window.findChild<QTabWidget*>("workspaceViews")->setCurrentIndex(1);
    QCoreApplication::processEvents();QVERIFY(window.grab().save(output+".effects.png"));
    regions->show();
    window.findChild<QListWidget*>(QStringLiteral("deviceList"))->setCurrentRow(1);
    QCoreApplication::processEvents();QVERIFY(regions->grab().save(output+QStringLiteral(".slots.png")));

}

void LightingStudioWindowTest::testHardwareSelectionAndCustomEffectEditing()
{
    Settings settings;LightingStudioWindow window(settings,{"JARGB1"},{{"JARGB1",{ChannelMode::Canvas}}});
    window.setDeviceComponents("JARGB1",{Component::makeGenericPlaceholder(120)});
    QVERIFY(window.configureComponents("JARGB1",{Component::makeFan(6,true),Component::makeFan(6,true,1)}));
    QCOMPARE(settings.activeProfile()->channelSettings[0].components.size(),size_t(2));
    QVERIFY(window.setRegionColor("JARGB1",7,{0,255,0}));
    const auto beforeIdentify=settings.profiles;
    QSignalSpy identify(&window,&LightingStudioWindow::regionIdentificationRequested);
    QVERIFY(window.identifyRegion("JARGB1",7));QCOMPARE(identify.count(),1);
    QCOMPARE(identify.last().at(1).toInt(),7);
    window.stopRegionIdentification();QCOMPARE(identify.last().at(1).toInt(),-1);
    QVERIFY(settings.profiles==beforeIdentify);
    QVERIFY(!window.identifyRegion("JARGB1",120));

    QVERIFY(!window.setRegionColor("JARGB1",12,{255,0,0}));
    auto huge=Component::makeGenericPlaceholder(121);QVERIFY(!window.configureComponents("JARGB1",{huge}));
    QSignalSpy libraryChanges(&window,&LightingStudioWindow::customEffectsChanged);
    QVERIFY(window.createCustomEffect({"My palette",{{255,0,0},{0,0,255}},6000,720}));
    QCOMPARE(libraryChanges.count(),1);
    QCOMPARE(settings.activeProfile()->globalEffect.effectName,std::string("custom:My palette"));
    auto* combo=window.findChild<QComboBox*>(QStringLiteral("globalEffectCombo"));
    QCOMPARE(combo->currentText(),QStringLiteral("My palette"));
    QVERIFY(!window.createCustomEffect({"Invalid",{{1,2,3}},0,0}));
    QCOMPARE(settings.customEffects.size(),size_t(1));
}

void LightingStudioWindowTest::testCustomEffectEditorDialogSavesRecipe()
{
    Settings settings;LightingStudioWindow window(settings,{"JARGB1"});
    auto* button=window.findChild<QPushButton*>(QStringLiteral("createEffectButton"));QVERIFY(button);
    QTimer::singleShot(0,[&] {
        auto* dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());if(!dialog)return;
        auto* name=dialog->findChild<QLineEdit*>(QStringLiteral("effectRecipeName"));
        auto* library=dialog->findChild<QListWidget*>(QStringLiteral("patternLibrary"));
        if(!name || !library){dialog->reject();return;}
        name->setText(QStringLiteral("Dialog recipe"));library->setCurrentRow(5);
        const QString output=qEnvironmentVariable("LIGHTING_STUDIO_SCREENSHOT");
        if(!output.isEmpty())dialog->grab().save(output+QStringLiteral(".effect-editor.png"));
        dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save)->click();
    });
    button->click();
    QCOMPARE(settings.customEffects.size(),size_t(1));
    QCOMPARE(settings.customEffects[0].name,std::string("Dialog recipe"));
    QCOMPARE(settings.customEffects[0].pattern,std::string("Plasma"));
    QVERIFY(settings.customEffects[0].validStops());
    QCOMPARE(settings.activeProfile()->globalEffect.effectName,std::string("custom:Dialog recipe"));
}

void LightingStudioWindowTest::testColorWheelRgbAndHexStaySynchronized()
{
    ColorPicker picker(QColor(255,0,0));
    auto* green=picker.findChild<QSpinBox*>("colorG");QVERIFY(green);green->setValue(128);
    QCOMPARE(picker.color(),QColor(255,128,0));
    ColorWheel wheel;wheel.color=QColor(255,0,0);QColor picked;
    wheel.changed=[&](QColor c){picked=c;};QTest::mouseClick(&wheel,Qt::LeftButton,Qt::NoModifier,QPoint(125,125));QCOMPARE(picked.rgb(),QColor(Qt::white).rgb());
    auto* hex=picker.findChild<QLineEdit*>("colorHex");hex->setText("#123456");
    QMetaObject::invokeMethod(hex,"editingFinished");QCOMPARE(picker.color(),QColor("#123456"));
    hex->setText("invalid");QMetaObject::invokeMethod(hex,"editingFinished");QCOMPARE(picker.color(),QColor("#123456"));
}

void LightingStudioWindowTest::testGradientDraggingAndPatternDrop()
{
    CustomEffectDefinition d{"Test",{{255,0,0},{0,0,255}},6000,720};
    GradientStops stops(&d,nullptr);stops.resize(500,74);stops.show();
    const int x=12+int(d.stops[1]*476);
    QTest::mousePress(&stops,Qt::LeftButton,Qt::NoModifier,QPoint(x,45));
    QMouseEvent move(QEvent::MouseMove,QPointF(390,45),QPointF(390,45),Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
    QApplication::sendEvent(&stops,&move);QTest::mouseRelease(&stops,Qt::LeftButton,Qt::NoModifier,QPoint(390,45));
    QVERIFY(d.stops[1]>.75);QVERIFY(d.validStops());
    stops.addStop(.4);QCOMPARE(d.colors.size(),size_t(3));stops.removeStop();QCOMPARE(d.colors.size(),size_t(2));
    EffectPreview preview(&d,nullptr);QMimeData mime;mime.setData("application/x-lighting-pattern","Plasma");
    QDragEnterEvent enter(QPoint(50,50),Qt::CopyAction,&mime,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(&preview,&enter);QVERIFY(enter.isAccepted());
    QDropEvent drop(QPointF(50,50),Qt::CopyAction,&mime,Qt::LeftButton,Qt::NoModifier);QApplication::sendEvent(&preview,&drop);QCOMPARE(d.pattern,std::string("Plasma"));
}

void LightingStudioWindowTest::testSlowHardwareDoesNotBlockGui()
{
    HardwareWorker worker;std::atomic<bool> started{false},done{false};int heartbeat=0;
    QTimer timer;connect(&timer,&QTimer::timeout,[&]{++heartbeat;});timer.start(5);
    QElapsedTimer submission;submission.start();
    worker.post([&]{started=true;QThread::msleep(150);done=true;});
    QVERIFY(submission.elapsed()<100);
    QTRY_VERIFY(started.load());QTest::qWait(60);QVERIFY(heartbeat>=3);
    QTRY_VERIFY(done.load());worker.stop();
}

void LightingStudioWindowTest::testHardwareRemovalGeometryAndCalibration()
{
    Settings settings;LightingStudioWindow window(settings,{"JARGB1","RoccatVulcan"},{{"RoccatVulcan",{ChannelMode::Canvas}}});
    window.setDeviceComponents("JARGB1",{Component::makeGenericPlaceholder(120)});
    const auto keyboard=RoccatVulcanDriver::createDefaultChannel().components();window.setDeviceComponents("RoccatVulcan",keyboard);
    QVERIFY(window.configureComponents("JARGB1",{Component::makeFan(6,true)}));
    QVERIFY(settings.activeProfile()->channelSettings[0].placement.width>150);
    QVERIFY(window.configureComponents("JARGB1",{}));QVERIFY(settings.activeProfile()->channelSettings[0].hasCustomComponents());QVERIFY(settings.activeProfile()->channelSettings[0].components.empty());
    QCOMPARE(settings.activeProfile()->channelSettings[1].placement.width,420.);
    settings.activeProfile()->channelSettings[1].placement.width=510;window.setDeviceComponents("RoccatVulcan",keyboard);QCOMPARE(settings.activeProfile()->channelSettings[1].placement.width,510.);
    auto modified=keyboard;modified[0].ledCoordinates[0].first+=5;QVERIFY(window.editComponentGeometry("RoccatVulcan",modified));
    QSignalSpy probe(&window,&LightingStudioWindow::regionIdentificationRequested);
    QTimer::singleShot(0,[&]{auto* dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());if(!dialog)return;auto* address=dialog->findChild<QSpinBox*>("calibrationAddress");if(!address){dialog->reject();return;}address->setValue(0x16);const auto checkboxes=dialog->findChildren<QCheckBox*>();if(!checkboxes.empty())checkboxes.front()->setChecked(true);dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save)->click();});
    window.findChild<QPushButton*>("calibrateKeyboard")->click();
    const auto& saved=settings.activeProfile()->channelSettings[1].components[0];QCOMPARE(saved.ledMapping[0x0a],0x16);QCOMPARE(saved.ledMapping[0x16],0x0a);QVERIFY(saved.valid());
    QCOMPARE(probe.count(),2);QCOMPARE(probe[0][1].toInt(),0x16);QCOMPARE(probe[1][1].toInt(),-1);
    const auto& catalog=hardwareLibrary();QVERIFY(catalog.size()>=5);for(const auto& c:catalog)QVERIFY(c.valid());qInfo()<<"Available hardware definitions:"<<catalog.size();
}

void LightingStudioWindowTest::testLayoutEditorDragShapesAndReset()
{
    const auto fan=Component::makeFan(6,true);LayoutEditor editor({fan});editor.show();QVERIFY(QTest::qWaitForWindowExposed(&editor));
    auto* view=editor.findChild<QGraphicsView*>("componentLayoutView");QVERIFY(view);
    view->fitInView(view->scene()->itemsBoundingRect().adjusted(-20,-20,20,20),Qt::KeepAspectRatio);
    QGraphicsItem* dot=nullptr;for(auto* item:view->scene()->items())if(item->data(0).toString()=="LED" && item->data(1).toInt()==0){dot=item;break;}QVERIFY(dot);
    const auto from=view->mapFromScene(dot->scenePos()),to=from+QPoint(30,15);
    QTest::mousePress(view->viewport(),Qt::LeftButton,Qt::NoModifier,from);QTest::mouseMove(view->viewport(),to);QTest::mouseRelease(view->viewport(),Qt::LeftButton,Qt::NoModifier,to);
    QVERIFY(dot->pos()!=QPointF(fan.ledCoordinates[0].first,fan.ledCoordinates[0].second));
    QGraphicsItem* second=nullptr;for(auto* item:view->scene()->items())if(item->data(0).toString()=="LED" && item->data(1).toInt()==1){second=item;break;}QVERIFY(second);
    dot->setSelected(true);second->setSelected(true);for(auto* button:editor.findChildren<QPushButton*>())if(button->text()=="Group selected LEDs")button->click();
    QCOMPARE(dot->pos(),second->pos());dot->setSelected(false);second->setSelected(false);dot->setPos(dot->pos()+QPointF(10,5));QCOMPARE(dot->pos(),second->pos());
    auto* shape=editor.findChild<QComboBox*>("layoutShape");shape->setCurrentText("Rectangle");
    for(auto* button:editor.findChildren<QPushButton*>())if(button->text()=="Apply shape")button->click();
    const QString screenshot=qEnvironmentVariable("LIGHTING_STUDIO_SCREENSHOT");if(!screenshot.isEmpty())editor.grab().save(screenshot+".layout-editor.png");
    editor.findChild<QPushButton*>("resetComponentLayout")->click();editor.findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save)->click();
    QCOMPARE(editor.components()[0].ledCoordinates,fan.ledCoordinates);QCOMPARE(editor.components()[0].ledMapping,fan.ledMapping);
}

void LightingStudioWindowTest::testCustomHardwareDefinitionDialog()
{
    Settings settings;LightingStudioWindow window(settings,{"JARGB1"});
    auto* create=window.findChild<QPushButton*>("createHardwareDefinition:JARGB1");QVERIFY(create);
    QTimer::singleShot(0,[&]{auto* dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());if(!dialog)return;dialog->findChild<QLineEdit*>("hardwareName")->setText("My paired fan");dialog->findChild<QSpinBox*>("hardwareAddressCount")->setValue(6);dialog->findChild<QSpinBox*>("hardwareLedsPerAddress")->setValue(2);dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save)->click();});
    create->click();QCOMPARE(settings.hardwareCatalog.size(),size_t(1));QCOMPARE(settings.hardwareCatalog[0].ledCount,6);QCOMPARE(settings.hardwareCatalog[0].physicalLedsPerAddress,2);
    QTimer::singleShot(0,[&]{auto* dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());if(!dialog)return;
        dialog->findChild<QLineEdit*>("hardwareSearch")->setText("My paired fan");dialog->findChild<QPushButton*>("confirmAddHardware")->click();});
    window.findChild<QPushButton*>("addHardware:JARGB1")->click();
    QCOMPARE(settings.activeProfile()->channelSettings[0].components.size(),size_t(1));QCOMPARE(settings.activeProfile()->channelSettings[0].components[0].ledCount,6);
}

void LightingStudioWindowTest::testEditPresetColorsSurviveSaveAndReopen()
{
    Settings settings;LightingStudioWindow window(settings,{"JARGB1"});
    auto* effects=window.findChild<QComboBox*>("globalEffectCombo");QVERIFY(effects);
    effects->setCurrentIndex(effects->findText(QString("Aurora")));
    auto* edit=window.findChild<QPushButton*>("editEffectButton");QVERIFY(edit);
    QTimer::singleShot(0,[&]{
        auto* dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());QVERIFY(dialog);
        QCOMPARE(dialog->findChild<QLineEdit*>("effectRecipeName")->text(),QString("Aurora"));
        QCOMPARE(dialog->findChild<QSpinBox*>("stopR")->value(),38);
        dialog->findChild<QSpinBox*>("stopR")->setValue(231);
        dialog->findChild<QSpinBox*>("stopG")->setValue(19);
        QVERIFY(!dialog->findChild<QComboBox*>("effectPath")->isEnabled());
        dialog->findChild<QDialogButtonBox*>()->button(QDialogButtonBox::Save)->click();
    });edit->click();
    QCOMPARE(settings.customEffects.size(),size_t(1));
    QTemporaryDir directory;const auto file=directory.filePath("settings.json").toStdString();QVERIFY(settings.saveToFile(file));
    Settings restored;QVERIFY(restored.loadFromFile(file));
    QCOMPARE(restored.customEffects[0].colors[0].r,uint8_t(231));QCOMPARE(restored.customEffects[0].colors[0].g,uint8_t(19));
    QCOMPARE(restored.customEffects[0].path,std::string("Canvas positions"));
    LightingStudioWindow reopened(restored,{"JARGB1"});
    QTimer::singleShot(0,[&]{auto* dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());QVERIFY(dialog);
        QCOMPARE(dialog->findChild<QSpinBox*>("stopR")->value(),231);
        QCOMPARE(dialog->findChild<QComboBox*>("effectPath")->currentText(),QString("Canvas positions"));dialog->reject();});
    reopened.findChild<QPushButton*>("editEffectButton")->click();
}

void LightingStudioWindowTest::testChannelPickerAddsMultipleAndRemovesInline()
{
    Settings settings;LightingStudioWindow window(settings,{"JARGB1","JARGB2"});
    QTimer::singleShot(0,[&]{auto* dialog=qobject_cast<QDialog*>(QApplication::activeModalWidget());if(!dialog)return;
        dialog->findChild<QSpinBox*>("hardwareQuantity")->setValue(3);
        const auto output=qEnvironmentVariable("LIGHTING_STUDIO_SCREENSHOT");if(!output.isEmpty())dialog->grab().save(output+".hardware-picker.png");
        dialog->findChild<QPushButton*>("confirmAddHardware")->click();});
    window.findChild<QPushButton*>("addHardware:JARGB2")->click();
    QCOMPARE(settings.activeProfile()->channelSettings[0].components.size(),size_t(0));
    QCOMPARE(settings.activeProfile()->channelSettings[1].components.size(),size_t(3));
    window.findChild<QListWidget*>("deviceList")->setCurrentRow(1);
    auto* remove=window.findChild<QPushButton*>("removeHardware:JARGB2:1");QVERIFY(remove);remove->click();
    QCOMPARE(settings.activeProfile()->channelSettings[1].components.size(),size_t(2));
    QCOMPARE(settings.activeProfile()->channelSettings[1].components[1].displayName.find("#3")!=std::string::npos,true);
}

void LightingStudioWindowTest::testHardwarePickerFiltersAndRejectsOverCapacity()
{
    HardwarePicker picker("JARGB1",{Component::makeFan(6,true),Component::makeFan(12,false)},5);
    auto* add=picker.findChild<QPushButton*>("confirmAddHardware");QVERIFY(!add->isEnabled());
    picker.findChild<QLineEdit*>("hardwareSearch")->setText("no such product");QVERIFY(!add->isEnabled());
    HardwarePicker fits("JARGB2",{Component::makeFan(6,true)},18);
    auto* count=fits.findChild<QSpinBox*>("hardwareQuantity");count->setValue(120);QCOMPARE(count->value(),3);
    fits.findChild<QPushButton*>("confirmAddHardware")->click();QCOMPARE(fits.result(),int(QDialog::Accepted));
    HardwarePicker cancel("JARGB3",{Component::makeFan(6,true)},120);cancel.reject();QCOMPARE(cancel.result(),int(QDialog::Rejected));
}

void LightingStudioWindowTest::testFocusedNavigationAndArtworkResources()
{
    for(const auto* kind:{"fan","mouse","keyboard","keyboard-zones","strip","controller"}){QSvgRenderer image(QString(":/studio/devices/%1.svg").arg(kind));QVERIFY2(image.isValid(),kind);}
    Settings settings;LightingStudioWindow window(settings,{"JARGB1"});window.show();QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto* views=window.findChild<QTabWidget*>("workspaceViews");QCOMPARE(views->count(),3);
    QCOMPARE(views->tabText(0),QString("Layout"));QCOMPARE(views->tabText(1),QString("Effects"));QCOMPARE(views->tabText(2),QString("Hardware"));
    auto* create=window.findChild<QPushButton*>("createEffectButton");QVERIFY(!create->isVisible());
    QVERIFY(!window.findChild<QPushButton*>("openHardwareChains"));QVERIFY(!window.findChild<QPushButton*>("newCanvasEffect"));
    views->setCurrentIndex(1);QVERIFY(create->isVisible());QVERIFY(window.findChild<QComboBox*>("globalEffectCombo")->isVisible());
    QVERIFY(!window.findChild<QStackedWidget*>("deviceInspector")->isVisible());
    views->setCurrentIndex(0);QVERIFY(!create->isVisible());QVERIFY(window.canvas()->isVisible());
}

void LightingStudioWindowTest::testDurationSliderHasUsefulQuarterRangeAndExactSeconds()
{
    EffectEditor editor({"Timing",{{255,0,0},{0,0,255}},6000,720});
    auto* slider=editor.findChild<QSlider*>("effectDuration");auto* seconds=editor.findChild<QDoubleSpinBox*>("effectDurationSeconds");QVERIFY(slider && seconds);
    QCOMPARE(editor.definition().periodMs,6000);QCOMPARE(seconds->value(),6.);
    slider->setValue(250);QVERIFY(editor.definition().periodMs>=1900 && editor.definition().periodMs<=2000);
    QCOMPARE(int(std::lround(seconds->value()*1000)),editor.definition().periodMs);
    slider->setValue(0);QCOMPARE(editor.definition().periodMs,500);
    slider->setValue(1000);QCOMPARE(editor.definition().periodMs,120000);
    seconds->setValue(6.125);QCOMPARE(editor.definition().periodMs,6125);
    QVERIFY(slider->value()>400 && slider->value()<500);
    EffectEditor reopened(editor.definition());QCOMPARE(reopened.definition().periodMs,6125);
}

QTEST_MAIN(LightingStudioWindowTest)
#include "test_lighting_studio_window.moc"

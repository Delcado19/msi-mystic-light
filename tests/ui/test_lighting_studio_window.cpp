// Unit test for LightingStudioWindow (Task 15): canvas + side panel (per-
// channel mode/source/effect rows, profile selector) over a Settings
// instance. Same off-screen-widget/QTest pattern as
// test_lighting_canvas_widget.cpp (Task 14) -- construct, show(), drive
// combos/mouse events programmatically, no interactive desktop required.
#include <QtTest/QtTest>

#include <QComboBox>

#include <algorithm>
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

QTEST_MAIN(LightingStudioWindowTest)
#include "test_lighting_studio_window.moc"

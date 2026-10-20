// Unit test for LightingCanvasWidget (Task 14): the draggable-box canvas
// over a live-rendered background effect. Same off-screen-widget pattern
// as test_tray_app.cpp -- construct the widget, show() it (needed so
// QTest's synthetic mouse events have a real window to target) and drive
// it programmatically; no interactive desktop session required.
#include <QtTest/QtTest>

#include <memory>

#include "effects/IPositionableEffect.h"
#include "ui/LightingCanvasWidget.h"

// Minimal non-positionable fake, standing in for effects like
// StaticColorEffect/BreathingEffect that implement only IEffect.
class FakeEffect : public IEffect {
public:
    MsiColor colorAt(int, int, int64_t) const override { return MsiColor{10, 20, 30}; }
};

// Minimal fake standing in for the IPositionableEffect-implementing
// effects (RadialRainbowEffect, SpiralEffect, SwirlCirclesEffect) that
// expose a draggable reference point on the canvas.
class FakePositionableEffect : public IEffect, public IPositionableEffect {
public:
    MsiColor colorAt(int, int, int64_t) const override { return MsiColor{40, 50, 60}; }
    void setReferencePoint(int x, int y) override { x_ = x; y_ = y; }
    std::pair<int, int> referencePoint() const override { return {x_, y_}; }

private:
    int x_ = 60, y_ = 60;
};

class LightingCanvasWidgetTest : public QObject
{
    Q_OBJECT

private slots:
    void testSetChannel_thenRemoveChannel_doesNotCrash();
    void testMousePressDragRelease_onAChannelBox_emitsChannelPlacementChanged();
    void testMousePressDragRelease_onGlobalEffectHandle_emitsGlobalEffectReferencePointChanged();
    void testPaintEvent_doesNotCrashWithNoChannelsRegistered();
    void testDragBoxFarPastWidgetEdge_clampsPlacementToVisibleBounds();
};

void LightingCanvasWidgetTest::testSetChannel_thenRemoveChannel_doesNotCrash()
{
    LightingCanvasWidget widget;
    widget.resize(300, 300);
    widget.setChannel("CPU", CanvasPlacement{10, 10}, CanvasSource::Independent, std::make_shared<FakeEffect>());
    widget.grab(); // forces a paintEvent while the channel is registered
    widget.removeChannel("CPU");
    widget.grab(); // and once more after removal
}

void LightingCanvasWidgetTest::testMousePressDragRelease_onAChannelBox_emitsChannelPlacementChanged()
{
    LightingCanvasWidget widget;
    widget.resize(300, 300);
    const CanvasPlacement originalPlacement{20.0, 20.0};
    widget.setChannel("GPU", originalPlacement, CanvasSource::Independent, std::make_shared<FakeEffect>());
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&widget));

    const QPoint pressPos(static_cast<int>(originalPlacement.x) + LightingCanvasWidget::kBoxWidth / 2,
                           static_cast<int>(originalPlacement.y) + LightingCanvasWidget::kBoxHeight / 2);
    const QPoint movePos = pressPos + QPoint(30, 15);

    QSignalSpy spy(&widget, &LightingCanvasWidget::channelPlacementChanged);
    QTest::mousePress(&widget, Qt::LeftButton, Qt::NoModifier, pressPos);
    QTest::mouseMove(&widget, movePos);
    QTest::mouseRelease(&widget, Qt::LeftButton, Qt::NoModifier, movePos);

    QVERIFY(spy.count() >= 1);
    const auto lastCall = spy.last();
    QCOMPARE(lastCall.at(0).value<std::string>(), std::string("GPU"));
    const CanvasPlacement newPlacement = lastCall.at(1).value<CanvasPlacement>();
    QVERIFY(newPlacement.x != originalPlacement.x || newPlacement.y != originalPlacement.y);
}

void LightingCanvasWidgetTest::testMousePressDragRelease_onGlobalEffectHandle_emitsGlobalEffectReferencePointChanged()
{
    LightingCanvasWidget widget;
    widget.resize(300, 300);
    auto fakePositionable = std::make_shared<FakePositionableEffect>();
    widget.setGlobalEffect(fakePositionable);

    const auto [startX, startY] = fakePositionable->referencePoint();
    // Registers a channel box overlapping the handle's position on
    // purpose: the handle is drawn on top (paintEvent draws it last), so
    // pressing here must grab the handle, not the box underneath it.
    // Locks in the hit-test precedence fixed after initial review.
    widget.setChannel("CPU", CanvasPlacement{static_cast<double>(startX) - 10, static_cast<double>(startY) - 10}, CanvasSource::Independent, std::make_shared<FakeEffect>());
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&widget));

    const QPoint pressPos(startX, startY);
    const QPoint movePos = pressPos + QPoint(25, 10);

    QSignalSpy spy(&widget, &LightingCanvasWidget::globalEffectReferencePointChanged);
    QTest::mousePress(&widget, Qt::LeftButton, Qt::NoModifier, pressPos);
    QTest::mouseMove(&widget, movePos);
    QTest::mouseRelease(&widget, Qt::LeftButton, Qt::NoModifier, movePos);

    QVERIFY(spy.count() >= 1);
    const auto lastCall = spy.last();
    QCOMPARE(lastCall.at(0).toInt(), movePos.x());
    QCOMPARE(lastCall.at(1).toInt(), movePos.y());
    QCOMPARE(fakePositionable->referencePoint(), std::make_pair(movePos.x(), movePos.y()));
}

void LightingCanvasWidgetTest::testPaintEvent_doesNotCrashWithNoChannelsRegistered()
{
    LightingCanvasWidget widget;
    widget.resize(300, 300);
    widget.grab(); // no channels, no global effect -- just the empty background/outline
}

// Regression test for the review finding that mouseMoveEvent did not clamp
// drag-produced placements to the widget's own bounds: Qt keeps delivering
// move/release events past rect() (including negative coordinates) while a
// button is held, so dragging a box far past an edge and releasing there
// used to persist an off-canvas placement with no way to click it again.
void LightingCanvasWidgetTest::testDragBoxFarPastWidgetEdge_clampsPlacementToVisibleBounds()
{
    LightingCanvasWidget widget;
    widget.resize(300, 300);
    const CanvasPlacement originalPlacement{20.0, 20.0};
    widget.setChannel("GPU", originalPlacement, CanvasSource::Independent, std::make_shared<FakeEffect>());
    widget.show();
    QVERIFY(QTest::qWaitForWindowExposed(&widget));

    const QPoint pressPos(static_cast<int>(originalPlacement.x) + LightingCanvasWidget::kBoxWidth / 2,
                           static_cast<int>(originalPlacement.y) + LightingCanvasWidget::kBoxHeight / 2);
    // Way past the widget's right/bottom edge (widget is 300x300).
    const QPoint farOutsidePos(10000, 10000);

    QSignalSpy spy(&widget, &LightingCanvasWidget::channelPlacementChanged);
    QTest::mousePress(&widget, Qt::LeftButton, Qt::NoModifier, pressPos);
    QTest::mouseMove(&widget, farOutsidePos);
    QTest::mouseRelease(&widget, Qt::LeftButton, Qt::NoModifier, farOutsidePos);

    QVERIFY(spy.count() >= 1);
    const CanvasPlacement newPlacement = spy.last().at(1).value<CanvasPlacement>();
    QVERIFY(newPlacement.x >= 0 && newPlacement.x <= widget.width() - LightingCanvasWidget::kBoxWidth);
    QVERIFY(newPlacement.y >= 0 && newPlacement.y <= widget.height() - LightingCanvasWidget::kBoxHeight);
}

QTEST_MAIN(LightingCanvasWidgetTest)
#include "test_lighting_canvas_widget.moc"

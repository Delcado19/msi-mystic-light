// Unit tests for Canvas (Task 8): the per-frame render-loop dispatcher
// tying together IEffect (Task 7), Channel (Task 5) and
// MsiMotherboardDriver (Task 6). Uses FakeHidDevice (tests/hid/FakeHidDevice.h)
// and FakeEffect (tests/canvas/FakeEffect.h) - no real hardware or effect
// math involved.
#include <QtTest/QtTest>

#include <map>
#include <memory>

#include "canvas/Canvas.h"
#include "device/MsiMotherboardDriver.h"
#include "hid/FakeHidDevice.h"
#include "canvas/FakeEffect.h"

class CanvasTest : public QObject
{
    Q_OBJECT

private slots:
    void testTick_canvasModeSamplesEffectAndPushes();
    void testTick_nativeModeSendsOnlyOnceAfterAssignment();
    void testTick_continuesAfterOneChannelFails();
    void testTick_continuesAfterFailureWhenFailingChannelIsNotLast();
};

void CanvasTest::testTick_canvasModeSamplesEffectAndPushes()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    auto defaultChannels = MsiMotherboardDriver::createDefaultChannels();
    std::map<std::string, Channel> channels;
    for (auto& c : defaultChannels) channels.emplace(c.name(), c);
    channels.at("JARGB1").addComponent(Component::makeGenericPlaceholder(2));
    driver.setChannelMode("JARGB1", ChannelMode::Canvas);

    Canvas canvas(driver, channels);
    canvas.setAssignment("JARGB1", ChannelAssignment{ChannelMode::Canvas, std::make_shared<FakeEffect>(MsiColor{1,2,3}), {}});
    canvas.tick(0);

    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);
    QCOMPARE(hid.sentReports[0][0], static_cast<uint8_t>(0x51));
    QCOMPARE(hid.sentReports[0][7], static_cast<uint8_t>(1)); // first LED's R
}

void CanvasTest::testTick_nativeModeSendsOnlyOnceAfterAssignment()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    auto defaultChannels = MsiMotherboardDriver::createDefaultChannels();
    std::map<std::string, Channel> channels;
    for (auto& c : defaultChannels) channels.emplace(c.name(), c);

    Canvas canvas(driver, channels);
    canvas.setAssignment("JARGB2", ChannelAssignment{ChannelMode::Native, nullptr, NativeModeSettings{}});
    canvas.tick(0);
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);
    canvas.tick(16); // next frame, no re-assignment
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1); // still 1, not sent again
}

void CanvasTest::testTick_continuesAfterOneChannelFails()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    auto defaultChannels = MsiMotherboardDriver::createDefaultChannels();
    std::map<std::string, Channel> channels;
    for (auto& c : defaultChannels) channels.emplace(c.name(), c);
    channels.at("JARGB1").addComponent(Component::makeGenericPlaceholder(1));
    channels.at("JARGB2").addComponent(Component::makeGenericPlaceholder(1));
    driver.setChannelMode("JARGB1", ChannelMode::Canvas);
    driver.setChannelMode("JARGB2", ChannelMode::Canvas);

    Canvas canvas(driver, channels);
    canvas.setAssignment("JARGB1", ChannelAssignment{ChannelMode::Canvas, std::make_shared<FakeEffect>(MsiColor{1,1,1}), {}});
    canvas.setAssignment("JARGB2", ChannelAssignment{ChannelMode::Canvas, std::make_shared<FakeEffect>(MsiColor{2,2,2}), {}});
    hid.sendSucceeds = false; // every send "fails" at the OS level
    canvas.tick(0); // must not throw
    QCOMPARE(static_cast<int>(hid.sentReports.size()), 2); // both channels still attempted
}

// Review Focus item for this task: "A HID feature-report send failing
// mid-session - the render loop must keep ticking the other channels, not
// abort the whole frame." The brief's given test above only covers a
// hardware-level failure that hits every channel identically (hid.sendSucceeds
// = false), which can't tell a true continue-past-one-failure from simply
// "nothing throws". This test makes exactly one channel's driver call fail
// (JARGB1's pushCanvasColors, because its driver-side mode is still Native
// while its Canvas assignment says otherwise) while a later channel
// (JARGB2, assigned/mode-matched correctly) succeeds - and JARGB1 iterates
// first (std::map key order), so the failure is not the last channel tried.
void CanvasTest::testTick_continuesAfterFailureWhenFailingChannelIsNotLast()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    auto defaultChannels = MsiMotherboardDriver::createDefaultChannels();
    std::map<std::string, Channel> channels;
    for (auto& c : defaultChannels) channels.emplace(c.name(), c);
    channels.at("JARGB1").addComponent(Component::makeGenericPlaceholder(1));
    channels.at("JARGB2").addComponent(Component::makeGenericPlaceholder(1));
    // JARGB1 intentionally left in the driver's default Native mode, so its
    // pushCanvasColors call below fails (mode mismatch) without touching hid.
    driver.setChannelMode("JARGB2", ChannelMode::Canvas);

    Canvas canvas(driver, channels);
    canvas.setAssignment("JARGB1", ChannelAssignment{ChannelMode::Canvas, std::make_shared<FakeEffect>(MsiColor{1,1,1}), {}});
    canvas.setAssignment("JARGB2", ChannelAssignment{ChannelMode::Canvas, std::make_shared<FakeEffect>(MsiColor{2,2,2}), {}});
    canvas.tick(0); // must not throw, must still reach JARGB2

    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1); // only JARGB2 actually sent
    QCOMPARE(hid.sentReports[0][7], static_cast<uint8_t>(2)); // JARGB2's LED color, R=2
}

QTEST_MAIN(CanvasTest)
#include "test_canvas.moc"

// Unit tests for Canvas (Task 8, extended by Task 3's canvas-global
// placement/follow-global-effect). Uses FakeHidDevice
// (tests/hid/FakeHidDevice.h), FakeEffect (tests/canvas/FakeEffect.h) and
// the test-local PositionColorEffect/FakeDriver below - no real hardware or
// effect math involved.
#include <QtTest/QtTest>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "canvas/Canvas.h"
#include "device/IDriver.h"
#include "device/MsiMotherboardDriver.h"
#include "hid/FakeHidDevice.h"
#include "canvas/FakeEffect.h"

namespace {

// Builds a driversByChannel map routing every channel in `channels` to the
// same driver -- the common case in these tests, where one
// MsiMotherboardDriver owns every channel on the board.
std::map<std::string, IDriver*> allChannelsTo(IDriver* driver, const std::map<std::string, Channel>& channels)
{
    std::map<std::string, IDriver*> result;
    for (const auto& [name, channel] : channels) {
        (void)channel;
        result[name] = driver;
    }
    return result;
}

// Test-only IEffect that encodes its sampled (x,y) into the returned color
// (x -> r, y -> g), so a test can tell apart "sampled at local (x,y)" from
// "sampled at global placement+local (x,y)" by just reading the color back.
class PositionColorEffect : public IEffect {
public:
    MsiColor colorAt(int x, int y, int64_t) const override {
        return MsiColor{static_cast<uint8_t>(x), static_cast<uint8_t>(y), 0};
    }
};

// Test double for IDriver, recording which channel name each call targeted.
// Used by the multi-driver routing test to confirm Canvas sends each
// channel's push only to that channel's own driver, never a sibling's.
class FakeDriver : public IDriver {
public:
    bool setChannelMode(const std::string&, ChannelMode) override { return true; }
    ChannelMode channelMode(const std::string&) const override { return ChannelMode::Canvas; }
    bool setNativeMode(const std::string& channelName, const NativeModeSettings&) override {
        nativeModeChannels.push_back(channelName);
        return true;
    }
    bool pushCanvasColors(const std::string& channelName, const std::vector<MsiColor>&) override {
        pushedChannels.push_back(channelName);
        return true;
    }
    std::vector<std::string> pushedChannels;
    std::vector<std::string> nativeModeChannels;
};

} // namespace

class CanvasTest : public QObject
{
    Q_OBJECT

private slots:
    void testTick_canvasModeSamplesEffectAndPushes();
    void testTick_nativeModeSendsOnlyOnceAfterAssignment();
    void testTick_continuesAfterOneChannelFails();
    void testTick_continuesAfterFailureWhenFailingChannelIsNotLast();
    void testTick_globalFollowSamplesGlobalEffectAtPlacementOffset();
    void testTick_globalFollowWithNoGlobalEffectDoesNotCrash();
    void testTick_extremePlacementDoesNotCrash();
    void testTick_multipleDriversEachReceiveOnlyTheirOwnChannel();
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

    Canvas canvas(allChannelsTo(&driver, channels), channels);
    canvas.setAssignment("JARGB1", ChannelAssignment{ChannelMode::Canvas, CanvasSource::Independent, std::make_shared<FakeEffect>(MsiColor{1,2,3}), {}, {}});
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

    Canvas canvas(allChannelsTo(&driver, channels), channels);
    canvas.setAssignment("JARGB2", ChannelAssignment{ChannelMode::Native, CanvasSource::Independent, nullptr, NativeModeSettings{}, {}});
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

    Canvas canvas(allChannelsTo(&driver, channels), channels);
    canvas.setAssignment("JARGB1", ChannelAssignment{ChannelMode::Canvas, CanvasSource::Independent, std::make_shared<FakeEffect>(MsiColor{1,1,1}), {}, {}});
    canvas.setAssignment("JARGB2", ChannelAssignment{ChannelMode::Canvas, CanvasSource::Independent, std::make_shared<FakeEffect>(MsiColor{2,2,2}), {}, {}});
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

    Canvas canvas(allChannelsTo(&driver, channels), channels);
    canvas.setAssignment("JARGB1", ChannelAssignment{ChannelMode::Canvas, CanvasSource::Independent, std::make_shared<FakeEffect>(MsiColor{1,1,1}), {}, {}});
    canvas.setAssignment("JARGB2", ChannelAssignment{ChannelMode::Canvas, CanvasSource::Independent, std::make_shared<FakeEffect>(MsiColor{2,2,2}), {}, {}});
    canvas.tick(0); // must not throw, must still reach JARGB2

    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1); // only JARGB2 actually sent
    QCOMPARE(hid.sentReports[0][7], static_cast<uint8_t>(2)); // JARGB2's LED color, R=2
}

void CanvasTest::testTick_globalFollowSamplesGlobalEffectAtPlacementOffset()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    auto defaultChannels = MsiMotherboardDriver::createDefaultChannels();
    std::map<std::string, Channel> channels;
    for (auto& c : defaultChannels) channels.emplace(c.name(), c);
    // Generic placeholder lays 2 LEDs at local (0,0) and (1,0).
    channels.at("JARGB1").addComponent(Component::makeGenericPlaceholder(2));
    driver.setChannelMode("JARGB1", ChannelMode::Canvas);

    Canvas canvas(allChannelsTo(&driver, channels), channels);
    canvas.setGlobalEffect(std::make_shared<PositionColorEffect>());
    ChannelAssignment assignment;
    assignment.mode = ChannelMode::Canvas;
    assignment.source = CanvasSource::GlobalFollow;
    assignment.placement = CanvasPlacement{10, 0};
    canvas.setAssignment("JARGB1", assignment);
    canvas.tick(0);

    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);
    // Sampled at placement+local (10,0) and (11,0) -- NOT at local (0,0)/(1,0),
    // which PositionColorEffect would instead report as R=0 and R=1.
    QCOMPARE(hid.sentReports[0][7], static_cast<uint8_t>(10));     // first LED's R
    QCOMPARE(hid.sentReports[0][7 + 3], static_cast<uint8_t>(11)); // second LED's R (stride 3 bytes/LED)
}

void CanvasTest::testTick_globalFollowWithNoGlobalEffectDoesNotCrash()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    auto defaultChannels = MsiMotherboardDriver::createDefaultChannels();
    std::map<std::string, Channel> channels;
    for (auto& c : defaultChannels) channels.emplace(c.name(), c);
    channels.at("JARGB1").addComponent(Component::makeGenericPlaceholder(1));
    driver.setChannelMode("JARGB1", ChannelMode::Canvas);

    Canvas canvas(allChannelsTo(&driver, channels), channels);
    // setGlobalEffect() deliberately never called.
    ChannelAssignment assignment;
    assignment.mode = ChannelMode::Canvas;
    assignment.source = CanvasSource::GlobalFollow;
    canvas.setAssignment("JARGB1", assignment);
    canvas.tick(0); // must not throw/crash

    QCOMPARE(static_cast<int>(hid.sentReports.size()), 0); // pushCanvasColors never reached
}

void CanvasTest::testTick_extremePlacementDoesNotCrash()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    auto defaultChannels = MsiMotherboardDriver::createDefaultChannels();
    std::map<std::string, Channel> channels;
    for (auto& c : defaultChannels) channels.emplace(c.name(), c);
    channels.at("JARGB1").addComponent(Component::makeGenericPlaceholder(1));
    driver.setChannelMode("JARGB1", ChannelMode::Canvas);

    Canvas canvas(allChannelsTo(&driver, channels), channels);
    canvas.setGlobalEffect(std::make_shared<FakeEffect>(MsiColor{9,9,9}));
    ChannelAssignment assignment;
    assignment.mode = ChannelMode::Canvas;
    assignment.source = CanvasSource::GlobalFollow;
    assignment.placement = CanvasPlacement{1e9, -1e9};
    canvas.setAssignment("JARGB1", assignment);
    canvas.tick(0); // must not throw/crash (Review Focus item)

    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);
}

void CanvasTest::testTick_multipleDriversEachReceiveOnlyTheirOwnChannel()
{
    std::map<std::string, Channel> channels;
    channels.emplace("ChannelA", Channel("ChannelA", 4));
    channels.emplace("ChannelB", Channel("ChannelB", 4));
    channels.at("ChannelA").addComponent(Component::makeGenericPlaceholder(1));
    channels.at("ChannelB").addComponent(Component::makeGenericPlaceholder(1));

    FakeDriver driverA;
    FakeDriver driverB;
    std::map<std::string, IDriver*> driversByChannel{{"ChannelA", &driverA}, {"ChannelB", &driverB}};

    Canvas canvas(driversByChannel, channels);
    canvas.setAssignment("ChannelA", ChannelAssignment{ChannelMode::Canvas, CanvasSource::Independent, std::make_shared<FakeEffect>(MsiColor{1,1,1}), {}, {}});
    canvas.setAssignment("ChannelB", ChannelAssignment{ChannelMode::Canvas, CanvasSource::Independent, std::make_shared<FakeEffect>(MsiColor{2,2,2}), {}, {}});
    canvas.tick(0);

    QCOMPARE(static_cast<int>(driverA.pushedChannels.size()), 1);
    QCOMPARE(driverA.pushedChannels[0], std::string("ChannelA"));
    QCOMPARE(static_cast<int>(driverB.pushedChannels.size()), 1);
    QCOMPARE(driverB.pushedChannels[0], std::string("ChannelB"));
}

QTEST_MAIN(CanvasTest)
#include "test_canvas.moc"

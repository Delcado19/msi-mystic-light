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

// Builds a synthetic grid-shaped component (local (col,row) per LED,
// row-major), to stand in for a real 2D component layout (e.g. the Roccat
// Vulcan's 24x6 key grid) without linking that driver's source just for
// this test -- see testTick_globalFollowScalesDifferentShapedChannelsIntoSameBox.
Component makeGridComponent(int width, int height) {
    Component c = Component::makeGenericPlaceholder(width * height);
    c.width = width;
    c.height = height;
    for (int row = 0; row < height; ++row) {
        for (int col = 0; col < width; ++col) {
            c.ledCoordinates[row * width + col] = {col, row};
        }
    }
    return c;
}

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
    void testTick_globalFollowScalesDifferentShapedChannelsIntoSameBox();
    void testTick_globalFollowSingleLedChannelSamplesBoxCenter();
    void testTick_globalFollowWithNoGlobalEffectDoesNotCrash();
    void testTick_extremePlacementDoesNotCrash();
    void testTick_multipleDriversEachReceiveOnlyTheirOwnChannel();
    void testTick_canvasModeChannelWithNoRoutedDriverIsSkipped();
    void testTick_nativeModeChannelWithNoRoutedDriverIsSkipped();
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
    // setChannelMode(Canvas) itself sends an arming 0x50 report (see
    // MsiMotherboardDriverTest::testSetChannelModeCanvas_armsZoneToAdvancedBefore0x51Push);
    // clear it here so this test's assertions only count what tick() sends.
    hid.sentReports.clear();

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
    // Clear the two arming 0x50 reports setChannelMode just sent, so this
    // test's assertions only count what tick() sends.
    hid.sentReports.clear();

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
    // Clear the arming 0x50 report setChannelMode just sent, so this test's
    // assertions only count what tick() sends.
    hid.sentReports.clear();

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
    // createDefaultChannels() already attaches each channel's own full-size
    // generic placeholder (JARGB1's is 120 LEDs, local x 0..119, y always
    // 0) -- no extra addComponent() needed/possible here (it would just
    // exceed the channel's ledLimit and no-op).
    for (auto& c : defaultChannels) channels.emplace(c.name(), c);
    driver.setChannelMode("JARGB1", ChannelMode::Canvas);
    // Clear the arming 0x50 report setChannelMode just sent, so this test's
    // assertions only count what tick() sends.
    hid.sentReports.clear();

    Canvas canvas(allChannelsTo(&driver, channels), channels);
    canvas.setGlobalEffect(std::make_shared<PositionColorEffect>());
    ChannelAssignment assignment;
    assignment.mode = ChannelMode::Canvas;
    assignment.source = CanvasSource::GlobalFollow;
    assignment.placement = CanvasPlacement{10, 0};
    canvas.setAssignment("JARGB1", assignment);
    canvas.tick(0);

    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);
    // Task 22 geometry fix: local x is rescaled from the channel's own
    // [0, 119] extent into [0, kGlobalFollowBoxWidth - 1] BEFORE adding the
    // placement offset, so LED 0 (local x=0) lands at placement+0 and LED
    // 119 (local x=119, the channel's own far end) lands at
    // placement+(kGlobalFollowBoxWidth-1) -- not at placement+119, which
    // would run far past the on-screen box LightingCanvasWidget draws for
    // this channel.
    QCOMPARE(hid.sentReports[0][7], static_cast<uint8_t>(10)); // LED 0's R: placement.x(10) + scaled(0)
    QCOMPARE(hid.sentReports[0][7 + 119 * 3],
             static_cast<uint8_t>(10 + kGlobalFollowBoxWidth - 1)); // LED 119's R: placement.x(10) + scaled(119->69)
}

// Task 22 geometry fix: confirms the fix is channel-shape-generic, not
// hardcoded per device. JARGB1 keeps its real 120-wide flat-line placeholder
// (motherboard channels); EZConnector is given a synthetic 24x5 grid (same
// column count as the Roccat Vulcan's real 24x6 key layout, trimmed to 5
// rows so it still fits EZConnector's 120-LED test limit) in place of its
// own default placeholder. Both channels' extreme LEDs (the ones furthest
// from their own component's local origin) must land exactly on the shared
// GlobalFollow box's far edge -- proving a 120-LED line and a 2D grid are
// scaled into the SAME box size, regardless of raw LED count or shape.
void CanvasTest::testTick_globalFollowScalesDifferentShapedChannelsIntoSameBox()
{
    FakeHidDevice hid; hid.open(0x0DB0, 0x0076);
    MsiMotherboardDriver driver(hid);
    auto defaultChannels = MsiMotherboardDriver::createDefaultChannels();
    std::map<std::string, Channel> channels;
    for (auto& c : defaultChannels) channels.emplace(c.name(), c);
    channels.at("EZConnector").clearComponents();
    channels.at("EZConnector").addComponent(makeGridComponent(24, 5));

    driver.setChannelMode("JARGB1", ChannelMode::Canvas);
    driver.setChannelMode("EZConnector", ChannelMode::Canvas);
    hid.sentReports.clear();

    Canvas canvas(allChannelsTo(&driver, channels), channels);
    canvas.setGlobalEffect(std::make_shared<PositionColorEffect>());

    ChannelAssignment assignment;
    assignment.mode = ChannelMode::Canvas;
    assignment.source = CanvasSource::GlobalFollow;
    assignment.placement = CanvasPlacement{0, 0};
    canvas.setAssignment("JARGB1", assignment);
    canvas.setAssignment("EZConnector", assignment);
    canvas.tick(0);

    const auto& lineColors = channels.at("JARGB1").colors();
    QCOMPARE(lineColors[0].r, static_cast<uint8_t>(0));                          // local x=0 -> scaled 0
    QCOMPARE(lineColors[119].r, static_cast<uint8_t>(kGlobalFollowBoxWidth - 1)); // local x=119 (far end) -> scaled box edge
    // y has no extent at all (every LED is row 0) -- scaleIntoBox() maps that
    // degenerate axis to the box's vertical center, not 0, so the line
    // samples through the box's middle rather than hugging its top edge.
    QCOMPARE(lineColors[0].g, static_cast<uint8_t>((kGlobalFollowBoxHeight - 1) / 2));
    QCOMPARE(lineColors[119].g, static_cast<uint8_t>((kGlobalFollowBoxHeight - 1) / 2));

    const auto& gridColors = channels.at("EZConnector").colors();
    QCOMPARE(gridColors[0].r, static_cast<uint8_t>(0));   // local (0,0) -> scaled (0,0)
    QCOMPARE(gridColors[0].g, static_cast<uint8_t>(0));
    QCOMPARE(gridColors[119].r, static_cast<uint8_t>(kGlobalFollowBoxWidth - 1));  // local col=23 (far end) -> scaled box edge
    QCOMPARE(gridColors[119].g, static_cast<uint8_t>(kGlobalFollowBoxHeight - 1)); // local row=4 (far end) -> scaled box edge
}

// Task 22 geometry fix, degenerate-extent case: a 1-LED channel (e.g. a
// mouse DPI indicator) has no extent on EITHER axis -- its one LED's local
// (x,y) is trivially its own min and max. scaleIntoBox() maps that to the
// box's center on both axes, so this channel samples the middle of its
// drawn box. Mapping to 0 instead (this task's first attempt) would have
// sampled the box's top-left corner -- indistinguishable from the pre-fix
// bug for exactly this case, since a 1-LED channel has no raw coordinate to
// run past the box in the first place.
void CanvasTest::testTick_globalFollowSingleLedChannelSamplesBoxCenter()
{
    std::map<std::string, Channel> channels;
    channels.emplace("Solo", Channel("Solo", 1));
    channels.at("Solo").addComponent(Component::makeGenericPlaceholder(1));

    FakeDriver driver;
    std::map<std::string, IDriver*> driversByChannel{{"Solo", &driver}};

    Canvas canvas(driversByChannel, channels);
    canvas.setGlobalEffect(std::make_shared<PositionColorEffect>());

    ChannelAssignment assignment;
    assignment.mode = ChannelMode::Canvas;
    assignment.source = CanvasSource::GlobalFollow;
    assignment.placement = CanvasPlacement{5, 7};
    canvas.setAssignment("Solo", assignment);
    canvas.tick(0);

    const auto& colors = channels.at("Solo").colors();
    QCOMPARE(colors[0].r, static_cast<uint8_t>(5 + (kGlobalFollowBoxWidth - 1) / 2));
    QCOMPARE(colors[0].g, static_cast<uint8_t>(7 + (kGlobalFollowBoxHeight - 1) / 2));
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
    // Clear the arming 0x50 report setChannelMode just sent, so this test's
    // assertions only count what tick() sends.
    hid.sentReports.clear();

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
    // Clear the arming 0x50 report setChannelMode just sent, so this test's
    // assertions only count what tick() sends.
    hid.sentReports.clear();

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

// Covers the "no driver routed for this channel" skip path in tick()
// (added by this task's map-based driver routing) for Canvas mode: the
// channel has an assignment but no entry in driversByChannel_ at all.
void CanvasTest::testTick_canvasModeChannelWithNoRoutedDriverIsSkipped()
{
    std::map<std::string, Channel> channels;
    channels.emplace("Ghost", Channel("Ghost", 4));
    channels.at("Ghost").addComponent(Component::makeGenericPlaceholder(1));

    std::map<std::string, IDriver*> driversByChannel; // deliberately empty: "Ghost" has no routed driver
    Canvas canvas(driversByChannel, channels);
    canvas.setAssignment("Ghost", ChannelAssignment{ChannelMode::Canvas, CanvasSource::Independent, std::make_shared<FakeEffect>(MsiColor{9,9,9}), {}, {}});
    canvas.tick(0); // must not throw/crash

    // The skip must happen before any per-LED sampling, not just "nothing
    // crashed": the effect is never consulted, so the LED stays at its
    // component-default color (0,0,0), not FakeEffect's {9,9,9}.
    QCOMPARE(channels.at("Ghost").colors()[0].r, static_cast<uint8_t>(0));
}

// Same skip path, for Native mode: confirms an unrouted channel's
// setNativeMode is never called, while a routed sibling channel still goes
// through normally (isolation, not just "no crash").
void CanvasTest::testTick_nativeModeChannelWithNoRoutedDriverIsSkipped()
{
    std::map<std::string, Channel> channels;
    channels.emplace("Real", Channel("Real", 4));
    channels.emplace("Ghost", Channel("Ghost", 4));

    FakeDriver driver;
    std::map<std::string, IDriver*> driversByChannel{{"Real", &driver}}; // "Ghost" deliberately unrouted

    Canvas canvas(driversByChannel, channels);
    canvas.setAssignment("Real", ChannelAssignment{ChannelMode::Native, CanvasSource::Independent, nullptr, NativeModeSettings{}, {}});
    canvas.setAssignment("Ghost", ChannelAssignment{ChannelMode::Native, CanvasSource::Independent, nullptr, NativeModeSettings{}, {}});
    canvas.tick(0); // must not throw/crash

    QCOMPARE(static_cast<int>(driver.nativeModeChannels.size()), 1);
    QCOMPARE(driver.nativeModeChannels[0], std::string("Real"));
}

QTEST_MAIN(CanvasTest)
#include "test_canvas.moc"

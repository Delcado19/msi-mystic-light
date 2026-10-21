// Unit tests for the Channel model (LED-chain bookkeeping for one
// controller output, e.g. one ARGB header). Pure data, no HID/device access.
#include <QtTest/QtTest>

#include "device/Channel.h"
#include "device/Component.h"

class ChannelTest : public QObject
{
    Q_OBJECT

private slots:
    void testFanChainsAndAtomicReplacement();
    void testAddComponent_withinLimit();
    void testAddComponent_exceedsLimitRejected();
    void testSetColorAndLedPosition();
    void testSetColor_outOfRangeIsNoOp();
    void testLedPosition_secondComponentUsesLocalIndex();
    void testNegativeIndex_isOutOfRange();
    void testClearComponents_resetsState();
};

void ChannelTest::testAddComponent_withinLimit()
{
    Channel ch("ARGB Header 1", 10);
    QVERIFY(ch.addComponent(Component::makeGenericPlaceholder(6)));
    QCOMPARE(ch.totalLedCount(), 6);
}

void ChannelTest::testAddComponent_exceedsLimitRejected()
{
    Channel ch("ARGB Header 1", 10);
    QVERIFY(ch.addComponent(Component::makeGenericPlaceholder(6)));
    QVERIFY(!ch.addComponent(Component::makeGenericPlaceholder(6))); // 6+6=12 > 10
    QCOMPARE(ch.totalLedCount(), 6); // unchanged
}

void ChannelTest::testSetColorAndLedPosition()
{
    Channel ch("ARGB Header 1", 10);
    ch.addComponent(Component::makeGenericPlaceholder(3));
    ch.setColor(1, MsiColor{10,20,30});
    QCOMPARE(ch.colors()[1].r, static_cast<uint8_t>(10));
    auto pos = ch.ledPosition(1);
    QVERIFY(pos.has_value());
    QCOMPARE(pos->first, 1); // placeholder component: x == local index
    QVERIFY(!ch.ledPosition(99).has_value());
}

// Controller-ruling addition (not in the brief's 3 given tests): the header
// doc comment for setColor specifies out-of-range indices are a no-op, but
// the brief's given tests only exercise that branch for ledPosition, not
// setColor. Exercises it here to keep the documented contract covered.
void ChannelTest::testSetColor_outOfRangeIsNoOp()
{
    Channel ch("ARGB Header 1", 10);
    ch.addComponent(Component::makeGenericPlaceholder(3));
    ch.setColor(99, MsiColor{10,20,30}); // out of range, must not crash or mutate
    QCOMPARE(ch.colors()[0].r, static_cast<uint8_t>(0));
    QCOMPARE(ch.colors()[1].r, static_cast<uint8_t>(0));
    QCOMPARE(ch.colors()[2].r, static_cast<uint8_t>(0));
}

// Advisor-flagged gap: every other test uses a single component, so
// ledPosition's "index - offset" local-index translation was never actually
// exercised (offset is always 0). With two components, ledPosition(3) must
// map to the SECOND component's local index 0, not its own global index 3 -
// a missing "- offset" would wrongly return {3,0} here instead of {0,0}.
void ChannelTest::testLedPosition_secondComponentUsesLocalIndex()
{
    Channel ch("ARGB Header 1", 10);
    ch.addComponent(Component::makeGenericPlaceholder(3)); // global indices 0..2
    ch.addComponent(Component::makeGenericPlaceholder(4)); // global indices 3..6
    auto pos = ch.ledPosition(3);
    QVERIFY(pos.has_value());
    QCOMPARE(pos->first, 0); // local index 0 of the second component
    QCOMPARE(pos->second, 0);
}

// Advisor-flagged gap: setColor/ledPosition's out-of-range guards were only
// tested on the above-range side; a negative index is the side that would be
// undefined behaviour (negative vector subscript) if the `index < 0` half of
// a guard were ever dropped.
void ChannelTest::testNegativeIndex_isOutOfRange()
{
    Channel ch("ARGB Header 1", 10);
    ch.addComponent(Component::makeGenericPlaceholder(3));
    ch.setColor(-1, MsiColor{10,20,30}); // must not crash or mutate
    QCOMPARE(ch.colors()[0].r, static_cast<uint8_t>(0));
    QVERIFY(!ch.ledPosition(-1).has_value());
}

void ChannelTest::testClearComponents_resetsState()
{
    Channel ch("ARGB Header 1", 10);
    ch.addComponent(Component::makeGenericPlaceholder(5));
    ch.clearComponents();
    QCOMPARE(ch.totalLedCount(), 0);
    QVERIFY(ch.colors().empty());
    QVERIFY(!ch.ledPosition(0).has_value());
}

void ChannelTest::testFanChainsAndAtomicReplacement()
{
    Channel channel("JARGB1",120);
    const auto fan=Component::makeFan(6,true);
    QCOMPARE(fan.ledCount,6);
    QVERIFY(channel.replaceComponents({fan,Component::makeFan(6,true,1)}));
    QCOMPARE(channel.totalLedCount(),12);
    QVERIFY(channel.ledPosition(0)!=channel.ledPosition(6));
    auto invalid=fan;invalid.ledCount=121;
    QVERIFY(!channel.replaceComponents({invalid}));
    QCOMPARE(channel.totalLedCount(),12);
    auto malformed=fan;malformed.ledCoordinates.clear();
    QVERIFY(!channel.replaceComponents({malformed}));
    QCOMPARE(channel.totalLedCount(),12);
}

QTEST_MAIN(ChannelTest)
#include "test_channel.moc"

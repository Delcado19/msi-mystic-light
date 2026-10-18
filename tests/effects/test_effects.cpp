// Unit tests for the IEffect implementations (StaticColorEffect,
// RainbowWaveEffect). Pure render-time color computation, no HID/device I/O.
#include <QtTest/QtTest>

#include "effects/RainbowWaveEffect.h"
#include "effects/StaticColorEffect.h"

class EffectsTest : public QObject
{
    Q_OBJECT

private slots:
    void testStaticColorEffect_ignoresInputs();
    void testRainbowWaveEffect_hueZeroIsRed();
    void testRainbowWaveEffect_variesWithTimeAndPosition();
    void testRainbowWaveEffect_hueWrapsWithoutCrashing();
    void testRainbowWaveEffect_hue120IsGreen();
    void testRainbowWaveEffect_hue240IsBlue();
};

void EffectsTest::testStaticColorEffect_ignoresInputs()
{
    StaticColorEffect e(MsiColor{9,8,7});
    MsiColor a = e.colorAt(0,0,0);
    MsiColor b = e.colorAt(5,5,12345);
    QCOMPARE(a.r, static_cast<uint8_t>(9));
    QCOMPARE(b.r, a.r); QCOMPARE(b.g, a.g); QCOMPARE(b.b, a.b);
}

void EffectsTest::testRainbowWaveEffect_hueZeroIsRed()
{
    RainbowWaveEffect e(3000);
    MsiColor c = e.colorAt(0, 0, 0); // hue = 0
    QCOMPARE(c.r, static_cast<uint8_t>(255));
    QCOMPARE(c.g, static_cast<uint8_t>(0));
    QCOMPARE(c.b, static_cast<uint8_t>(0));
}

void EffectsTest::testRainbowWaveEffect_variesWithTimeAndPosition()
{
    RainbowWaveEffect e(3000);
    MsiColor t0 = e.colorAt(0, 0, 0);
    MsiColor t1 = e.colorAt(0, 0, 750); // quarter period -> hue 90
    QVERIFY(t0.r != t1.r || t0.g != t1.g || t0.b != t1.b);
    MsiColor x0 = e.colorAt(0, 0, 0);
    MsiColor x5 = e.colorAt(5, 0, 0); // hue 180 at t=0
    QVERIFY(x0.r != x5.r || x0.g != x5.g || x0.b != x5.b);
}

void EffectsTest::testRainbowWaveEffect_hueWrapsWithoutCrashing()
{
    RainbowWaveEffect e(3000);
    MsiColor c = e.colorAt(100, 0, 1000000); // large inputs, must not throw/crash
    Q_UNUSED(c);
    QVERIFY(true);
}

// Controller-ruling addition (not in the brief's 4 given tests): the brief's
// only exact-RGB assertion is hue=0 (red). Pin two more known HSV(h,1,1)
// angles so a wrong HSV->RGB conversion that still "varies with time and
// position" would still be caught.
void EffectsTest::testRainbowWaveEffect_hue120IsGreen()
{
    RainbowWaveEffect e(3000);
    // x=0, elapsedMs=1000 -> hue = 1000*360/3000 = 120
    MsiColor c = e.colorAt(0, 0, 1000);
    QCOMPARE(c.r, static_cast<uint8_t>(0));
    QCOMPARE(c.g, static_cast<uint8_t>(255));
    QCOMPARE(c.b, static_cast<uint8_t>(0));
}

void EffectsTest::testRainbowWaveEffect_hue240IsBlue()
{
    RainbowWaveEffect e(3000);
    // x=0, elapsedMs=2000 -> hue = 2000*360/3000 = 240
    MsiColor c = e.colorAt(0, 0, 2000);
    QCOMPARE(c.r, static_cast<uint8_t>(0));
    QCOMPARE(c.g, static_cast<uint8_t>(0));
    QCOMPARE(c.b, static_cast<uint8_t>(255));
}

QTEST_MAIN(EffectsTest)
#include "test_effects.moc"

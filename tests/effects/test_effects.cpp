// Unit tests for the IEffect implementations (StaticColorEffect,
// RainbowWaveEffect, RadialRainbowEffect, BreathingEffect). Pure render-time
// color computation, no HID/device I/O.
#include <QtTest/QtTest>

#include "effects/BreathingEffect.h"
#include "effects/RadialRainbowEffect.h"
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
    void testRainbowWaveEffect_zeroOrNegativePeriodDoesNotCrash();
    void testRadialRainbowEffect_sameAngleSameHueRegardlessOfDistance();
    void testRadialRainbowEffect_angle90DiffersFromAngle0();
    void testRadialRainbowEffect_zeroDistanceDoesNotCrash();
    void testRadialRainbowEffect_setReferencePointMovesTheCenter();
    void testRadialRainbowEffect_zeroOrNegativePeriodDoesNotCrash();
    void testRadialRainbowEffect_offsetCenterExactHueAtTwoTimes();
    void testBreathingEffect_quarterPeriodIsFullBrightness();
    void testBreathingEffect_threeQuarterPeriodIsOff();
    void testBreathingEffect_zeroOrNegativePeriodDoesNotCrash();
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

// Review-mandated addition: periodMs_ is a divisor in colorAt, so a caller
// passing 0 or a negative period (reachable from Task 8's config) would be
// integer division by zero (SIGFPE) without the constructor's clamp. Confirms
// the clamp holds for both a zero and a negative period.
void EffectsTest::testRainbowWaveEffect_zeroOrNegativePeriodDoesNotCrash()
{
    RainbowWaveEffect zero(0);
    MsiColor a = zero.colorAt(0, 0, 500);
    Q_UNUSED(a);

    RainbowWaveEffect negative(-3000);
    MsiColor b = negative.colorAt(5, 0, 500);
    Q_UNUSED(b);

    QVERIFY(true);
}

// Due east of center (angle 0deg) at two different distances, at t=0:
// hue depends only on angle, not on distance from the center.
void EffectsTest::testRadialRainbowEffect_sameAngleSameHueRegardlessOfDistance()
{
    RadialRainbowEffect e(0, 0, 3000);
    MsiColor near = e.colorAt(10, 0, 0);
    MsiColor far = e.colorAt(100, 0, 0);
    QCOMPARE(far.r, near.r);
    QCOMPARE(far.g, near.g);
    QCOMPARE(far.b, near.b);
}

// North of center (dx=0, dy=-10) vs. east of center (dx=10, dy=0), both at
// t=0, must differ: atan2(-10,0) and atan2(0,10) are different angles (the
// formula's atan2(y-cy, x-cx) convention puts "north" at 270deg on screen,
// since screen y grows downward -- this test only needs the two to differ,
// not which exact degree each one is).
void EffectsTest::testRadialRainbowEffect_angle90DiffersFromAngle0()
{
    RadialRainbowEffect e(0, 0, 3000);
    MsiColor north = e.colorAt(0, -10, 0);
    MsiColor east = e.colorAt(10, 0, 0);
    QVERIFY(north.r != east.r || north.g != east.g || north.b != east.b);
}

// Review Focus item: atan2(0,0) is well-defined in C++ (returns 0), so
// sampling colorAt exactly at the center must not throw/crash. Pinned here
// rather than just trusted by inspection.
void EffectsTest::testRadialRainbowEffect_zeroDistanceDoesNotCrash()
{
    RadialRainbowEffect e(5, 5, 3000);
    MsiColor c = e.colorAt(5, 5, 0);
    Q_UNUSED(c);
    QVERIFY(true);
}

void EffectsTest::testRadialRainbowEffect_setReferencePointMovesTheCenter()
{
    RadialRainbowEffect moved(0, 0, 3000);
    moved.setReferencePoint(50, 50);
    std::pair<int,int> p = moved.referencePoint();
    QCOMPARE(p.first, 50);
    QCOMPARE(p.second, 50);

    RadialRainbowEffect constructedAtTarget(50, 50, 3000);
    MsiColor a = moved.colorAt(60, 50, 0);
    MsiColor b = constructedAtTarget.colorAt(60, 50, 0);
    QCOMPARE(a.r, b.r);
    QCOMPARE(a.g, b.g);
    QCOMPARE(a.b, b.b);
}

// Same reasoning as RainbowWaveEffect's equivalent test: periodMs_ is a
// divisor in colorAt, so a caller-supplied 0 or negative period would
// otherwise be integer division by zero (SIGFPE) without the constructor's
// clamp.
void EffectsTest::testRadialRainbowEffect_zeroOrNegativePeriodDoesNotCrash()
{
    RadialRainbowEffect zero(0, 0, 0);
    MsiColor a = zero.colorAt(10, 0, 500);
    Q_UNUSED(a);

    RadialRainbowEffect negative(0, 0, -3000);
    MsiColor b = negative.colorAt(10, 0, 500);
    Q_UNUSED(b);

    QVERIFY(true);
}

// Controller-ruling addition (not in the brief's 4 given tests), mirroring
// this file's existing hue120/hue240 additions for RainbowWaveEffect: the
// other RadialRainbowEffect tests above all use a (0,0) center and/or
// compare samples at t=0, so a center that's silently ignored, a dropped
// elapsedMs term, or swapped atan2(y,x) arguments would still pass them.
// Pin two exact RGB values against an *offset* center at two different
// times to catch all three: (60,50) is due east of center (50,50) (angle
// 0deg), giving hue 0 (red) at t=0 and hue 120 (green) at t=1000 (a third
// of a 3000ms period).
void EffectsTest::testRadialRainbowEffect_offsetCenterExactHueAtTwoTimes()
{
    RadialRainbowEffect e(50, 50, 3000);
    MsiColor t0 = e.colorAt(60, 50, 0);
    QCOMPARE(t0.r, static_cast<uint8_t>(255));
    QCOMPARE(t0.g, static_cast<uint8_t>(0));
    QCOMPARE(t0.b, static_cast<uint8_t>(0));

    MsiColor t1 = e.colorAt(60, 50, 1000);
    QCOMPARE(t1.r, static_cast<uint8_t>(0));
    QCOMPARE(t1.g, static_cast<uint8_t>(255));
    QCOMPARE(t1.b, static_cast<uint8_t>(0));
}

// brightness(t) = (sin(2*pi*t/period) + 1) / 2: a quarter period is the sine
// peak (full brightness), so the configured color comes back unchanged.
void EffectsTest::testBreathingEffect_quarterPeriodIsFullBrightness()
{
    BreathingEffect e(MsiColor{200, 100, 50}, 4000);
    MsiColor c = e.colorAt(0, 0, 1000);
    QCOMPARE(c.r, static_cast<uint8_t>(200));
    QCOMPARE(c.g, static_cast<uint8_t>(100));
    QCOMPARE(c.b, static_cast<uint8_t>(50));
}

// Three quarters of a period is the sine trough (brightness 0), so the output
// is black.
void EffectsTest::testBreathingEffect_threeQuarterPeriodIsOff()
{
    BreathingEffect e(MsiColor{200, 100, 50}, 4000);
    MsiColor c = e.colorAt(0, 0, 3000);
    QCOMPARE(c.r, static_cast<uint8_t>(0));
    QCOMPARE(c.g, static_cast<uint8_t>(0));
    QCOMPARE(c.b, static_cast<uint8_t>(0));
}

// Same divisor-clamp reasoning as the RainbowWaveEffect test above.
void EffectsTest::testBreathingEffect_zeroOrNegativePeriodDoesNotCrash()
{
    BreathingEffect zero(MsiColor{200, 100, 50}, 0);
    MsiColor a = zero.colorAt(0, 0, 500);
    Q_UNUSED(a);

    BreathingEffect negative(MsiColor{200, 100, 50}, -3000);
    MsiColor b = negative.colorAt(0, 0, 500);
    Q_UNUSED(b);

    QVERIFY(true);
}

QTEST_MAIN(EffectsTest)
#include "test_effects.moc"

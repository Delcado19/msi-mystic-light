// Unit tests for the IEffect implementations (StaticColorEffect,
// RainbowWaveEffect, RadialRainbowEffect, BreathingEffect). Pure render-time
// color computation, no HID/device I/O.
#include <QtTest/QtTest>

#include "effects/CustomGradientEffect.h"
#include "effects/EffectFactory.h"
#include "effects/BreathingEffect.h"
#include "effects/CometEffect.h"
#include "effects/MarqueeEffect.h"
#include "effects/RadialRainbowEffect.h"
#include "effects/RainbowWaveEffect.h"
#include "effects/SpiralEffect.h"
#include "effects/StaticColorEffect.h"
#include "effects/SwirlCirclesEffect.h"

class EffectsTest : public QObject
{
    Q_OBJECT

private slots:
    void testSmoothWaveAndCustomPalette();
    void testLayersZonesAndScroll();
    void testCircularCentersTranslate();
    void testBaseEffectsVisibleAtStart();
    void testSourceMotionModes();
    void testBreathingDoesNotScrollAndPatternsDiffer();
    void testCustomPatternsAndStops();
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
    void testSpiralEffect_zeroDistanceDoesNotCrashAndIsFinite();
    void testSpiralEffect_sameAngleDifferentDistanceDiffersInHue();
    void testSpiralEffect_setReferencePointMovesTheCenter();
    void testCometEffect_headPositionIsBrightest();
    void testCometEffect_farFromHeadIsDark();
    void testCometEffect_tinyTrackOrTailDoesNotCrash();
    void testMarqueeEffect_onInsideDashOffInsideGap();
    void testMarqueeEffect_scrollsWithTime();
    void testSwirlCirclesEffect_sameRingSameHue();
    void testSwirlCirclesEffect_adjacentRingsDiffer();
    void testSwirlCirclesEffect_zeroRingWidthDoesNotCrash();
};

void EffectsTest::testBreathingDoesNotScrollAndPatternsDiffer()
{
    CustomEffectDefinition d{"Breath",{{255,0,0},{0,0,255}},6000,720};d.pattern="Breathing";d.path="Canvas positions";
    const CustomGradientEffect effect(d);const auto a=effect.colorAt(180,0,1500),b=effect.colorAt(180,0,3000);
    QCOMPARE(a.r,effect.colorAt(1000,600,1500).r);QCOMPARE(a.b,effect.colorAt(1000,600,1500).b);
    QVERIFY(a.r!=b.r || a.b!=b.b);QCOMPARE(effect.colorAt(180,0,0).r,uint8_t(255));
    d.path="Component LEDs";const CustomGradientEffect leds(d);const auto l=leds.colorForLed(0,0,1500,3,12,3,12),m=leds.colorForLed(600,400,1500,9,12,9,12);QCOMPARE(l.r,m.r);QCOMPARE(l.b,m.b);
    d.colors={{255,255,255},{255,255,255}};d.path="Canvas positions";d.pattern="Comet";const auto comet=CustomGradientEffect(d).colorAt(90,0,0);
    d.pattern="Marquee";const auto marquee=CustomGradientEffect(d).colorAt(90,0,0);QVERIFY(comet.r!=marquee.r);
    d.pattern="Vertical";d.normalizeScroll();QCOMPARE(d.pattern,std::string("Scroll"));QCOMPARE(d.angle,270.);
}

void EffectsTest::testSourceMotionModes()
{
    CustomEffectDefinition d{"Motion",{{255,0,0},{0,0,0}},6000,300};d.pattern="Color source";
    const auto x=[&](int time){return CustomGradientEffect(d).sourceCenterAt(time).first;};
    QVERIFY(std::abs(x(0)-550)<.001);QVERIFY(std::abs(x(1500)-935)<.001);
    d.sourceMotion="Cosine";QVERIFY(std::abs(x(0)-935)<.001);QVERIFY(std::abs(x(3000)-165)<.001);
    d.sourceMotion="Ping-pong";QVERIFY(std::abs(x(0)-165)<.001);QVERIFY(std::abs(x(1500)-550)<.001);QVERIFY(std::abs(x(3000)-935)<.001);
    d.sourceMotion="Static";QCOMPARE(x(0),x(1500));d.sourceMotion="Sine";d.angle=90;const auto point=CustomGradientEffect(d).sourceCenterAt(1500);QVERIFY(std::abs(point.first-550)<.001);QVERIFY(std::abs(point.second-595)<.001);
}

void EffectsTest::testBaseEffectsVisibleAtStart()
{
    for(const auto* name:{"StaticColor","Scroll","Breathing","RadialRainbow","Spiral","Comet","Marquee","SwirlCircles","Plasma","Color source","Static gradient"}){
        const auto effect=buildEffect(name,{255,255,255},550,350);QVERIFY(effect);int maximum=0;
        for(int y=0;y<700;y+=70)for(int x=0;x<1100;x+=55){const auto c=effect->colorAt(x,y,0);maximum=std::max(maximum,int(c.r)+c.g+c.b);}QVERIFY2(maximum>0,name);
    }
    CustomEffectDefinition comet{"Head",{{255,255,255},{255,255,255}},6000,720};comet.pattern="Comet";QVERIFY(CustomGradientEffect(comet).colorAt(0,0,0).r>0);
}

void EffectsTest::testCircularCentersTranslate()
{
    CustomEffectDefinition namedSpiral{"Spiral",{{255,0,0},{0,0,255}},6000,720};namedSpiral.pattern="Scroll";
    QVERIFY(dynamic_cast<SpiralEffect*>(buildEffect("Spiral",{},0,0,{namedSpiral}).get()));QVERIFY(dynamic_cast<CustomGradientEffect*>(buildEffect("custom:Spiral",{},0,0,{namedSpiral}).get()));
    for(const auto* pattern:{"Radial","Angular","Spiral"}){
        CustomEffectDefinition d{"Center",{{255,0,0},{0,0,255}},6000,720};d.pattern=pattern;
        const auto original=CustomGradientEffect(d).colorAt(610,390,1500);d.centerX+=100;d.centerY+=50;
        const auto moved=CustomGradientEffect(d).colorAt(710,440,1500);QCOMPARE(original.r,moved.r);QCOMPARE(original.b,moved.b);
    }
}

void EffectsTest::testLayersZonesAndScroll()
{
    CustomEffectDefinition invalid;invalid.opacity=2;QVERIFY(!invalid.validLayerSettings());invalid.opacity=1;invalid.layers.resize(9);QVERIFY(!invalid.validLayerSettings());
    CustomEffectDefinition d{"Red",{{255,0,0},{255,0,0}},6000,720};d.path="Canvas positions";
    CustomEffectDefinition blue{"Blue",{{0,0,255},{0,0,255}},6000,720};blue.zoneEnabled=true;blue.zoneX=100;blue.zoneY=100;blue.zoneWidth=200;blue.zoneHeight=200;
    d.layers={blue};auto mixed=CustomGradientEffect(d).colorAt(150,150,0);QCOMPARE(mixed.r,uint8_t(255));QCOMPARE(mixed.b,uint8_t(255));
    QCOMPARE(CustomGradientEffect(d).colorAt(300,150,0).b,uint8_t(0));
    d.zoneEnabled=true;d.zoneX=0;d.zoneWidth=50;
    const auto overlayOnly=CustomGradientEffect(d).colorAt(150,150,0);QCOMPARE(overlayOnly.r,uint8_t(0));QCOMPARE(overlayOnly.b,uint8_t(255));d.zoneEnabled=false;
    d.layers[0].blend="Normal";d.layers[0].opacity=.5;QCOMPARE(CustomGradientEffect(d).colorAt(150,150,0).r,uint8_t(128));
    d.layers.clear();d.pattern="Scroll";d.colors={{255,0,0},{0,0,255}};d.angle=90;
    const auto down=CustomGradientEffect(d).colorAt(0,180,0);const auto same=CustomGradientEffect(d).colorAt(200,180,0);QCOMPARE(down.r,same.r);
    d.angle=0;const auto right=CustomGradientEffect(d).colorAt(180,0,0);QCOMPARE(right.r,down.r);
    d.pattern="Color source";d.colors={{255,0,0},{255,0,0}};d.wavelength=200;d.layers={d};d.layers[0].layers.clear();d.layers[0].colors={{0,0,255},{0,0,255}};
    mixed=CustomGradientEffect(d).colorAt(550,350,0);QCOMPARE(mixed.r,uint8_t(255));QCOMPARE(mixed.b,uint8_t(255));
}

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

// Review Focus item: distance 0 contributes 0 to the hue, so sampling exactly
// at the center must give a deterministic color (no NaN from atan2(0,0) or
// the distance term), repeatable across calls.
void EffectsTest::testSpiralEffect_zeroDistanceDoesNotCrashAndIsFinite()
{
    SpiralEffect e(5, 5, 4.0, 3000);
    MsiColor a = e.colorAt(5, 5, 0);
    MsiColor b = e.colorAt(5, 5, 0);
    QCOMPARE(a.r, b.r);
    QCOMPARE(a.g, b.g);
    QCOMPARE(a.b, b.b);
}

// Due east (same angle 0deg) at distance 10 vs. 100. Tightness is 1.0 so the
// hue offsets are 10deg vs. 100deg; with the old tightness 4.0 they would be 40deg
// vs. 400 mod 360 = 40deg and collide, which would make this test meaningless.
// Unlike RadialRainbowEffect, distance must change the hue.
void EffectsTest::testSpiralEffect_sameAngleDifferentDistanceDiffersInHue()
{
    SpiralEffect e(0, 0, 1.0, 3000);
    MsiColor near = e.colorAt(10, 0, 0);
    MsiColor far = e.colorAt(100, 0, 0);
    QVERIFY(near.r != far.r || near.g != far.g || near.b != far.b);
}

// Mirrors testRadialRainbowEffect_setReferencePointMovesTheCenter.
void EffectsTest::testSpiralEffect_setReferencePointMovesTheCenter()
{
    SpiralEffect moved(0, 0, 4.0, 3000);
    moved.setReferencePoint(50, 50);
    std::pair<int,int> p = moved.referencePoint();
    QCOMPARE(p.first, 50);
    QCOMPARE(p.second, 50);

    SpiralEffect constructedAtTarget(50, 50, 4.0, 3000);
    MsiColor a = moved.colorAt(60, 50, 0);
    MsiColor b = constructedAtTarget.colorAt(60, 50, 0);
    QCOMPARE(a.r, b.r);
    QCOMPARE(a.g, b.g);
    QCOMPARE(a.b, b.b);
}

// headX(0) = 0, so x=0 sits exactly on the head: brightness 1.0, color unchanged.
void EffectsTest::testCometEffect_headPositionIsBrightest()
{
    CometEffect e(MsiColor{200, 100, 50}, 100, 0.05, 20);
    MsiColor c = e.colorAt(0, 0, 0);
    QCOMPARE(c.r, static_cast<uint8_t>(200));
    QCOMPARE(c.g, static_cast<uint8_t>(100));
    QCOMPARE(c.b, static_cast<uint8_t>(50));
}

// Half a 100px track away from the head at x=0 is x=50, well beyond the 20px
// tail, so brightness clamps to 0.
void EffectsTest::testCometEffect_farFromHeadIsDark()
{
    CometEffect e(MsiColor{200, 100, 50}, 100, 0.05, 20);
    MsiColor c = e.colorAt(50, 0, 0);
    QCOMPARE(c.r, static_cast<uint8_t>(0));
    QCOMPARE(c.g, static_cast<uint8_t>(0));
    QCOMPARE(c.b, static_cast<uint8_t>(0));
}

// Zero track width and zero tail would divide by zero without the clamps.
void EffectsTest::testCometEffect_tinyTrackOrTailDoesNotCrash()
{
    CometEffect zeroTrack(MsiColor{200, 100, 50}, 0, 0.05, 20);
    MsiColor a = zeroTrack.colorAt(0, 0, 0);
    Q_UNUSED(a);

    CometEffect zeroTail(MsiColor{200, 100, 50}, 100, 0.05, 0);
    MsiColor b = zeroTail.colorAt(0, 0, 0);
    Q_UNUSED(b);

    QVERIFY(true);
}

// Dash 5 / gap 5 gives period 10. x=2 is inside the dash (on), x=7 inside the
// gap (off). At elapsedMs=0 the pattern is unshifted.
void EffectsTest::testMarqueeEffect_onInsideDashOffInsideGap()
{
    MarqueeEffect e(MsiColor{200, 100, 50}, 5, 5, 0.05);
    MsiColor on = e.colorAt(2, 0, 0);
    QCOMPARE(on.r, static_cast<uint8_t>(200));
    QCOMPARE(on.g, static_cast<uint8_t>(100));
    QCOMPARE(on.b, static_cast<uint8_t>(50));

    MsiColor off = e.colorAt(7, 0, 0);
    QCOMPARE(off.r, static_cast<uint8_t>(0));
    QCOMPARE(off.g, static_cast<uint8_t>(0));
    QCOMPARE(off.b, static_cast<uint8_t>(0));
}

// At speed 0.05 px/ms, elapsedMs=100 shifts the pattern by 5px (half of the
// period 10). x=2 goes from dash (t=0) to gap (t=100): pos = (2 - 5) mod 10 = 7.
void EffectsTest::testMarqueeEffect_scrollsWithTime()
{
    MarqueeEffect e(MsiColor{200, 100, 50}, 5, 5, 0.05);
    MsiColor atStart = e.colorAt(2, 0, 0);
    MsiColor halfPeriodLater = e.colorAt(2, 0, 100);
    QVERIFY(atStart.r != halfPeriodLater.r || atStart.g != halfPeriodLater.g || atStart.b != halfPeriodLater.b);
    QCOMPARE(halfPeriodLater.r, static_cast<uint8_t>(0));
    QCOMPARE(halfPeriodLater.g, static_cast<uint8_t>(0));
    QCOMPARE(halfPeriodLater.b, static_cast<uint8_t>(0));
}

// Equal radii match at any angle; neighboring radii now change continuously.
void EffectsTest::testSwirlCirclesEffect_sameRingSameHue()
{
    SwirlCirclesEffect e(0,0,15,3000);
    const auto a=e.colorAt(10,0,500),b=e.colorAt(0,10,500);
    QCOMPARE(a.r,b.r);QCOMPARE(a.g,b.g);QCOMPARE(a.b,b.b);
    const auto c=e.colorAt(14,0,0),d=e.colorAt(15,0,0);
    QVERIFY(qAbs(int(c.r)-d.r)<=18 && qAbs(int(c.g)-d.g)<=18 && qAbs(int(c.b)-d.b)<=18);
}

void EffectsTest::testSwirlCirclesEffect_adjacentRingsDiffer()
{
    SwirlCirclesEffect e(0, 0, 15, 3000);
    MsiColor inner = e.colorAt(14, 0, 0);
    MsiColor outer = e.colorAt(15, 0, 0);
    QVERIFY(inner.r != outer.r || inner.g != outer.g || inner.b != outer.b);
}

// ringWidthPx is clamped to >=1, so 0 behaves exactly like 1 (ring = distance)
// and must not divide by zero.
void EffectsTest::testSwirlCirclesEffect_zeroRingWidthDoesNotCrash()
{
    SwirlCirclesEffect zero(0, 0, 0, 3000);
    SwirlCirclesEffect one(0, 0, 1, 3000);
    MsiColor a = zero.colorAt(3, 0, 0);
    MsiColor b = one.colorAt(3, 0, 0);
    QCOMPARE(a.r, b.r);
    QCOMPARE(a.g, b.g);
    QCOMPARE(a.b, b.b);
}

void EffectsTest::testSmoothWaveAndCustomPalette()
{
    RainbowWaveEffect wave;
    const auto a=wave.colorAt(0,0,0),b=wave.colorAt(1,0,0);
    QVERIFY(qAbs(int(a.r)-b.r)<=4 && qAbs(int(a.g)-b.g)<=4 && qAbs(int(a.b)-b.b)<=4);
    for(const auto& effect:std::vector<std::shared_ptr<IEffect>>{std::make_shared<SpiralEffect>(),std::make_shared<SwirlCirclesEffect>()}) {
        const auto left=effect->colorAt(100,0,0),right=effect->colorAt(101,0,0);
        QVERIFY(qAbs(int(left.r)-right.r)<=4 && qAbs(int(left.g)-right.g)<=4 && qAbs(int(left.b)-right.b)<=4);
    }
    CustomGradientEffect gradient({"Test",{{255,0,0},{0,0,255}},6000,720});
    auto middle=gradient.colorAt(180,0,0);
    QCOMPARE(middle.r,uint8_t(128));QCOMPARE(middle.b,uint8_t(128));
    auto wrap=gradient.colorAt(-720,0,0);
    QCOMPARE(wrap.r,uint8_t(255));QCOMPARE(wrap.b,uint8_t(0));
    CustomGradientEffect invalid({"",{},0,0});QCOMPARE(invalid.colorAt(0,0,0).r,uint8_t(0));
}

void EffectsTest::testCustomPatternsAndStops()
{
    for(const auto* pattern:{"Wave","Vertical","Diagonal","Radial","Spiral","Plasma","Breathing","Sweep","Static gradient"}) {
        CustomEffectDefinition d{"Pattern",{{255,0,0},{0,0,255}},6000,720};d.pattern=pattern;d.stops={0,.25};
        QVERIFY(CustomEffectDefinition::validPattern(pattern));QVERIFY(d.validStops());CustomGradientEffect effect(d);
        const auto start=effect.colorAt(100,150,1234),repeat=effect.colorAt(100,150,7234);
        QCOMPARE(start.r,repeat.r);QCOMPARE(start.g,repeat.g);QCOMPARE(start.b,repeat.b);
        const auto later=effect.colorAt(100,150,2734);
        if(d.pattern=="Static gradient"){QCOMPARE(start.r,later.r);QCOMPARE(start.b,later.b);}
        else QVERIFY(start.r!=later.r || start.b!=later.b);
    }
    CustomEffectDefinition d{"Stops",{{255,0,0},{0,0,255}},6000,720};d.stops={0,.25};
    const auto blue=CustomGradientEffect(d).colorAt(180,0,0);QCOMPARE(blue.b,uint8_t(255));QCOMPARE(blue.r,uint8_t(0));
    d.stops={.7,.2};QVERIFY(!d.validStops());d.stops={0,1};QVERIFY(!d.validStops());d.stops={0};QVERIFY(!d.validStops());
    QVERIFY(!CustomEffectDefinition::validPattern("Untrusted"));
    d.stops={0,.5};d.path="Component LEDs";CustomGradientEffect ledEffect(d);
    QCOMPARE(ledEffect.colorForLed(0,0,0,0,12,0,6).r,uint8_t(255));
    QCOMPARE(ledEffect.colorForLed(0,0,0,3,12,3,6).b,uint8_t(255));
    QCOMPARE(ledEffect.colorForLed(0,0,0,6,12,0,6).r,uint8_t(255));
    d.path="Entire chain";CustomGradientEffect chainEffect(d);
    QCOMPARE(chainEffect.colorForLed(0,0,0,6,12,0,6).b,uint8_t(255));
    d.path="Canvas positions";CustomGradientEffect spatial(d);
    QCOMPARE(spatial.colorForLed(180,0,0,6,12,0,6).b,spatial.colorAt(180,0,0).b);
}

QTEST_MAIN(EffectsTest)
#include "test_effects.moc"

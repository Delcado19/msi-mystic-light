#include "modules/ModuleRegistry.h"
#include "hid/FakeHidDevice.h"
#include <QtTest>

namespace {
DeviceProfile profile() { return DeviceProfile::load(TEST_PROFILE_PATH); }
DeviceIdentity identity()
{
    DeviceIdentity device;
    device.path = "fake-hid-path";
    device.vendorId = 9934;
    device.productId = 418;
    return device;
}
}

class AsrockPolychromeTest : public QObject {
    Q_OBJECT
private slots:
    void acceptsProfile_rejectsBadInput()
    {
        ModuleRegistry modules;
        QString error;
        QVERIFY2(modules.load(TEST_MODULE_PATH, &error), qPrintable(error));

        auto good = profile();
        QVERIFY(modules.acceptsProfile(good, &error));

        auto wrongVendor = good; wrongVendor.match["vendorId"] = 1;
        QVERIFY(!modules.acceptsProfile(wrongVendor, &error));

        auto wrongDriver = good; wrongDriver.driverId = "other.driver";
        QVERIFY(!modules.acceptsProfile(wrongDriver, &error));

        auto duplicateZone = good; duplicateZone.channels[1].address["zone"] = 0;
        QVERIFY(!modules.acceptsProfile(duplicateZone, &error));

        auto zoneOutOfRange = good; zoneOutOfRange.channels[0].address["zone"] = 8;
        QVERIFY(!modules.acceptsProfile(zoneOutOfRange, &error));

        auto directChannel = good; directChannel.channels[0].direct = true;
        QVERIFY(!modules.acceptsProfile(directChannel, &error));

        auto unknownMode = good; unknownMode.channels[0].nativeModes = {"disco"};
        QVERIFY(!modules.acceptsProfile(unknownMode, &error));

        auto noModes = good; noModes.channels[0].nativeModes.clear();
        QVERIFY(!modules.acceptsProfile(noModes, &error));
    }

    void setNativeMode_producesExpectedReportAndDedupes()
    {
        ModuleRegistry modules;
        QString error;
        QVERIFY2(modules.load(TEST_MODULE_PATH, &error), qPrintable(error));

        FakeHidDevice transport;
        QVERIFY(transport.open(9934, 418));
        auto driver = modules.create(identity(), profile(), transport, &error);
        QVERIFY2(driver, qPrintable(error));

        NativeEffect effect;
        effect.id = "static";
        effect.colors = {{0x11, 0x22, 0x33}};
        effect.speed = 1;

        // zone 0 = RGB1. Byte layout independently derived from the protocol
        // doc (not copied from OpenRGB): [0]=0x00 report id, [1]=0x10 set-zone,
        // [3]=zone 0, [4]=mode Static(0x01), [5..7]=G,R,B (non-swapped) =
        // 0x22,0x11,0x33, [8]=speed 0xE0 (default), [9]=0xFF.
        QVERIFY(driver->driver().setNativeMode("RGB1", effect));
        QCOMPARE(transport.sentReports.size(), std::size_t{1});
        const auto& report = transport.sentReports[0];
        QCOMPARE(report.size(), std::size_t{65});
        QCOMPARE(report[0], std::uint8_t{0x00});
        QCOMPARE(report[1], std::uint8_t{0x10});
        QCOMPARE(report[3], std::uint8_t{0x00});
        QCOMPARE(report[4], std::uint8_t{0x01});
        QCOMPARE(report[5], std::uint8_t{0x22});
        QCOMPARE(report[6], std::uint8_t{0x11});
        QCOMPARE(report[7], std::uint8_t{0x33});
        QCOMPARE(report[8], std::uint8_t{0xE0});
        QCOMPARE(report[9], std::uint8_t{0xFF});
        QCOMPARE(transport.readInputReportCalls, 1); // Drained, upstream also never inspects it.

        // Identical state must not be resent (flash wears on every write per
        // upstream's own "@save" annotation).
        QVERIFY(driver->driver().setNativeMode("RGB1", effect));
        QCOMPARE(transport.sentReports.size(), std::size_t{1});

        // A real change does send again.
        effect.colors = {{0, 0, 0}};
        QVERIFY(driver->driver().setNativeMode("RGB1", effect));
        QCOMPARE(transport.sentReports.size(), std::size_t{2});
    }

    void setNativeMode_rejectsInvalidEffectsAndClosedDevice()
    {
        ModuleRegistry modules;
        QString error;
        QVERIFY2(modules.load(TEST_MODULE_PATH, &error), qPrintable(error));

        FakeHidDevice transport;
        QVERIFY(transport.open(9934, 418));
        auto driver = modules.create(identity(), profile(), transport, &error);
        QVERIFY2(driver, qPrintable(error));

        NativeEffect effect;
        effect.id = "unknown-mode";
        QVERIFY(!driver->driver().setNativeMode("RGB1", effect));

        effect.id = "static";
        effect.colors.clear();
        QVERIFY(!driver->driver().setNativeMode("RGB1", effect));

        effect.colors = {{1, 2, 3}, {4, 5, 6}};
        QVERIFY(!driver->driver().setNativeMode("RGB1", effect));

        effect.colors = {{1, 2, 3}};
        effect.speed = 3;
        QVERIFY(!driver->driver().setNativeMode("RGB1", effect));
        effect.speed = -1;
        QVERIFY(!driver->driver().setNativeMode("RGB1", effect));

        effect.speed = 0;
        QVERIFY(!driver->driver().setNativeMode("NoSuchChannel", effect));
        QCOMPARE(transport.sentReports.size(), std::size_t{0});

        transport.close();
        QVERIFY(!driver->driver().setNativeMode("RGB1", effect));
    }

    void pushColors_alwaysUnsupported()
    {
        ModuleRegistry modules;
        QString error;
        QVERIFY2(modules.load(TEST_MODULE_PATH, &error), qPrintable(error));

        FakeHidDevice transport;
        QVERIFY(transport.open(9934, 418));
        auto driver = modules.create(identity(), profile(), transport, &error);
        QVERIFY2(driver, qPrintable(error));

        QVERIFY(!driver->driver().pushColors("RGB1", {{1, 2, 3}}));
        QCOMPARE(transport.sentReports.size(), std::size_t{0});
    }
};

QTEST_GUILESS_MAIN(AsrockPolychromeTest)
#include "test_asrock_polychrome.moc"

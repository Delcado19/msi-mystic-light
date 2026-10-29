#include "modules/ModuleRegistry.h"
#include "hid/FakeHidDevice.h"
#include <QtTest>

namespace {
DeviceProfile profile() { return DeviceProfile::load(TEST_PROFILE_PATH); }
DeviceIdentity identity()
{
    DeviceIdentity device;
    device.path = "fake-hid-path";
    device.vendorId = 6127;
    device.productId = 51541;
    return device;
}
}

class LenovoMotherboardTest : public QObject {
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

        auto duplicateZone = good; duplicateZone.channels[1].address["zone"] = 18;
        QVERIFY(!modules.acceptsProfile(duplicateZone, &error));

        auto badZone = good; badZone.channels[0].address["zone"] = 0x13;
        QVERIFY(!modules.acceptsProfile(badZone, &error));

        auto directChannel = good; directChannel.channels[0].direct = true;
        QVERIFY(!modules.acceptsProfile(directChannel, &error));

        auto unknownMode = good; unknownMode.channels[0].nativeModes = {"disco"};
        QVERIFY(!modules.acceptsProfile(unknownMode, &error));
    }

    void setNativeMode_producesExpectedReports()
    {
        ModuleRegistry modules;
        QString error;
        QVERIFY2(modules.load(TEST_MODULE_PATH, &error), qPrintable(error));

        FakeHidDevice transport;
        QVERIFY(transport.open(6127, 51541));
        auto driver = modules.create(identity(), profile(), transport, &error);
        QVERIFY2(driver, qPrintable(error));

        NativeEffect effect;
        effect.id = "breathing";
        effect.colors = {{0x44, 0x55, 0x66}};
        effect.speed = 1;
        effect.brightness = 5;

        // CpuFan = zone 0x12. Byte layout independently derived from the
        // protocol doc (not copied from OpenRGB): report 1: [0]=0xCC,
        // [1]=zone 0x12, [2]=mode Breathing(0x03), [3]=speed (clamp(1+1,1,4)=2),
        // [4]=brightness (clamp(5,1,4)=4), [5..7]=R,G,B.
        // report 2: [0]=0xCC,[1]=0x28,[2]=0x06,[33..39]= same fields.
        QVERIFY(driver->driver().setNativeMode("CpuFan", effect));
        QCOMPARE(transport.sentReports.size(), std::size_t{2});
        const auto& first = transport.sentReports[0];
        const auto& second = transport.sentReports[1];
        QCOMPARE(first.size(), std::size_t{64});
        QCOMPARE(first[0], std::uint8_t{0xCC});
        QCOMPARE(first[1], std::uint8_t{0x12});
        QCOMPARE(first[2], std::uint8_t{0x03});
        QCOMPARE(first[3], std::uint8_t{2});
        QCOMPARE(first[4], std::uint8_t{4});
        QCOMPARE(first[5], std::uint8_t{0x44});
        QCOMPARE(first[6], std::uint8_t{0x55});
        QCOMPARE(first[7], std::uint8_t{0x66});

        QCOMPARE(second.size(), std::size_t{64});
        QCOMPARE(second[0], std::uint8_t{0xCC});
        QCOMPARE(second[1], std::uint8_t{0x28});
        QCOMPARE(second[2], std::uint8_t{0x06});
        QCOMPARE(second[33], std::uint8_t{0x12});
        QCOMPARE(second[34], std::uint8_t{0x03});
        QCOMPARE(second[35], std::uint8_t{2});
        QCOMPARE(second[36], std::uint8_t{4});
        QCOMPARE(second[37], std::uint8_t{0x44});
        QCOMPARE(second[38], std::uint8_t{0x55});
        QCOMPARE(second[39], std::uint8_t{0x66});
    }

    void setNativeMode_rejectsInvalidEffectsAndClosedDevice()
    {
        ModuleRegistry modules;
        QString error;
        QVERIFY2(modules.load(TEST_MODULE_PATH, &error), qPrintable(error));

        FakeHidDevice transport;
        QVERIFY(transport.open(6127, 51541));
        auto driver = modules.create(identity(), profile(), transport, &error);
        QVERIFY2(driver, qPrintable(error));

        NativeEffect effect;
        effect.id = "unknown-mode";
        QVERIFY(!driver->driver().setNativeMode("CpuFan", effect));

        effect.id = "static";
        effect.colors.clear();
        QVERIFY(!driver->driver().setNativeMode("CpuFan", effect));

        effect.colors = {{1, 2, 3}};
        effect.speed = 3;
        QVERIFY(!driver->driver().setNativeMode("CpuFan", effect));
        effect.speed = 0;
        effect.brightness = 6;
        QVERIFY(!driver->driver().setNativeMode("CpuFan", effect));

        effect.brightness = 5;
        QVERIFY(!driver->driver().setNativeMode("NoSuchChannel", effect));
        QCOMPARE(transport.sentReports.size(), std::size_t{0});

        transport.close();
        QVERIFY(!driver->driver().setNativeMode("CpuFan", effect));
    }

    void pushColors_alwaysUnsupported()
    {
        ModuleRegistry modules;
        QString error;
        QVERIFY2(modules.load(TEST_MODULE_PATH, &error), qPrintable(error));

        FakeHidDevice transport;
        QVERIFY(transport.open(6127, 51541));
        auto driver = modules.create(identity(), profile(), transport, &error);
        QVERIFY2(driver, qPrintable(error));

        QVERIFY(!driver->driver().pushColors("CpuFan", {{1, 2, 3}}));
        QCOMPARE(transport.sentReports.size(), std::size_t{0});
    }
};

QTEST_GUILESS_MAIN(LenovoMotherboardTest)
#include "test_lenovo_motherboard.moc"

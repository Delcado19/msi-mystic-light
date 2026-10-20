// Unit tests for FakeHidDevice itself (Task 2): pins the IHidDevice
// interface's new interface-aware open() and output-report write() on the
// test double, so real-driver tests (e.g. test_msi_motherboard_driver) can
// rely on FakeHidDevice correctly tracking what a peripheral driver asked
// for, independently of HidApiDevice/real hidapi behavior.
#include <QtTest/QtTest>

#include "hid/FakeHidDevice.h"

class FakeHidDeviceTest : public QObject
{
    Q_OBJECT

private slots:
    void testOpen_defaultInterfaceNumberIsNegativeOne();
    void testOpen_capturesExplicitInterfaceNumber();
    void testOpen_defaultUsagePageIsNegativeOne();
    void testOpen_capturesExplicitUsagePage();
    void testWrite_appendsToWrittenReportsIndependentlyOfSentReports();
};

void FakeHidDeviceTest::testOpen_defaultInterfaceNumberIsNegativeOne()
{
    FakeHidDevice hid;
    QVERIFY(hid.open(0x1234, 0x5678));
    QCOMPARE(hid.openInterfaceNumber, -1);
}

void FakeHidDeviceTest::testOpen_capturesExplicitInterfaceNumber()
{
    FakeHidDevice hid;
    QVERIFY(hid.open(0x1234, 0x5678, 2));
    QCOMPARE(hid.openInterfaceNumber, 2);
}

void FakeHidDeviceTest::testOpen_defaultUsagePageIsNegativeOne()
{
    FakeHidDevice hid;
    QVERIFY(hid.open(0x1234, 0x5678));
    QCOMPARE(hid.openUsagePage, -1);
}

// Mirrors testOpen_capturesExplicitInterfaceNumber: pins that the fake
// records usagePage independently, needed to distinguish HID collections
// that share one interface_number (e.g. Roccat Kone Aimo interface 0,
// disambiguated only by usage_page 0x0B).
void FakeHidDeviceTest::testOpen_capturesExplicitUsagePage()
{
    FakeHidDevice hid;
    QVERIFY(hid.open(0x1234, 0x5678, 0, 0x0B));
    QCOMPARE(hid.openUsagePage, 0x0B);
}

void FakeHidDeviceTest::testWrite_appendsToWrittenReportsIndependentlyOfSentReports()
{
    FakeHidDevice hid;
    hid.open(0x1234, 0x5678);

    const uint8_t featureReport[] = {0x01, 0xAA};
    QVERIFY(hid.sendFeatureReport(featureReport, sizeof(featureReport)));

    const uint8_t outputReport[] = {0x02, 0xBB, 0xCC};
    QVERIFY(hid.write(outputReport, sizeof(outputReport)));

    QCOMPARE(static_cast<int>(hid.sentReports.size()), 1);
    QCOMPARE(static_cast<int>(hid.writtenReports.size()), 1);
    QCOMPARE(hid.writtenReports[0], std::vector<uint8_t>({0x02, 0xBB, 0xCC}));
}

QTEST_MAIN(FakeHidDeviceTest)
#include "test_fake_hid_device.moc"

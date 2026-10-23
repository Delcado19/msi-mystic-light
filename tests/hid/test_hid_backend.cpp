#include "hid/HidApiDevice.h"
#include "FakeHidDevice.h"
#include <hidapi.h>
#include <QtTest>
#include <limits>
#include <type_traits>

// Link-time HIDAPI doubles: these tests cannot enumerate or open real devices.
struct hid_device_ {};
namespace {
hid_device_ token;
int initCalls = 0, exitCalls = 0, openCalls = 0, closeCalls = 0, sendCalls = 0;
bool openSucceeds = true;
int sendResult = -2;
unsigned short lastVendor = 0, lastProduct = 0;
std::vector<unsigned char> lastReport;
}

extern "C" {
int HID_API_CALL hid_init()
{
    ++initCalls;
    return qEnvironmentVariableIsSet("HID_TEST_INIT_FAIL") ? -1 : 0;
}
int HID_API_CALL hid_exit() { ++exitCalls; return 0; }
hid_device* HID_API_CALL hid_open(unsigned short vendor, unsigned short product, const wchar_t*)
{
    ++openCalls;
    lastVendor = vendor;
    lastProduct = product;
    return openSucceeds ? &token : nullptr;
}
void HID_API_CALL hid_close(hid_device*) { ++closeCalls; }
int HID_API_CALL hid_send_feature_report(hid_device*, const unsigned char* data, size_t length)
{
    ++sendCalls;
    lastReport.assign(data, data + length);
    return sendResult == -2 ? static_cast<int>(length) : sendResult;
}
}

static_assert(!std::is_copy_constructible_v<HidApiDevice>);
static_assert(!std::is_copy_assignable_v<HidApiDevice>);

class HidBackendTest : public QObject {
    Q_OBJECT
private slots:
    void init()
    {
        openCalls = closeCalls = sendCalls = 0;
        openSucceeds = true;
        sendResult = -2;
        lastReport.clear();
    }

    void lifecycle()
    {
        {
            HidApiDevice first;
            QVERIFY(!first.isOpen());
            QVERIFY(first.open(0x0db0, 0x0076));
            QCOMPARE(lastVendor, 0x0db0);
            QCOMPARE(lastProduct, 0x0076);
            {
                HidApiDevice second;
                QVERIFY(second.open(1, 2));
                QCOMPARE(initCalls, 1);
            }
            QCOMPARE(closeCalls, 1);
            QCOMPARE(exitCalls, 0);
            QVERIFY(first.isOpen());
            QVERIFY(first.open(3, 4));
            QCOMPARE(closeCalls, 2);
            first.close();
            first.close();
            QCOMPARE(closeCalls, 3);
            QVERIFY(!first.isOpen());
        }
        QCOMPARE(closeCalls, 3);
    }

    void failedOpen()
    {
        HidApiDevice device;
        QVERIFY(device.open(1, 2));
        openSucceeds = false;
        QVERIFY(!device.open(1, 2));
        QVERIFY(!device.isOpen());
        QCOMPARE(closeCalls, 1);
    }

    void writes()
    {
        HidApiDevice device;
        const std::uint8_t report[]{0x50, 1, 2};
        QVERIFY(!device.sendFeatureReport(report, sizeof(report)));
        QVERIFY(device.open(1, 2));
        QVERIFY(!device.sendFeatureReport(nullptr, 3));
        QVERIFY(!device.sendFeatureReport(report, 0));
        QVERIFY(!device.sendFeatureReport(report,
            static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1));
        QCOMPARE(sendCalls, 0);
        QVERIFY(device.sendFeatureReport(report, sizeof(report)));
        QVERIFY(lastReport == std::vector<unsigned char>({0x50, 1, 2}));
        for (auto result : {-1, 0, 2, 4}) {
            sendResult = result;
            QVERIFY(!device.sendFeatureReport(report, sizeof(report)));
        }
        device.close();
        QVERIFY(!device.sendFeatureReport(report, sizeof(report)));
        QCOMPARE(sendCalls, 5);
    }

    void fakeFailures()
    {
        FakeHidDevice device;
        device.openSucceeds = false;
        QVERIFY(!device.open(1, 2));
        QVERIFY(!device.isOpen());
        device.openSucceeds = true;
        QVERIFY(device.open(1, 2));
        device.sendSucceeds = false;
        const std::uint8_t report[]{0x51};
        QVERIFY(!device.sendFeatureReport(report, sizeof(report)));
        QCOMPARE(device.sentReports.size(), std::size_t{1});
        device.close();
        QVERIFY(!device.sendFeatureReport(report, sizeof(report)));
        QCOMPARE(device.sentReports.size(), std::size_t{1});
    }

    void initializationFailure()
    {
        if (!qEnvironmentVariableIsSet("HID_TEST_INIT_FAIL"))
            QSKIP("Run by the separate hid_init_failure CTest entry");
        HidApiDevice device;
        QVERIFY(!device.open(1, 2));
        QVERIFY(!device.isOpen());
        QCOMPARE(initCalls, 1);
        QCOMPARE(openCalls, 0);
    }
};

QTEST_APPLESS_MAIN(HidBackendTest)
#include "test_hid_backend.moc"

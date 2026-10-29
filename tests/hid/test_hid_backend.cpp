#include "hid/HidApiDevice.h"
#include "FakeHidDevice.h"
#include <hidapi.h>
#include <QtTest>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <type_traits>

// Link-time HIDAPI doubles: these tests cannot enumerate or open real devices.
struct hid_device_ {};
namespace {
hid_device_ token;
int initCalls = 0, exitCalls = 0, openCalls = 0, closeCalls = 0, sendCalls = 0, getFeatureCalls = 0, readCalls = 0;
bool openSucceeds = true;
int sendResult = -2;
int getFeatureResult = -2, readResult = -2;
std::vector<unsigned char> featureReportToReturn, inputReportToReturn;
unsigned short lastVendor = 0, lastProduct = 0;
std::vector<unsigned char> lastReport;
std::string lastPath;
hid_device_info firstInfo{}, secondInfo{}, invalidInfo{};
int enumCalls = 0, freeCalls = 0;
bool enumFails = false, enumEmpty = false;
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
hid_device* HID_API_CALL hid_open_path(const char* path)
{
    ++openCalls;
    lastPath = path;
    return openSucceeds ? &token : nullptr;
}
hid_device_info* HID_API_CALL hid_enumerate(unsigned short, unsigned short)
{
    ++enumCalls;
    return enumFails || enumEmpty ? nullptr : &firstInfo;
}
void HID_API_CALL hid_free_enumeration(hid_device_info*) { ++freeCalls; }
const wchar_t* HID_API_CALL hid_error(hid_device*) { return enumFails ? L"Enumeration error" : nullptr; }
void HID_API_CALL hid_close(hid_device*) { ++closeCalls; }
int HID_API_CALL hid_write(hid_device*, const unsigned char* data, size_t length)
{ ++sendCalls; lastReport.assign(data,data+length); return sendResult == -2 ? static_cast<int>(length) : sendResult; }
int HID_API_CALL hid_send_feature_report(hid_device*, const unsigned char* data, size_t length)
{
    ++sendCalls;
    lastReport.assign(data, data + length);
    return sendResult == -2 ? static_cast<int>(length) : sendResult;
}
int HID_API_CALL hid_get_feature_report(hid_device*, unsigned char* data, size_t length)
{
    ++getFeatureCalls;
    const auto copyLength = std::min(length, featureReportToReturn.size());
    std::copy_n(featureReportToReturn.begin(), copyLength, data);
    return getFeatureResult == -2 ? static_cast<int>(copyLength) : getFeatureResult;
}
int HID_API_CALL hid_read_timeout(hid_device*, unsigned char* data, size_t length, int)
{
    ++readCalls;
    const auto copyLength = std::min(length, inputReportToReturn.size());
    std::copy_n(inputReportToReturn.begin(), copyLength, data);
    return readResult == -2 ? static_cast<int>(copyLength) : readResult;
}
}

static_assert(!std::is_copy_constructible_v<HidApiDevice>);
static_assert(!std::is_copy_assignable_v<HidApiDevice>);

class HidBackendTest : public QObject {
    Q_OBJECT
private slots:
    void init()
    {
        openCalls = closeCalls = sendCalls = getFeatureCalls = readCalls = 0;
        openSucceeds = true;
        sendResult = -2;
        getFeatureResult = readResult = -2;
        featureReportToReturn.clear();
        inputReportToReturn.clear();
        lastReport.clear();
        lastPath.clear();
        enumCalls = freeCalls = 0;
        enumFails = enumEmpty = false;
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
        QVERIFY(!device.writeOutputReport(report,sizeof(report)));
        QVERIFY(device.open(1,2)); sendResult=-2;
        QVERIFY(device.writeOutputReport(report,sizeof(report)));
        QVERIFY(!device.writeOutputReport(nullptr,sizeof(report)));
        sendResult=2; QVERIFY(!device.writeOutputReport(report,sizeof(report)));
        QCOMPARE(sendCalls,7);
    }

    void reads()
    {
        HidApiDevice device;
        std::uint8_t buffer[8]{};
        QCOMPARE(device.getFeatureReport(buffer, sizeof(buffer)), std::size_t{0});
        QCOMPARE(device.readInputReport(buffer, sizeof(buffer)), std::size_t{0});
        QVERIFY(device.open(1, 2));
        QCOMPARE(device.getFeatureReport(nullptr, sizeof(buffer)), std::size_t{0});
        QCOMPARE(device.getFeatureReport(buffer, 0), std::size_t{0});
        QCOMPARE(device.readInputReport(nullptr, sizeof(buffer)), std::size_t{0});
        QCOMPARE(device.readInputReport(buffer, 0), std::size_t{0});
        QCOMPARE(getFeatureCalls, 0);
        QCOMPARE(readCalls, 0);

        featureReportToReturn = {0x52, 1, 2, 3};
        QCOMPARE(device.getFeatureReport(buffer, sizeof(buffer)), std::size_t{4});
        QCOMPARE(buffer[0], std::uint8_t{0x52});
        QCOMPARE(getFeatureCalls, 1);

        inputReportToReturn = {0xAA, 0xBB};
        QCOMPARE(device.readInputReport(buffer, sizeof(buffer)), std::size_t{2});
        QCOMPARE(buffer[0], std::uint8_t{0xAA});
        QCOMPARE(readCalls, 1);

        for (auto result : {-1, 0}) {
            getFeatureResult = result;
            QCOMPARE(device.getFeatureReport(buffer, sizeof(buffer)), std::size_t{0});
            readResult = result;
            QCOMPARE(device.readInputReport(buffer, sizeof(buffer)), std::size_t{0});
        }

        device.close();
        QCOMPARE(device.getFeatureReport(buffer, sizeof(buffer)), std::size_t{0});
        QCOMPARE(device.readInputReport(buffer, sizeof(buffer)), std::size_t{0});
        QCOMPARE(getFeatureCalls, 3);
        QCOMPARE(readCalls, 3);
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

    void fakeReads()
    {
        FakeHidDevice device;
        std::uint8_t buffer[8]{};
        QCOMPARE(device.getFeatureReport(buffer, sizeof(buffer)), std::size_t{0});
        QVERIFY(device.open(1, 2));
        device.featureReportResponse = {0x52, 9, 9};
        device.inputReportResponse = {0x14, 1};
        QCOMPARE(device.getFeatureReport(buffer, sizeof(buffer)), std::size_t{3});
        QCOMPARE(buffer[0], std::uint8_t{0x52});
        QCOMPARE(device.readInputReport(buffer, sizeof(buffer)), std::size_t{2});
        QCOMPARE(buffer[0], std::uint8_t{0x14});
        QCOMPARE(device.getFeatureReportCalls, 1);
        QCOMPARE(device.readInputReportCalls, 1);

        device.readSucceeds = false;
        QCOMPARE(device.getFeatureReport(buffer, sizeof(buffer)), std::size_t{0});
        QCOMPARE(device.readInputReport(buffer, sizeof(buffer)), std::size_t{0});
        QCOMPARE(device.getFeatureReportCalls, 2);
        QCOMPARE(device.readInputReportCalls, 2);
    }

    void enumeration()
    {
        char firstPath[] = "endpoint-a", secondPath[] = "endpoint-b";
        firstInfo = {}; secondInfo = {}; invalidInfo = {};
        firstInfo.path = firstPath; firstInfo.vendor_id = 0x0db0; firstInfo.product_id = 0x76;
        firstInfo.usage_page = 0xff00; firstInfo.usage = 1; firstInfo.interface_number = 2;
        firstInfo.next = &secondInfo;
        secondInfo.path = secondPath; secondInfo.interface_number = -1; secondInfo.next = &invalidInfo;
        const auto descriptors = HidApiDevice::enumerate();
        QCOMPARE(descriptors.size(), std::size_t{2});
        QCOMPARE(descriptors[0].interfaceNumber, 2);
        QCOMPARE(descriptors[0].usagePage, 0xff00);
        QCOMPARE(descriptors[0].vendorId, 0x0db0);
        QCOMPARE(descriptors[1].interfaceNumber, -1);
        QVERIFY(descriptors[1].manufacturer.empty());
        firstPath[0] = 'X';
        QCOMPARE(descriptors[0].path, std::string("endpoint-a"));
        QCOMPARE(freeCalls, 1);
        QCOMPARE(openCalls, 0); QCOMPARE(sendCalls, 0);
        enumEmpty = true;
        QVERIFY(HidApiDevice::enumerate().empty());
        enumFails = true;
        QVERIFY_EXCEPTION_THROWN(HidApiDevice::enumerate(), std::runtime_error);
    }

    void exactPath()
    {
        HidApiDevice device;
        QVERIFY(device.openPath("endpoint-b"));
        QCOMPARE(lastPath, std::string("endpoint-b"));
        QVERIFY(!device.openPath(""));
        QVERIFY(!device.isOpen()); QCOMPARE(closeCalls, 1);
        QVERIFY(!device.openPath(std::string("x\0y", 3)));
        QCOMPARE(openCalls, 1);
        openSucceeds = false;
        QVERIFY(!device.openPath("missing"));
        QVERIFY(!device.isOpen());
    }

    void initializationFailure()
    {
        if (!qEnvironmentVariableIsSet("HID_TEST_INIT_FAIL"))
            QSKIP("Run by the separate hid_init_failure CTest entry");
        HidApiDevice device;
        QVERIFY(!device.open(1, 2));
        QVERIFY(!device.openPath("endpoint"));
        QVERIFY_EXCEPTION_THROWN(HidApiDevice::enumerate(), std::runtime_error);
        QVERIFY(!device.isOpen());
        QCOMPARE(enumCalls, 0);
        QCOMPARE(initCalls, 1);
        QCOMPARE(openCalls, 0);
    }
};

QTEST_APPLESS_MAIN(HidBackendTest)
#include "test_hid_backend.moc"

#include "modules/ModuleRegistry.h"
#include "discovery/HardwareDiscovery.h"
#include "hid/FakeHidDevice.h"
#include <QtTest>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QTemporaryDir>
#include <stdexcept>

class ModulesTest : public QObject {
    Q_OBJECT
private:
    DeviceProfile profile() const { return DeviceProfile::load(TEST_PROFILE_PATH); }
    QJsonObject profileJson() const
    {
        QFile file(TEST_PROFILE_PATH);
        if (!file.open(QIODevice::ReadOnly)) return {};
        return QJsonDocument::fromJson(file.readAll()).object();
    }
    DeviceIdentity identity() const
    {
        DeviceIdentity device;
        device.path = "fake-hid-path";
        device.vendorId = 0x0db0;
        device.productId = 0x0076;
        device.boardManufacturer = "Micro-Star International Co., Ltd.";
        device.boardModel = "MPG B850 EDGE TI WIFI (MS-7E62)";
        return device;
    }

private slots:
    void discoveryCandidates()
    {
        auto boardProfile = profile();
        HidDescriptor endpoint;
        endpoint.path = "endpoint-a";
        endpoint.vendorId = 0x0db0; endpoint.productId = 0x76;
        endpoint.interfaceNumber = 0;
        auto second = endpoint; second.path = "endpoint-b"; second.interfaceNumber = 2;
        BoardIdentity board{identity().boardManufacturer, identity().boardModel};
        auto matches = findProfileCandidates({endpoint, second}, board, {boardProfile});
        QCOMPARE(matches.size(), 2); // No unverified first-interface preference.
        QCOMPARE(matches[1].device.path, QString("endpoint-b"));
        QVERIFY(findProfileCandidates({endpoint}, {}, {boardProfile}).isEmpty());
        boardProfile.match.insert("interfaceNumber", 2);
        matches = findProfileCandidates({endpoint, second}, board, {boardProfile});
        QCOMPARE(matches.size(), 1);
        QCOMPARE(matches[0].device.interfaceNumber, 2);
        second.path.clear();
        QVERIFY(findProfileCandidates({second}, board, {boardProfile}).isEmpty());
        auto alias = boardProfile; alias.id = "another-profile";
        second.path = "endpoint-b";
        QCOMPARE(findProfileCandidates({second}, board, {boardProfile, alias}).size(), 2);
    }

    void descriptorMatching()
    {
        auto board = profile();
        auto device = identity();
        QVERIFY(board.matches(device));
        device.boardModel = "MAG B850 TOMAHAWK MAX WIFI";
        QVERIFY(!board.matches(device));
        device.boardModel.clear();
        QVERIFY(!board.matches(device));
        device = identity();
        device.boardModel = " mpg b850 edge ti wifi (ms-7e62) ";
        QVERIFY(board.matches(device));
        device.productId = 1;
        QVERIFY(!board.matches(device));
        device = identity();
        board.match.insert("interfaceNumber", 2);
        board.match.insert("usagePage", 0xff00);
        board.match.insert("usage", 1);
        QVERIFY(!board.matches(device));
        device.interfaceNumber = 2;
        device.usagePage = 0xff00;
        device.usage = 1;
        QVERIFY(board.matches(device));
        device.usage = 2;
        QVERIFY(!board.matches(device));
        board.match.insert("usagePage", 0);
        board.match.insert("usage", 0);
        device.usagePage = -1;
        device.usage = -1;
        QVERIFY(!board.matches(device));
        device.usagePage = 0;
        device.usage = 0;
        QVERIFY(board.matches(device));
    }

    void invalidProfiles()
    {
        auto json = profileJson();
        json.insert("schemaVersion", 2);
        QVERIFY_EXCEPTION_THROWN(DeviceProfile::fromJson(json), std::invalid_argument);
        json = profileJson();
        auto match = json["match"].toObject();
        match.insert("boardModle", "misspelled-selector");
        json.insert("match", match);
        QVERIFY_EXCEPTION_THROWN(DeviceProfile::fromJson(json), std::invalid_argument);
        json = profileJson();
        match = json["match"].toObject();
        match.insert("vendorId", 3504.5);
        json.insert("match", match);
        QVERIFY_EXCEPTION_THROWN(DeviceProfile::fromJson(json), std::invalid_argument);
        json = profileJson();
        auto channels = json["channels"].toArray();
        channels.append(channels[0]);
        json.insert("channels", channels);
        QVERIFY_EXCEPTION_THROWN(DeviceProfile::fromJson(json), std::invalid_argument);
        json = profileJson();
        channels = json["channels"].toArray();
        auto channel = channels[0].toObject();
        channel.insert("fixedLeds", 121);
        channels[0] = channel;
        json.insert("channels", channels);
        QVERIFY_EXCEPTION_THROWN(DeviceProfile::fromJson(json), std::invalid_argument);
        QTemporaryDir temporary;
        QVERIFY(temporary.isValid());
        QFile file(temporary.filePath("broken.json"));
        QVERIFY(file.open(QIODevice::WriteOnly));
        file.write("{broken");
        file.close();
        QVERIFY_EXCEPTION_THROWN(DeviceProfile::load(file.fileName()), std::invalid_argument);
        QVERIFY_EXCEPTION_THROWN(DeviceProfile::load(temporary.filePath("missing.json")), std::invalid_argument);
    }

    void loadingAndCompatibility()
    {
        ModuleRegistry registry;
        QString error;
        QVERIFY(!registry.load("missing.dll", &error));
        QVERIFY(!error.isEmpty());
        QVERIFY(!registry.load(INCOMPATIBLE_MODULE_PATH, &error));
        QVERIFY(error.contains("Incompatible"));
        QVERIFY(!registry.load(MISSING_INTERFACE_PATH, &error));
        QVERIFY(error.contains("interface"));
        QVERIFY2(registry.load(TEST_MODULE_PATH, &error), qPrintable(error));
        QVERIFY(error.isEmpty());
        QCOMPARE(registry.moduleIds(), QStringList{"msi.gen1"});
        QVERIFY(!registry.load(TEST_MODULE_PATH, &error));
        QVERIFY(error.contains("Duplicate"));
        auto board = profile();
        QVERIFY(registry.acceptsProfile(board, &error));
        board.driverId = "absent.driver";
        QVERIFY(!registry.acceptsProfile(board, &error));
        QVERIFY(error.contains("not loaded"));
        QTemporaryDir emptyDirectory;
        QVERIFY(emptyDirectory.isValid());
        QVERIFY(registry.loadDirectory(emptyDirectory.path()).isEmpty());
        QVERIFY(!registry.loadDirectory(emptyDirectory.filePath("missing")).isEmpty());
    }

    void moduleAddressValidation()
    {
        ModuleRegistry registry;
        QString error;
        QVERIFY(registry.load(TEST_MODULE_PATH, &error));
        auto board = profile();
        board.channels[0].address.insert("zone", 18);
        QVERIFY(!registry.acceptsProfile(board, &error));
        board = profile();
        board.channels[0].address.insert("area", 0x0300);
        QVERIFY(!registry.acceptsProfile(board, &error));
        board = profile();
        board.channels[1].address.insert("zone", 0);
        QVERIFY(!registry.acceptsProfile(board, &error));
        board = profile();
        board.channels[0].nativeModes.append("invented-mode");
        QVERIFY(!registry.acceptsProfile(board, &error));
        board = profile();
        board.match.remove("boardModel");
        QVERIFY(!registry.acceptsProfile(board, &error));
    }

    void independentProtocol()
    {
        ModuleRegistry registry;
        QString error;
        QVERIFY(registry.load(TEST_MODULE_PATH, &error));
        QVERIFY(registry.load(VIRTUAL_MODULE_PATH, &error));
        const auto json = QJsonDocument::fromJson(R"({
            "schemaVersion":1,"id":"test.device","driverId":"test.virtual","name":"Virtual device",
            "match":{"vendorId":123,"productId":456},
            "channels":[{"id":"Virtual","maxLeds":1,"fixedLeds":1,"direct":true,"nativeModes":[],"address":{}}]
        })").object();
        const auto deviceProfile = DeviceProfile::fromJson(json);
        DeviceIdentity device;
        device.path = "virtual-path";
        device.vendorId = 123;
        device.productId = 456;
        FakeHidDevice transport;
        QVERIFY(transport.open(123, 456));
        auto instance = registry.create(device, deviceProfile, transport, &error);
        QVERIFY2(instance != nullptr, qPrintable(error));
        QVERIFY(instance->driver().pushColors("Virtual", {{10, 20, 30}}));
        QVERIFY(transport.sentReports.back() == std::vector<std::uint8_t>({1, 10, 20, 30}));
    }

    void driverLifetimeAndCreation()
    {
        FakeHidDevice transport;
        QVERIFY(transport.open(1, 2));
        std::unique_ptr<DriverInstance> instance;
        {
            ModuleRegistry registry;
            QString error;
            QVERIFY(registry.load(TEST_MODULE_PATH, &error));
            auto wrongDevice = identity();
            wrongDevice.boardModel.clear();
            QVERIFY(!registry.create(wrongDevice, profile(), transport, &error));
            wrongDevice = identity();
            wrongDevice.path.clear();
            QVERIFY(!registry.create(wrongDevice, profile(), transport, &error));
            QVERIFY(transport.sentReports.empty());
            instance = registry.create(identity(), profile(), transport, &error);
            QVERIFY2(instance != nullptr, qPrintable(error));
            QVERIFY(transport.sentReports.empty());
        }
        // The driver remains callable after the registry is destroyed.
        QCOMPARE(instance->driver().discoverLeds().status, DiscoveryStatus::Unknown);
        QVERIFY(transport.sentReports.empty());
        QVERIFY(instance->driver().setNativeMode("IOShield", NativeEffect{}));
        QCOMPARE(transport.sentReports.size(), std::size_t{1});
    }

    void driverWritesAndFailures()
    {
        ModuleRegistry registry;
        QString error;
        QVERIFY(registry.load(TEST_MODULE_PATH, &error));
        FakeHidDevice transport;
        auto instance = registry.create(identity(), profile(), transport, &error);
        QVERIFY(instance != nullptr);
        auto& driver = instance->driver();
        QVERIFY(!driver.pushColors("JARGB1", {{1, 2, 3}}));
        QVERIFY(transport.open(1, 2));
        QVERIFY(!driver.pushColors("IOShield", {{1, 2, 3}}));
        QVERIFY(!driver.pushColors("JRGB1", {{1, 2, 3}}));
        QVERIFY(!driver.pushColors("unknown", {{1, 2, 3}}));
        QVERIFY(!driver.pushColors("JARGB1", {}));
        QVERIFY(!driver.pushColors("JARGB1", QVector<RgbColor>(121)));
        NativeEffect invalid;
        invalid.brightness = 6;
        QVERIFY(!driver.setNativeMode("JARGB1", invalid));
        QVERIFY(transport.sentReports.empty());
        NativeEffect native;
        native.id = "breathing";
        native.colors = {{11, 22, 33}};
        QVERIFY(driver.setNativeMode("JARGB2", native));
        QVERIFY(driver.pushColors("JARGB1", {{1, 2, 3}, {4, 5, 6}}));
        QCOMPARE(transport.sentReports.size(), std::size_t{3});
        const auto& arm = transport.sentReports[1];
        QCOMPARE(arm[1], 9);
        QCOMPARE(arm[17], 4); // The other channel retains native Breathing.
        QCOMPARE(arm[18], 11);
        const auto& colors = transport.sentReports[2];
        QCOMPARE(colors.size(), std::size_t{727});
        QCOMPARE(colors[0], 0x51);
        QCOMPARE(colors[2], 4);
        QCOMPARE(colors[6], 2);
        QCOMPARE(colors[7], 1);
        QCOMPARE(colors[12], 6);
        QVERIFY(driver.pushColors("JARGB1", {{7, 8, 9}}));
        QCOMPARE(transport.sentReports.size(), std::size_t{4});
        for (const auto& report : transport.sentReports)
            if (report[0] == 0x50) QCOMPARE(report[289], 0);
        transport.sendSucceeds = false;
        QVERIFY(!driver.pushColors("JARGB1", {{1, 2, 3}}));
        transport.sendSucceeds = true;
        QVERIFY(driver.pushColors("JARGB1", {{1, 2, 3}}));
        QCOMPARE(transport.sentReports[5][0], 0x50); // Retry rearms after a failed write.
        transport.sendSucceeds = false;
        native.colors = {{99, 88, 77}};
        QVERIFY(!driver.setNativeMode("JARGB2", native));
        transport.sendSucceeds = true;
        QVERIFY(driver.setNativeMode("IOShield", NativeEffect{}));
        QCOMPARE(transport.sentReports.back()[18], 11); // Failed settings were not committed.
        const auto before = transport.sentReports.size();
        QVERIFY(driver.pushColors("JARGB1", {{1, 2, 3}}));
        QCOMPARE(transport.sentReports.size(), before + 2);
        QCOMPARE(transport.sentReports[before][0], 0x50);
    }
};

QTEST_APPLESS_MAIN(ModulesTest)
#include "test_modules.moc"

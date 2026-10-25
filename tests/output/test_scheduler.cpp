#include "output/OutputScheduler.h"
#include "protocol/MsiProtocol.h"
#include <QTimer>
#include <QtTest>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <stdexcept>
#include <chrono>

namespace {
struct Record {
    mutable std::mutex mutex;
    std::condition_variable wake;
    bool blockFactory = false, releaseFactory = false, factoryEntered = false;
    bool blockWrite = false, releaseWrite = false, writeEntered = false;
    bool opened = true;
    int failAt = 0, closes = 0, destroyed = 0;
    std::thread::id createdOn, writtenOn, closedOn, destroyedOn;
    std::vector<std::vector<std::uint8_t>> reports;
    std::vector<std::chrono::steady_clock::time_point> timestamps;
    int writes() const { const std::lock_guard<std::mutex> lock(mutex); return int(reports.size()); }
    bool inFactory() const { const std::lock_guard<std::mutex> lock(mutex); return factoryEntered; }
    bool inWrite() const { const std::lock_guard<std::mutex> lock(mutex); return writeEntered; }
    void release() {
        { const std::lock_guard<std::mutex> lock(mutex); releaseFactory = releaseWrite = true; }
        wake.notify_all();
    }
};
// Every assertion path releases blocked test I/O before the scheduler destructor joins its worker.
struct ReleaseGate {
    std::shared_ptr<Record> record;
    ~ReleaseGate() { record->release(); }
};
class Transport : public IHidDevice {
public:
    explicit Transport(std::shared_ptr<Record> record) : record_(std::move(record)) {
        const std::lock_guard<std::mutex> lock(record_->mutex); record_->createdOn = std::this_thread::get_id();
    }
    ~Transport() override {
        const std::lock_guard<std::mutex> lock(record_->mutex);
        ++record_->destroyed; record_->destroyedOn = std::this_thread::get_id();
    }
    bool open(std::uint16_t, std::uint16_t) override { return false; }
    void close() override {
        const std::lock_guard<std::mutex> lock(record_->mutex);
        record_->opened = false; ++record_->closes; record_->closedOn = std::this_thread::get_id();
    }
    bool isOpen() const override { const std::lock_guard<std::mutex> lock(record_->mutex); return record_->opened; }
    bool sendFeatureReport(const std::uint8_t* bytes, std::size_t size) override {
        std::unique_lock<std::mutex> lock(record_->mutex);
        record_->writtenOn = std::this_thread::get_id();
        record_->reports.emplace_back(bytes,bytes+size);
        record_->timestamps.push_back(std::chrono::steady_clock::now());
        if (record_->blockWrite && !record_->writeEntered) {
            record_->writeEntered = true;
            record_->wake.wait(lock,[this] { return record_->releaseWrite; });
        }
        return int(record_->reports.size()) != record_->failAt;
    }
private:
    std::shared_ptr<Record> record_;
};
DeviceIdentity identity()
{
    DeviceIdentity result; result.path = "fake-hid-path"; result.vendorId = 0x0db0; result.productId = 0x76;
    result.boardManufacturer = "Micro-Star International Co., Ltd.";
    result.boardModel = "MPG B850 EDGE TI WIFI (MS-7E62)"; return result;
}
OutputScheduler::SessionFactory factory(std::shared_ptr<Record> record)
{
    return [record] {
        {
            std::unique_lock<std::mutex> lock(record->mutex); record->factoryEntered = true;
            if (record->blockFactory) record->wake.wait(lock,[record] { return record->releaseFactory; });
        }
        ModuleRegistry modules; QString error;
        if (!modules.load(TEST_MODULE_PATH,&error)) throw std::runtime_error(error.toStdString());
        auto session = std::make_unique<OutputSession>();
        session->connect(modules,identity(),DeviceProfile::load(TEST_PROFILE_PATH),std::make_unique<Transport>(record));
        return session;
    };
}
OutputFrame frame(quint8 red = 1)
{
    OutputFrame result; result.endpoint = identity().path;
    result.profileId = DeviceProfile::load(TEST_PROFILE_PATH).id;
    ChannelOutput software; software.channelId = "JARGB1"; software.software = true; software.colors = {{red,2,3}};
    ChannelOutput native; native.channelId = "IOShield";
    result.channels = {software,native}; return result;
}
void assertClosedOnWorker(const std::shared_ptr<Record>& record)
{
    const std::lock_guard<std::mutex> lock(record->mutex);
    QCOMPARE(record->closes,1); QCOMPARE(record->destroyed,1);
    QVERIFY(record->createdOn != std::this_thread::get_id());
    QVERIFY(record->createdOn == record->closedOn); QVERIFY(record->createdOn == record->destroyedOn);
    if (!record->reports.empty()) QVERIFY(record->createdOn == record->writtenOn);
}
}

class SchedulerTest : public QObject {
    Q_OBJECT
private slots:
    void latestWhileStarting();
    void latestDuringSlowWrite();
    void stopDuringWrite();
    void cancelFactoryAndRestart();
    void faultsAndExplicitReconnect();
    void destruction();
};

void SchedulerTest::latestWhileStarting()
{
    auto record = std::make_shared<Record>(); record->blockFactory = true;
    OutputScheduler scheduler; ReleaseGate release{record};
    int notifications = 0; bool ownerThread = true;
    const auto owner = std::this_thread::get_id();
    QObject::connect(&scheduler,&OutputScheduler::statusChanged,&scheduler,[&] {
        ++notifications; ownerThread = ownerThread && std::this_thread::get_id() == owner;
    });
    const auto generation = scheduler.start(factory(record)); QVERIFY(generation);
    QTRY_VERIFY(record->inFactory());
    QCOMPARE(scheduler.status().state,OutputScheduler::State::Starting);
    QVERIFY(!scheduler.start(factory(record)));
    for (int value = 1; value <= 100; ++value) QVERIFY(scheduler.submit(generation,frame(quint8(value))));
    QCOMPARE(record->writes(),0); record->release();
    QTRY_COMPARE(record->writes(),3);
    {
        const std::lock_guard<std::mutex> lock(record->mutex);
        const auto expected = BuildReport0x51(MsiGen1AdvArea::JARGB1,{{100,2,3}});
        QVERIFY(record->reports[2] == std::vector<std::uint8_t>(expected.begin(),expected.end())); // Board configuration precedes pixel data.
    }
    QTest::qWait(80); QCOMPARE(record->writes(),3); // No backlog and no implicit repeat of an idle frame.
    scheduler.stop(); QTRY_COMPARE(scheduler.status().state,OutputScheduler::State::Stopped);
    QTRY_VERIFY(notifications >= 4); QVERIFY(ownerThread);
    assertClosedOnWorker(record);
}

void SchedulerTest::latestDuringSlowWrite()
{
    auto record = std::make_shared<Record>(); record->blockWrite = true;
    OutputScheduler scheduler; ReleaseGate release{record};
    const auto generation = scheduler.start(factory(record)); QVERIFY(scheduler.submit(generation,frame()));
    QTRY_VERIFY(record->inWrite());
    bool eventProcessed = false; QTimer::singleShot(0,[&] { eventProcessed = true; });
    QTRY_VERIFY(eventProcessed); // The owner event loop stays usable while fake HID is blocked.
    QVERIFY(scheduler.submit(generation,frame(20))); QVERIFY(scheduler.submit(generation,frame(30)));
    record->release(); QTRY_COMPARE(record->writes(),4);
    {
        const std::lock_guard<std::mutex> lock(record->mutex);
        const auto expected = BuildReport0x51(MsiGen1AdvArea::JARGB1,{{30,2,3}});
        QVERIFY(record->reports.back() == std::vector<std::uint8_t>(expected.begin(),expected.end()));
        QVERIFY(record->timestamps[3] - record->timestamps[2] >= std::chrono::milliseconds(30));
    }
    scheduler.stop(); QTRY_COMPARE(scheduler.status().state,OutputScheduler::State::Stopped);
    assertClosedOnWorker(record);
}

void SchedulerTest::stopDuringWrite()
{
    auto record = std::make_shared<Record>(); record->blockWrite = true;
    OutputScheduler scheduler; ReleaseGate release{record};
    const auto generation = scheduler.start(factory(record)); QVERIFY(scheduler.submit(generation,frame()));
    QTRY_VERIFY(record->inWrite()); QVERIFY(scheduler.submit(generation,frame(90)));
    scheduler.stop(); QCOMPARE(scheduler.status().state,OutputScheduler::State::Stopping);
    QVERIFY(!scheduler.submit(generation,frame())); QVERIFY(!scheduler.start(factory(record)));
    record->release(); QTRY_COMPARE(scheduler.status().state,OutputScheduler::State::Stopped);
    // Configuration is now first: finish that in-flight call, then cancel before any pixel writes.
    QCOMPARE(record->writes(),1); assertClosedOnWorker(record);
}

void SchedulerTest::cancelFactoryAndRestart()
{
    auto record = std::make_shared<Record>(); record->blockFactory = true;
    OutputScheduler scheduler; ReleaseGate release{record};
    const auto old = scheduler.start(factory(record)); QTRY_VERIFY(record->inFactory());
    QVERIFY(scheduler.submit(old,frame())); scheduler.stop();
    QCOMPARE(scheduler.status().state,OutputScheduler::State::Stopping);
    record->release(); QTRY_COMPARE(scheduler.status().state,OutputScheduler::State::Stopped);
    QCOMPARE(record->writes(),0); assertClosedOnWorker(record);
    auto fresh = std::make_shared<Record>(); ReleaseGate freshRelease{fresh};
    const auto generation = scheduler.start(factory(fresh)); QVERIFY(generation > old);
    QVERIFY(!scheduler.submit(old,frame(80))); QVERIFY(scheduler.submit(generation,frame(90)));
    QTRY_COMPARE(fresh->writes(),3);
    scheduler.stop(); QTRY_COMPARE(scheduler.status().state,OutputScheduler::State::Stopped);
    assertClosedOnWorker(fresh);
}

void SchedulerTest::faultsAndExplicitReconnect()
{
    OutputScheduler scheduler;
    QVERIFY(!scheduler.start({}));
    QVERIFY(scheduler.start([] { return std::unique_ptr<OutputSession>{}; }));
    QTRY_COMPARE(scheduler.status().state,OutputScheduler::State::Faulted);
    QVERIFY(scheduler.start([] { return std::make_unique<OutputSession>(); }));
    QTRY_COMPARE(scheduler.status().state,OutputScheduler::State::Faulted);
    const auto failed = scheduler.start([]() -> std::unique_ptr<OutputSession> { throw std::runtime_error("Factory failed"); });
    QVERIFY(failed); QTRY_COMPARE(scheduler.status().state,OutputScheduler::State::Faulted);
    QCOMPARE(scheduler.status().error,QString("Factory failed"));
    QVERIFY(!scheduler.submit(failed,frame()));
    auto record = std::make_shared<Record>(); record->failAt = 1; ReleaseGate release{record};
    const auto writing = scheduler.start(factory(record)); QVERIFY(writing > failed);
    QVERIFY(scheduler.submit(writing,frame())); QTRY_COMPARE(scheduler.status().state,OutputScheduler::State::Faulted);
    QCOMPARE(record->writes(),1); QVERIFY(!scheduler.status().error.isEmpty()); assertClosedOnWorker(record);
    QTest::qWait(40); QCOMPARE(record->writes(),1); // Never retries by itself.
    auto fresh = std::make_shared<Record>(); ReleaseGate freshRelease{fresh};
    const auto invalid = scheduler.start(factory(fresh)); auto wrong = frame(); wrong.endpoint = "wrong-endpoint";
    QVERIFY(scheduler.submit(invalid,wrong)); QTRY_COMPARE(scheduler.status().state,OutputScheduler::State::Faulted);
    QCOMPARE(fresh->writes(),0); assertClosedOnWorker(fresh);
    auto reconnected = std::make_shared<Record>(); ReleaseGate reconnectRelease{reconnected};
    const auto current = scheduler.start(factory(reconnected)); QVERIFY(scheduler.submit(current,frame()));
    QTRY_COMPARE(reconnected->writes(),3); // Reconnect re-arms and reapplies native settings.
    scheduler.stop(); QTRY_COMPARE(scheduler.status().state,OutputScheduler::State::Stopped);
    assertClosedOnWorker(reconnected);
}

void SchedulerTest::destruction()
{
    auto record = std::make_shared<Record>();
    {
        OutputScheduler scheduler; ReleaseGate release{record};
        const auto generation = scheduler.start(factory(record)); QVERIFY(scheduler.submit(generation,frame()));
        QTRY_COMPARE(record->writes(),3);
    }
    assertClosedOnWorker(record);
}

QTEST_GUILESS_MAIN(SchedulerTest)
#include "test_scheduler.moc"

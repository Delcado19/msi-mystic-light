#include "OutputScheduler.h"
#include <QThread>
#include <chrono>
#include <exception>
#ifdef Q_OS_WIN
#include <windows.h>
#include <cfgmgr32.h>
#endif

namespace {
// Owner decision 2026-11-18: hardware is installed with the PC off, so the presence check runs only
// when Windows reports a HID device arriving or leaving, not once a second per output. Resume is
// handled by the power events (service/main.cpp, main.cpp), write failures by the fault path.
struct DeviceChanges { std::mutex mutex; std::condition_variable wake; quint64 generation = 0; };
// shortcut: never freed, because detached probes can outlive every scheduler; fine for one per process.
DeviceChanges& deviceChanges() { static auto* changes = new DeviceChanges; return *changes; }
#ifdef Q_OS_WIN
DWORD CALLBACK onHidInterface(HCMNOTIFICATION, PVOID, CM_NOTIFY_ACTION, PCM_NOTIFY_EVENT_DATA, DWORD)
{
    OutputScheduler::deviceChanged(); return ERROR_SUCCESS; // Threadpool callback: only signal, never probe here.
}
#endif
// Set once registration succeeded. Probes read it without waiting; until then (or if registration
// fails) they use the one-second check (CodeRabbit review PR #4: start() must never wait on it).
std::atomic_bool watching{false};
// Blocking one-time registration; only the startup helper thread (and tests) call it.
bool watchDevices()
{
#ifdef Q_OS_WIN
    static const bool registered = [] {
        CM_NOTIFY_FILTER filter{}; filter.cbSize = sizeof(filter); filter.FilterType = CM_NOTIFY_FILTER_TYPE_DEVICEINTERFACE;
        filter.u.DeviceInterface.ClassGuid = {0x4d1e55b2, 0xf16f, 0x11cf, {0x88, 0xcb, 0x00, 0x11, 0x11, 0x00, 0x00, 0x30}}; // GUID_DEVINTERFACE_HID
        HCMNOTIFICATION handle = nullptr; // Never unregistered: it lives as long as the process.
        const bool ok = CM_Register_Notification(&filter, nullptr, onHidInterface, &handle) == CR_SUCCESS;
        if (!ok) { qWarning("HID device notifications unavailable; presence check falls back to once a second"); return false; }
        watching = true; OutputScheduler::deviceChanged(); // One check covers changes made before registration.
        return true;
    }();
    return registered;
#else
    return false;
#endif
}
}

bool OutputScheduler::deviceNotifications() { return watchDevices(); }

void OutputScheduler::deviceChanged()
{
    auto& changes = deviceChanges();
    { const std::lock_guard<std::mutex> lock(changes.mutex); ++changes.generation; }
    changes.wake.notify_all();
}

OutputScheduler::OutputScheduler(QObject* parent) : QObject(parent) {}
OutputScheduler::~OutputScheduler()
{
    stopRequested_.store(true); wake_.notify_all();
    if (worker_.joinable()) worker_.join(); // Do not destroy a transport under an active driver call.
}
OutputScheduler::Status OutputScheduler::status() const
{
    const std::lock_guard<std::mutex> lock(mutex_); return status_;
}
quint64 OutputScheduler::start(SessionFactory factory, std::function<bool()> present)
{
    Q_ASSERT(QThread::currentThread() == thread());
    if (!factory) return 0;
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        if (status_.state != State::Stopped && status_.state != State::Faulted) return 0;
    }
    if (worker_.joinable()) worker_.join();
    quint64 generation;
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        pending_.reset(); stopRequested_.store(false);
        status_.state = State::Starting; status_.error.clear(); generation = ++status_.generation;
    }
    // Snapshot before the device opens, so a removal during the open still triggers a check.
    const quint64 seen = [] { auto& changes = deviceChanges(); const std::lock_guard<std::mutex> lock(changes.mutex); return changes.generation; }();
    try { worker_ = std::thread([this, factory = std::move(factory), present = std::move(present), seen]() mutable { run(std::move(factory), std::move(present), seen); }); }
    catch (const std::exception& error) { finish(State::Faulted, QString::fromUtf8(error.what())); return 0; }
    emit statusChanged(); return generation;
}
bool OutputScheduler::submit(quint64 generation, OutputFrame frame)
{
    const std::lock_guard<std::mutex> lock(mutex_);
    if (generation != status_.generation || stopRequested_.load() ||
        (status_.state != State::Starting && status_.state != State::Running)) return false;
    pending_ = std::move(frame); return true;
}
void OutputScheduler::stop()
{
    Q_ASSERT(QThread::currentThread() == thread());
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        if (status_.state != State::Starting && status_.state != State::Running) return;
        stopRequested_.store(true); pending_.reset(); status_.state = State::Stopping;
    }
    wake_.notify_all(); emit statusChanged();
}
void OutputScheduler::finish(State state, const QString& error)
{
    {
        const std::lock_guard<std::mutex> lock(mutex_);
        pending_.reset(); status_.state = stopRequested_.load() ? State::Stopped : state;
        status_.error = stopRequested_.load() ? QString{} : error;
    }
    emit statusChanged();
}
void OutputScheduler::run(SessionFactory factory, std::function<bool()> present, quint64 seenAtStart)
{
    std::unique_ptr<OutputSession> session;
    QString error;
    try {
        if (!stopRequested_.load()) session = factory();
        if (!stopRequested_.load() && (!session || session->state() != OutputSession::State::Connected)) {
            error = session ? session->error() : "Output factory returned no session";
            if (error.isEmpty()) error = "Output factory returned a disconnected session";
        } else if (!stopRequested_.load()) {
            {
                const std::lock_guard<std::mutex> lock(mutex_);
                if (!stopRequested_.load()) status_.state = State::Running;
            }
            emit statusChanged();
            // The presence check (HID enumeration, board identity, auxiliary scan) took 112-453 ms. On this
            // worker it froze the LEDs that long (owner report 2026-11-18), so it runs on its own thread and
            // only reports its verdict. It waits for a HID arrival/removal (see deviceChanges) and is still
            // needed: native-only outputs write once, so a replugged device never fails a write.
            // The probe is detached and owns its state: HID enumeration can hang, and teardown must neither
            // wait for it nor let a late verdict reach a replacement session (CodeRabbit review 2026-11-18).
            // shortcut: a probe hung in the driver stays parked until the process ends; one per reopened output.
            struct Probe { std::mutex mutex; std::atomic_bool done{false}; std::atomic_bool gone{false}; QString error; };
            const auto probe = std::make_shared<Probe>();
            struct Finish { std::shared_ptr<Probe> probe;
                ~Finish() { auto& changes = deviceChanges(); { const std::lock_guard<std::mutex> lock(changes.mutex); probe->done = true; } changes.wake.notify_all(); } } finish{probe};
            if (present) std::thread([probe, present, seen = seenAtStart]() mutable {
                auto& changes = deviceChanges();
                for (;;) {
                    {
                        std::unique_lock<std::mutex> lock(changes.mutex);
                        const auto changed = [&] { return probe->done.load() || changes.generation != seen; };
                        if (!watching) {
                            if (changes.wake.wait_for(lock, std::chrono::seconds(1), [&] { return probe->done.load(); })) return;
                            seen = changes.generation; // This timed check covers every earlier event.
                        } else {
                            changes.wake.wait(lock, changed);
                            // One plug-in raises several interface events: probe once after they settle.
                            do { seen = changes.generation; if (probe->done) return; }
                            while (changes.wake.wait_for(lock, std::chrono::milliseconds(500), changed));
                        }
                    }
                    QString failure;
                    try { if (present()) continue; failure = "Selected endpoint disappeared or changed identity"; }
                    catch (const std::exception& e) { failure = QString::fromUtf8(e.what()); }
                    const std::lock_guard<std::mutex> lock(probe->mutex);
                    if (!probe->done) { probe->error = failure; probe->gone.store(true); }
                    return;
                }
            }).detach();
            while (!stopRequested_.load()) {
                std::optional<OutputFrame> frame;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    // Pace from completion, never catch up with a burst after a slow HID call.
                    wake_.wait_for(lock, std::chrono::milliseconds(33), [this] { return stopRequested_.load(); });
                    if (stopRequested_.load()) break;
                    if (probe->gone.load()) { const std::lock_guard<std::mutex> verdict(probe->mutex); error = probe->error; break; }
                    frame = std::move(pending_); pending_.reset();
                }
                // Diagnostics (owner report 2026-11-18): a call over 50 ms delays the next frame and is logged.
                const auto timed = [](const char* what, auto&& call) {
                    const auto started = std::chrono::steady_clock::now();
                    const bool ok = call();
                    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - started).count();
                    if (ms > 50) qWarning("Output stall: %s took %lld ms", what, static_cast<long long>(ms));
                    return ok;
                };
                if (frame && !timed("stage", [&] { return session->stage(std::move(*frame)); })) { error = session->error(); break; }
                if (!timed("write", [&] { return session->flush([this] { return stopRequested_.load(); }); })) { error = session->error(); break; }
            }
        }
    } catch (const std::exception& failure) { error = QString::fromUtf8(failure.what()); }
    // Publish terminal status only after all module/transport destruction has completed on this worker.
    session.reset();
    finish(error.isEmpty() ? State::Stopped : State::Faulted, error);
}

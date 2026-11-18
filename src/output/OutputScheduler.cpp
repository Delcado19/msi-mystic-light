#include "OutputScheduler.h"
#include <QThread>
#include <chrono>
#include <exception>

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
    try { worker_ = std::thread([this, factory = std::move(factory), present = std::move(present)]() mutable { run(std::move(factory), std::move(present)); }); }
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
void OutputScheduler::run(SessionFactory factory, std::function<bool()> present)
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
            // The presence check (HID enumeration, board identity, auxiliary scan) took 112-453 ms, once a
            // second per output. On this worker it froze the LEDs that long each time; the service log showed
            // 15,888 such stalls in 45 minutes (owner report 2026-11-18, subtle color jumps at rest). It runs
            // on its own thread now and only reports its verdict.
            // The probe is detached and owns its state: HID enumeration can hang, and teardown must neither
            // wait for it nor let a late verdict reach a replacement session (CodeRabbit review 2026-11-18).
            // shortcut: a probe hung in the driver stays parked until the process ends; one per reopened output.
            struct Probe { std::mutex mutex; std::condition_variable wake; bool done = false; std::atomic_bool gone{false}; QString error; };
            const auto probe = std::make_shared<Probe>();
            struct Finish { std::shared_ptr<Probe> probe;
                ~Finish() { { const std::lock_guard<std::mutex> lock(probe->mutex); probe->done = true; } probe->wake.notify_all(); } } finish{probe};
            if (present) std::thread([probe, present] {
                for (;;) {
                    {
                        std::unique_lock<std::mutex> lock(probe->mutex);
                        if (probe->wake.wait_for(lock, std::chrono::seconds(1), [&] { return probe->done; })) return;
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

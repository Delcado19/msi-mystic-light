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
quint64 OutputScheduler::start(SessionFactory factory)
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
    try { worker_ = std::thread([this, factory = std::move(factory)]() mutable { run(std::move(factory)); }); }
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
void OutputScheduler::run(SessionFactory factory)
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
            while (!stopRequested_.load()) {
                std::optional<OutputFrame> frame;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    // Pace from completion, never catch up with a burst after a slow HID call.
                    wake_.wait_for(lock, std::chrono::milliseconds(33), [this] { return stopRequested_.load(); });
                    if (stopRequested_.load()) break;
                    frame = std::move(pending_); pending_.reset();
                }
                if (frame && !session->stage(std::move(*frame))) { error = session->error(); break; }
                if (!session->flush([this] { return stopRequested_.load(); })) { error = session->error(); break; }
            }
        }
    } catch (const std::exception& failure) { error = QString::fromUtf8(failure.what()); }
    // Publish terminal status only after all module/transport destruction has completed on this worker.
    session.reset();
    finish(error.isEmpty() ? State::Stopped : State::Faulted, error);
}

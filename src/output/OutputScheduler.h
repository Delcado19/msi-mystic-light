#pragma once
#include "OutputSession.h"
#include <QObject>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

// Control start/stop on the QObject's owner thread. Submit/status are thread-safe.
// Factory, session, module calls and transport teardown all run on the worker.
class OutputScheduler : public QObject {
    Q_OBJECT
public:
    enum class State { Stopped, Starting, Running, Stopping, Faulted };
    Q_ENUM(State)
    struct Status { State state = State::Stopped; quint64 generation = 0; QString error; };
    using SessionFactory = std::function<std::unique_ptr<OutputSession>()>;
    explicit OutputScheduler(QObject* parent = nullptr);
    ~OutputScheduler() override;
    quint64 start(SessionFactory factory, std::function<bool()> present = {}); // Zero means already active or no factory.
    bool submit(quint64 generation, OutputFrame frame);
    void stop(); // Nonblocking; an in-flight factory/module call may finish.
    Status status() const;
signals:
    void statusChanged(); // Read status(); queued notifications may refer to an older generation.
private:
    void run(SessionFactory factory, std::function<bool()> present);
    void finish(State state, const QString& error = {});
    mutable std::mutex mutex_;
    std::condition_variable wake_;
    std::thread worker_;
    std::atomic_bool stopRequested_{false};
    Status status_;
    std::optional<OutputFrame> pending_; // One latest frame, never one queued event per submission.
};

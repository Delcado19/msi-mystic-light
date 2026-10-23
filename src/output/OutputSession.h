#pragma once
#include "editor/LightingDocument.h"
#include "modules/ModuleRegistry.h"
#include <optional>
#include <functional>

struct ChannelOutput {
    QString channelId;
    bool software = false;
    NativeEffect native;
    QVector<RgbColor> colors;
};
struct OutputFrame {
    QString endpoint, profileId;
    QVector<ChannelOutput> channels;
};
// Data preparation only: samples software chains once, never invokes a driver.
QVector<OutputFrame> prepareOutput(const LightingDocument& document, const QImage& composed);

// Single-thread owned. The host schedules flush(); stage() replaces rather than queues frames.
// The host must resolve conflicts, confirm output and open the exact path before handing over a transport.
class OutputSession {
public:
    enum class State { Disconnected, Connected, Faulted };
    ~OutputSession();
    OutputSession() = default;
    OutputSession(const OutputSession&) = delete;
    OutputSession& operator=(const OutputSession&) = delete;
    bool connect(const ModuleRegistry& modules, const DeviceIdentity& identity,
        const DeviceProfile& profile, std::unique_ptr<IHidDevice> openedTransport);
    void disconnect(); // Releases ownership without sending off/restore reports.
    bool stage(OutputFrame frame);
    // Cancellation is checked between driver calls; an in-flight module call cannot be interrupted.
    bool flush(const std::function<bool()>& cancelled = {});
    State state() const { return state_; }
    QString error() const { return error_; }
    bool hasPendingFrame() const { return pending_.has_value(); }
private:
    void release();
    bool fail(const QString& error);
    // Destruction/release always destroys the driver while its transport is still alive.
    std::unique_ptr<IHidDevice> transport_;
    std::unique_ptr<DriverInstance> driver_;
    DeviceIdentity identity_;
    DeviceProfile profile_;
    QMap<QString, QByteArray> appliedNative_;
    std::optional<OutputFrame> pending_;
    State state_ = State::Disconnected;
    QString error_;
};

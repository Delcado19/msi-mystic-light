#include "OutputSession.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <stdexcept>

namespace {
QByteArray nativeKey(const NativeEffect& effect)
{
    QJsonArray colors;
    for (const auto& color : effect.colors) colors.append(QJsonArray{color.r, color.g, color.b});
    return QJsonDocument(QJsonObject{{"id", effect.id}, {"colors", colors},
        {"brightness", effect.brightness}, {"speed", effect.speed}, {"inward", effect.inward}}).toJson(QJsonDocument::Compact);
}
void validateFrame(const OutputFrame& frame, const DeviceIdentity& identity, const DeviceProfile& profile)
{
    if (frame.endpoint != identity.path || frame.profileId != profile.id || frame.channels.size() > 256)
        throw std::invalid_argument("Output frame does not match the connected endpoint/profile or exceeds 256 channels");
    QSet<QString> channels;
    for (const auto& output : frame.channels) {
        if (channels.contains(output.channelId)) throw std::invalid_argument("Duplicate output channel");
        channels.insert(output.channelId);
        const auto& channel = assignmentChannel(profile, output.channelId);
        if (output.software) {
            if (!channel.direct || output.colors.isEmpty() || output.colors.size() > channel.maxLeds ||
                (channel.fixedLeds && output.colors.size() != channel.fixedLeds))
                throw std::invalid_argument("Invalid software channel frame");
        } else if (!channel.nativeModes.contains(output.native.id) || !output.colors.isEmpty() ||
            output.native.colors.isEmpty() || output.native.colors.size() > 4 ||
            output.native.brightness < 0 || output.native.brightness > 5 ||
            output.native.speed < 0 || output.native.speed > 2) {
            throw std::invalid_argument("Invalid native channel command");
        }
    }
}
}

bool sameOutputScope(const OutputFrame& approved,const OutputFrame& candidate)
{
    if(approved.endpoint!=candidate.endpoint || approved.profileId!=candidate.profileId || approved.channels.size()!=candidate.channels.size()) return false;
    for(int i=0;i<approved.channels.size();++i) {
        const auto& a=approved.channels[i]; const auto& b=candidate.channels[i];
        if(a.channelId!=b.channelId || a.software!=b.software || a.colors.size()!=b.colors.size()) return false;
        if(!a.software && nativeKey(a.native)!=nativeKey(b.native)) return false;
    }
    return true;
}

QVector<OutputFrame> prepareOutput(const LightingDocument& document, const QImage& composed, double seconds, const QVector<ComponentPlacement>* placements)
{
    const auto sampled = document.sampleChannels(composed,seconds,placements);
    QVector<OutputFrame> result;
    for (const auto& assignment : document.state().assignments) {
        int index = 0;
        while (index < result.size() && result[index].endpoint != assignment.endpoint) ++index;
        if (index == result.size()) result.append({assignment.endpoint, assignment.profileId, {}});
        ChannelOutput channel; channel.channelId = assignment.channelId;
        channel.software = assignment.software; channel.native = assignment.native;
        if (channel.software) {
            for (const auto& color : sampled.value(assignment.id))
                channel.colors.append({quint8(color.red()), quint8(color.green()), quint8(color.blue())});
        }
        result[index].channels.append(channel);
    }
    return result;
}

OutputSession::~OutputSession() { release(); }
void OutputSession::release()
{
    pending_.reset(); appliedNative_.clear();
    driver_.reset();
    if (transport_) transport_->close();
    transport_.reset();
}
void OutputSession::disconnect()
{
    release(); state_ = State::Disconnected; error_.clear();
}
bool OutputSession::fail(const QString& error)
{
    release(); state_ = State::Faulted; error_ = error; return false;
}
bool OutputSession::connect(const ModuleRegistry& modules, const DeviceIdentity& identity,
    const DeviceProfile& profile, std::unique_ptr<IHidDevice> openedTransport)
{
    disconnect();
    transport_ = std::move(openedTransport); identity_ = identity; profile_ = profile;
    if (!transport_ || !transport_->isOpen()) return fail("Connection requires an explicitly opened transport");
    QString error;
    driver_ = modules.create(identity_, profile_, *transport_, &error);
    if (!driver_) return fail(error.isEmpty() ? "Driver creation failed" : error);
    state_ = State::Connected;
    return true; // No reports here: only an explicit stage/flush begins output.
}
bool OutputSession::stage(OutputFrame frame)
{
    // Invalid replacement must not leave an older frame waiting to be transmitted.
    pending_.reset();
    if (state_ != State::Connected) {
        if (error_.isEmpty()) error_ = "Output session is not connected";
        return false;
    }
    if (!transport_->isOpen()) return fail("Transport is closed");
    try { validateFrame(frame, identity_, profile_); }
    catch (const std::exception& error) { error_ = QString::fromUtf8(error.what()); return false; }
    pending_ = std::move(frame); error_.clear(); return true;
}
bool OutputSession::flush(const std::function<bool()>& cancelled)
{
    if (state_ != State::Connected) return false;
    if (!transport_->isOpen()) return fail("Transport is closed");
    if (!pending_) return true;
    auto frame = std::move(*pending_); pending_.reset();
    try {
        for (const auto& output : frame.channels) {
            if (cancelled && cancelled()) return true;
            if (output.software) {
                if (!driver_->driver().pushColors(output.channelId, output.colors))
                    return fail("Software frame write failed for channel " + output.channelId);
                appliedNative_.remove(output.channelId);
            } else {
                const auto key = nativeKey(output.native);
                if (appliedNative_.value(output.channelId) == key) continue;
                if (!driver_->driver().setNativeMode(output.channelId, output.native))
                    return fail("Native command failed for channel " + output.channelId);
                // Cache only successful commands, never a failed or merely staged setting.
                appliedNative_.insert(output.channelId, key);
            }
        }
    } catch (const std::exception& error) { return fail(QString::fromUtf8(error.what())); }
    error_.clear(); return true;
}

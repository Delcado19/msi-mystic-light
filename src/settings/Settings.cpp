#include "settings/Settings.h"

#include <algorithm>

#include <QFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QString>

namespace {

bool colorEquals(const MsiColor& a, const MsiColor& b)
{
    return a.r == b.r && a.g == b.g && a.b == b.b;
}

bool nativeSettingsEquals(const NativeModeSettings& a, const NativeModeSettings& b)
{
    return a.lightingMode == b.lightingMode
        && colorEquals(a.color1, b.color1)
        && colorEquals(a.color2, b.color2)
        && colorEquals(a.color3, b.color3)
        && colorEquals(a.color4, b.color4)
        && a.colorCount == b.colorCount
        && a.selectAll == b.selectAll
        && a.directionInward == b.directionInward
        && a.colorSelectionUser == b.colorSelectionUser
        && a.brightness == b.brightness
        && a.speed == b.speed
        && a.cycleNum == b.cycleNum;
}

QJsonObject colorToJson(const MsiColor& c)
{
    QJsonObject o;
    o["r"] = c.r;
    o["g"] = c.g;
    o["b"] = c.b;
    return o;
}

// Missing/wrong-type fields fall back to NativeModeSettings' own defaults
// (via the default-constructed `s`) rather than failing the whole file —
// see the Review Focus note in Settings.h: a hand-edited settings file
// that's valid JSON but incomplete must not be treated as corrupted.
MsiColor colorFromJson(const QJsonValue& v, const MsiColor& fallback = {})
{
    if (!v.isObject()) return fallback;
    QJsonObject o = v.toObject();
    MsiColor c = fallback;
    c.r = static_cast<uint8_t>(o.value("r").toInt(c.r));
    c.g = static_cast<uint8_t>(o.value("g").toInt(c.g));
    c.b = static_cast<uint8_t>(o.value("b").toInt(c.b));
    return c;
}

QJsonObject nativeSettingsToJson(const NativeModeSettings& s)
{
    QJsonObject o;
    o["lightingMode"] = static_cast<int>(s.lightingMode);
    o["color1"] = colorToJson(s.color1);
    o["color2"] = colorToJson(s.color2);
    o["color3"] = colorToJson(s.color3);
    o["color4"] = colorToJson(s.color4);
    o["colorCount"] = s.colorCount;
    o["selectAll"] = s.selectAll;
    o["directionInward"] = s.directionInward;
    o["colorSelectionUser"] = s.colorSelectionUser;
    o["brightness"] = s.brightness;
    o["speed"] = s.speed;
    o["cycleNum"] = s.cycleNum;
    return o;
}

// A hand-edited settings.json reaches the real board within ~16ms of
// launch (Canvas sends any pending native-mode channel on the first
// tick), so out-of-range values here are "unconfirmed bytes to hardware"
// in exactly the sense the project's safety rule exists to prevent. Each
// field is clamped/rejected to the range MsiProtocol.cpp's bit-packing
// assumes (see docs/MB_800_HID_PROTOCOL.md / Task 2's MsiZoneSettings
// comments): lightingMode must be a real enum value or it falls back to
// the default rather than encoding garbage into report 0x50's first byte.
MsiLightingMode clampLightingMode(int raw, MsiLightingMode fallback)
{
    return (raw >= static_cast<int>(MsiLightingMode::Off) && raw <= static_cast<int>(MsiLightingMode::GodLike))
        ? static_cast<MsiLightingMode>(raw)
        : fallback;
}

NativeModeSettings nativeSettingsFromJson(const QJsonValue& v)
{
    NativeModeSettings s{}; // defaults used for any field missing below
    if (!v.isObject()) return s;
    QJsonObject o = v.toObject();
    s.lightingMode = clampLightingMode(o.value("lightingMode").toInt(static_cast<int>(s.lightingMode)), s.lightingMode);
    s.color1 = colorFromJson(o.value("color1"), s.color1);
    s.color2 = colorFromJson(o.value("color2"), s.color2);
    s.color3 = colorFromJson(o.value("color3"), s.color3);
    s.color4 = colorFromJson(o.value("color4"), s.color4);
    s.colorCount = static_cast<uint8_t>(std::clamp(o.value("colorCount").toInt(s.colorCount), 1, 4));
    s.selectAll = o.value("selectAll").toBool(s.selectAll);
    s.directionInward = o.value("directionInward").toBool(s.directionInward);
    s.colorSelectionUser = o.value("colorSelectionUser").toBool(s.colorSelectionUser);
    s.brightness = static_cast<uint8_t>(std::clamp(o.value("brightness").toInt(s.brightness), 0, 5));
    s.speed = static_cast<uint8_t>(std::clamp(o.value("speed").toInt(s.speed), 0, 2));
    s.cycleNum = static_cast<uint8_t>(o.value("cycleNum").toInt(s.cycleNum));
    return s;
}

QJsonObject channelSettingsToJson(const ChannelSettings& cs)
{
    QJsonObject o;
    o["channelName"] = QString::fromStdString(cs.channelName);
    o["mode"] = (cs.mode == ChannelMode::Canvas) ? QStringLiteral("Canvas") : QStringLiteral("Native");
    o["effectName"] = QString::fromStdString(cs.effectName);
    o["staticColor"] = colorToJson(cs.staticColor);
    o["nativeSettings"] = nativeSettingsToJson(cs.nativeSettings);
    return o;
}

// Builds one ChannelSettings from a channel JSON object, defaulting any
// field that's absent or the wrong type instead of rejecting the entry.
ChannelSettings channelSettingsFromJson(const QJsonObject& o)
{
    ChannelSettings cs;
    cs.channelName = o.value("channelName").toString().toStdString();
    cs.mode = (o.value("mode").toString() == QStringLiteral("Canvas")) ? ChannelMode::Canvas : ChannelMode::Native;
    cs.effectName = o.value("effectName").toString().toStdString();
    cs.staticColor = colorFromJson(o.value("staticColor"), cs.staticColor);
    cs.nativeSettings = nativeSettingsFromJson(o.value("nativeSettings"));
    return cs;
}

} // namespace

bool ChannelSettings::operator==(const ChannelSettings& other) const
{
    return channelName == other.channelName
        && mode == other.mode
        && effectName == other.effectName
        && colorEquals(staticColor, other.staticColor)
        && nativeSettingsEquals(nativeSettings, other.nativeSettings);
}

bool Settings::saveToFile(const std::string& path) const
{
    QJsonArray channels;
    for (const auto& cs : channelSettings) channels.append(channelSettingsToJson(cs));

    QJsonObject root;
    root["channels"] = channels;

    QFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false; // I/O failure, e.g. directory doesn't exist

    const QByteArray bytes = QJsonDocument(root).toJson();
    const bool ok = file.write(bytes) == bytes.size();
    file.close();
    return ok;
}

// Review Focus (owned by this task): the settings file can be missing on
// first run, or hand-edited into something broken. Both must fall back to
// defaults rather than crash, since "restores last-applied config on
// launch" is a stated goal. channelSettings is only overwritten once the
// whole file has been confirmed to be a well-formed {"channels": [...]}
// document; per-entry field defaulting happens in channelSettingsFromJson
// above, and non-object array entries are skipped, so a partially-edited
// file loads as much as it validly can instead of failing outright.
bool Settings::loadFromFile(const std::string& path)
{
    QFile file(QString::fromStdString(path));
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) return false;
    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError error{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) return false;

    const QJsonValue channelsValue = doc.object().value("channels");
    if (!channelsValue.isArray()) return false;

    std::vector<ChannelSettings> parsed;
    const QJsonArray channelsArray = channelsValue.toArray();
    parsed.reserve(static_cast<size_t>(channelsArray.size()));
    for (const QJsonValue& entry : channelsArray) {
        if (!entry.isObject()) continue; // tolerate a malformed entry rather than failing the whole file
        parsed.push_back(channelSettingsFromJson(entry.toObject()));
    }

    channelSettings = std::move(parsed);
    return true;
}

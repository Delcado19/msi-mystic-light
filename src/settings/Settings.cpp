#include "settings/Settings.h"

#include <algorithm>
#include <cmath>

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

CanvasSource sourceFromJson(const QJsonValue& v, CanvasSource fallback)
{
    if (!v.isString()) return fallback;
    const QString s = v.toString();
    if (s == QStringLiteral("GlobalFollow")) return CanvasSource::GlobalFollow;
    if (s == QStringLiteral("Independent")) return CanvasSource::Independent;
    return fallback;
}

QString sourceToJson(CanvasSource s)
{
    return (s == CanvasSource::GlobalFollow) ? QStringLiteral("GlobalFollow") : QStringLiteral("Independent");
}

QJsonObject placementToJson(const CanvasPlacement& p)
{
    QJsonObject o;
    o["x"] = p.x;
    o["y"] = p.y;
    return o;
}

// Clamp range for a loaded placement coordinate. Canvas::tick() does
// `static_cast<int>(assignment.placement.x + x)` (Canvas.cpp) -- a
// placement value too large/non-finite to survive that cast is undefined
// behavior, not just a visually-wrong position. This is the deferred item
// from Task 3's review ("add a clamp when the persistence task (13) starts
// feeding real user data into placement"); JSON can't encode NaN/Infinity
// but can encode e.g. 1e300, so both the finite check and the range clamp
// are needed. The bound is generous for any plausible canvas UI while
// leaving headroom under INT_MAX once a local LED coordinate is added.
constexpr double kMaxPlacementCoord = 1'000'000.0;

double clampPlacementCoord(double raw, double fallback)
{
    if (!std::isfinite(raw)) return fallback;
    return std::clamp(raw, -kMaxPlacementCoord, kMaxPlacementCoord);
}

CanvasPlacement placementFromJson(const QJsonValue& v, const CanvasPlacement& fallback = {})
{
    if (!v.isObject()) return fallback;
    QJsonObject o = v.toObject();
    CanvasPlacement p = fallback;
    p.x = clampPlacementCoord(o.value("x").toDouble(p.x), p.x);
    p.y = clampPlacementCoord(o.value("y").toDouble(p.y), p.y);
    return p;
}

QJsonObject channelSettingsToJson(const ChannelSettings& cs)
{
    QJsonObject o;
    o["channelName"] = QString::fromStdString(cs.channelName);
    o["mode"] = (cs.mode == ChannelMode::Canvas) ? QStringLiteral("Canvas") : QStringLiteral("Native");
    o["source"] = sourceToJson(cs.source);
    o["placement"] = placementToJson(cs.placement);
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
    cs.source = sourceFromJson(o.value("source"), cs.source);
    cs.placement = placementFromJson(o.value("placement"), cs.placement);
    cs.effectName = o.value("effectName").toString().toStdString();
    cs.staticColor = colorFromJson(o.value("staticColor"), cs.staticColor);
    cs.nativeSettings = nativeSettingsFromJson(o.value("nativeSettings"));
    return cs;
}

QJsonObject globalEffectSettingsToJson(const GlobalEffectSettings& g)
{
    QJsonObject o;
    o["effectName"] = QString::fromStdString(g.effectName);
    o["color"] = colorToJson(g.color);
    o["referenceX"] = g.referenceX;
    o["referenceY"] = g.referenceY;
    return o;
}

GlobalEffectSettings globalEffectSettingsFromJson(const QJsonValue& v)
{
    GlobalEffectSettings g{};
    if (!v.isObject()) return g;
    QJsonObject o = v.toObject();
    g.effectName = o.value("effectName").toString().toStdString();
    g.color = colorFromJson(o.value("color"), g.color);
    g.referenceX = o.value("referenceX").toInt(g.referenceX);
    g.referenceY = o.value("referenceY").toInt(g.referenceY);
    return g;
}

QJsonObject profileToJson(const Profile& p)
{
    QJsonArray channels;
    for (const auto& cs : p.channelSettings) channels.append(channelSettingsToJson(cs));

    QJsonObject o;
    o["name"] = QString::fromStdString(p.name);
    o["channels"] = channels;
    o["globalEffect"] = globalEffectSettingsToJson(p.globalEffect);
    return o;
}

// Builds one Profile from a profile JSON object, tolerating a missing/
// non-array "channels" as an empty channel list rather than failing the
// whole profile -- same "default the field" tolerance as the channel-level
// helpers above.
Profile profileFromJson(const QJsonObject& o)
{
    Profile p;
    p.name = o.value("name").toString().toStdString();
    const QJsonValue channelsValue = o.value("channels");
    if (channelsValue.isArray()) {
        const QJsonArray channelsArray = channelsValue.toArray();
        p.channelSettings.reserve(static_cast<size_t>(channelsArray.size()));
        for (const QJsonValue& entry : channelsArray) {
            if (!entry.isObject()) continue; // tolerate a malformed entry rather than failing the whole profile
            p.channelSettings.push_back(channelSettingsFromJson(entry.toObject()));
        }
    }
    p.globalEffect = globalEffectSettingsFromJson(o.value("globalEffect"));
    return p;
}

} // namespace

bool ChannelSettings::operator==(const ChannelSettings& other) const
{
    return channelName == other.channelName
        && mode == other.mode
        && source == other.source
        && placement.x == other.placement.x
        && placement.y == other.placement.y
        && effectName == other.effectName
        && colorEquals(staticColor, other.staticColor)
        && nativeSettingsEquals(nativeSettings, other.nativeSettings);
}

bool GlobalEffectSettings::operator==(const GlobalEffectSettings& other) const
{
    return effectName == other.effectName
        && colorEquals(color, other.color)
        && referenceX == other.referenceX
        && referenceY == other.referenceY;
}

bool Profile::operator==(const Profile& other) const
{
    return name == other.name
        && channelSettings == other.channelSettings
        && globalEffect == other.globalEffect;
}

bool Settings::saveToFile(const std::string& path) const
{
    QJsonArray profilesArray;
    for (const auto& p : profiles) profilesArray.append(profileToJson(p));

    QJsonObject root;
    root["profiles"] = profilesArray;
    root["activeProfile"] = QString::fromStdString(activeProfileName);

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
// launch" is a stated goal. profiles/activeProfileName are only overwritten
// once the whole file has been confirmed to be well-formed JSON; per-entry
// field defaulting happens in channelSettingsFromJson/profileFromJson
// above, and non-object array entries are skipped, so a partially-edited
// file loads as much as it validly can instead of failing outright.
//
// Migration (Task 13): a v1-style file -- top-level {"channels": [...]},
// no "profiles" key -- predates the multi-profile format added here. It
// loads as a single Profile named "Default" holding those channels, so
// existing users' settings.json keeps working unchanged after upgrading.
bool Settings::loadFromFile(const std::string& path)
{
    QFile file(QString::fromStdString(path));
    if (!file.exists() || !file.open(QIODevice::ReadOnly)) return false;
    const QByteArray data = file.readAll();
    file.close();

    QJsonParseError error{};
    const QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    if (error.error != QJsonParseError::NoError || !doc.isObject()) return false;

    const QJsonObject root = doc.object();

    if (!root.contains("profiles")) {
        // v1 migration path: no "profiles" key means this is (or claims to
        // be) a pre-profiles file. Still require "channels" to be an array,
        // matching v1's own structural-validity check, so garbage JSON
        // ({} or {"channels": "nope"}) is rejected exactly as it was before.
        if (!root.value("channels").isArray()) return false;

        // profileFromJson reads "channels" the same way a real profile
        // object would; only the name differs, so reuse it instead of
        // duplicating the array-walk/per-entry-defaulting logic.
        Profile defaultProfile = profileFromJson(root);
        defaultProfile.name = "Default";

        profiles = {std::move(defaultProfile)};
        activeProfileName = "Default";
        return true;
    }

    const QJsonValue profilesValue = root.value("profiles");
    if (!profilesValue.isArray()) return false;

    std::vector<Profile> parsed;
    const QJsonArray profilesArray = profilesValue.toArray();
    parsed.reserve(static_cast<size_t>(profilesArray.size()));
    for (const QJsonValue& entry : profilesArray) {
        if (!entry.isObject()) continue; // tolerate a malformed entry rather than failing the whole file
        parsed.push_back(profileFromJson(entry.toObject()));
    }

    profiles = std::move(parsed);
    activeProfileName = root.value("activeProfile").toString().toStdString();
    return true;
}

// Review Focus: activeProfileName can name a profile that no longer exists
// (e.g. it was renamed/deleted in a hand-edited file, or elsewhere in the
// app) -- falling back to the first profile instead of returning nullptr
// keeps the caller from treating a merely-stale name as "no profiles at
// all".
Profile* Settings::activeProfile()
{
    if (profiles.empty()) return nullptr;
    for (auto& p : profiles) {
        if (p.name == activeProfileName) return &p;
    }
    return &profiles.front();
}

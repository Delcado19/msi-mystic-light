#include "settings/Settings.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

#include <QFile>
#include <QSaveFile>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>
#include <QString>

namespace {

void writeEffectLayers(QJsonObject& item,const CustomEffectDefinition& d,bool overlays=true) {
    item["sourceMotion"]=QString::fromStdString(d.sourceMotion);item["centerX"]=d.centerX;item["centerY"]=d.centerY;item["enabled"]=d.enabled;item["reverse"]=d.reverse;item["spreadDegrees"]=d.spreadDegrees;
    item["angle"]=d.angle;item["opacity"]=d.opacity;item["blend"]=QString::fromStdString(d.blend);
    item["zoneEnabled"]=d.zoneEnabled;item["zoneX"]=d.zoneX;item["zoneY"]=d.zoneY;item["zoneWidth"]=d.zoneWidth;item["zoneHeight"]=d.zoneHeight;
    QJsonArray layers;
    if(overlays)for(const auto& layer:d.layers) {
        QJsonObject value;value["name"]=QString::fromStdString(layer.name);value["pattern"]=QString::fromStdString(layer.pattern);
        value["periodMs"]=layer.periodMs;value["wavelength"]=layer.wavelength;
        QJsonArray colors;for(auto c:layer.colors){QJsonObject color;color["r"]=c.r;color["g"]=c.g;color["b"]=c.b;colors.append(color);}value["colors"]=colors;
        QJsonArray stops;for(auto stop:layer.stops)stops.append(stop);value["stops"]=stops;
        writeEffectLayers(value,layer,false);layers.append(value);
    }
    item["layers"]=layers;
}
void readEffectLayers(const QJsonObject& item,CustomEffectDefinition& d,bool overlays=true) {
    const auto number=[&](const char* key,double fallback,double low,double high){const double v=item.value(key).toDouble(fallback);return std::isfinite(v)?std::clamp(v,low,high):fallback;};
    d.sourceMotion=item.value("sourceMotion").toString("Sine").toStdString();if(d.sourceMotion!="Sine" && d.sourceMotion!="Cosine" && d.sourceMotion!="Ping-pong" && d.sourceMotion!="Static")d.sourceMotion="Sine";
    d.centerX=int(number("centerX",550,0,1100));d.centerY=int(number("centerY",350,0,700));d.enabled=item.value("enabled").toBool(true);d.reverse=item.value("reverse").toBool();d.spreadDegrees=number("spreadDegrees",360,15,720);
    d.angle=number("angle",0,-360,360);d.opacity=number("opacity",1,0,1);
    d.blend=item.value("blend").toString("Screen").toStdString();if(d.blend!="Screen" && d.blend!="Add" && d.blend!="Normal")d.blend="Screen";
    d.zoneEnabled=item.value("zoneEnabled").toBool();d.zoneX=int(number("zoneX",0,0,1100));d.zoneY=int(number("zoneY",0,0,700));
    d.zoneWidth=int(number("zoneWidth",1100,1,1100));d.zoneHeight=int(number("zoneHeight",700,1,700));
    if(overlays)for(const auto& value:item.value("layers").toArray()) {
        if(d.layers.size()>=8)break;const auto o=value.toObject();CustomEffectDefinition layer;
        layer.name=o.value("name").toString("Layer").left(80).toStdString();layer.path="Canvas positions";
        layer.pattern=o.value("pattern").toString("Scroll").toStdString();if(!CustomEffectDefinition::validPattern(layer.pattern))layer.pattern="Scroll";
        layer.periodMs=std::clamp(o.value("periodMs").toInt(6000),500,120000);layer.wavelength=std::clamp(o.value("wavelength").toInt(720),40,4000);
        for(auto color:o.value("colors").toArray()){if(layer.colors.size()>=16)break;const auto c=color.toObject();layer.colors.push_back({uint8_t(std::clamp(c.value("r").toInt(),0,255)),uint8_t(std::clamp(c.value("g").toInt(),0,255)),uint8_t(std::clamp(c.value("b").toInt(),0,255))});}
        if(layer.colors.size()<2)continue;
        for(auto stop:o.value("stops").toArray())layer.stops.push_back(stop.toDouble(-1));if(!layer.validStops())layer.stops.clear();
        readEffectLayers(o,layer,false);d.layers.push_back(std::move(layer));
    }
}

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
    o["y"] = p.y;o["width"]=p.width;o["height"]=p.height;o["rotation"]=p.rotation;
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
    p.width=std::clamp(o.value("width").toDouble(70),10.,2000.);
    p.height=std::clamp(o.value("height").toDouble(40),10.,2000.);
    p.rotation=std::clamp(o.value("rotation").toDouble(0),-360.,360.);
    return p;
}

QJsonObject channelSettingsToJson(const ChannelSettings& cs)
{
    QJsonObject o;
    o["channelName"] = QString::fromStdString(cs.channelName);
    o["layoutVisible"]=cs.layoutVisible;
    o["mode"] = (cs.mode == ChannelMode::Canvas) ? QStringLiteral("Canvas") : QStringLiteral("Native");
    o["source"] = sourceToJson(cs.source);
    o["placement"] = placementToJson(cs.placement);
    o["effectName"] = QString::fromStdString(cs.effectName);
    o["staticColor"] = colorToJson(cs.staticColor);
    o["nativeSettings"] = nativeSettingsToJson(cs.nativeSettings);
    QJsonArray components;
    for (const auto& component : cs.components) components.append(component.toJson());
    o["components"] = components;
    o["componentsConfigured"]=cs.hasCustomComponents();
    QJsonObject overrides;
    for (const auto& [index, color] : cs.ledOverrides) overrides[QString::number(index)] = colorToJson(color);
    o["ledOverrides"] = overrides;

    return o;
}

// Builds one ChannelSettings from a channel JSON object, defaulting any
// field that's absent or the wrong type instead of rejecting the entry.
ChannelSettings channelSettingsFromJson(const QJsonObject& o)
{
    ChannelSettings cs;
    cs.channelName = o.value("channelName").toString().toStdString();
    cs.layoutVisible=o.value("layoutVisible").toBool(true);
    cs.mode = (o.value("mode").toString() == QStringLiteral("Canvas")) ? ChannelMode::Canvas : ChannelMode::Native;
    cs.source = sourceFromJson(o.value("source"), cs.source);
    cs.placement = placementFromJson(o.value("placement"), cs.placement);
    cs.effectName = o.value("effectName").toString().toStdString();
    cs.staticColor = colorFromJson(o.value("staticColor"), cs.staticColor);
    cs.nativeSettings = nativeSettingsFromJson(o.value("nativeSettings"));
    const bool header = cs.channelName == "JARGB1" || cs.channelName == "JARGB2" || cs.channelName == "JARGB3" || cs.channelName == "EZConnector";
    int total = 0;
    const int capacity=header?120:cs.channelName=="RoccatVulcan"?144:cs.channelName=="SteelSeriesApex3"?10:cs.channelName.rfind("XpgRam",0)==0?8:(cs.channelName=="GigabyteAero5080" || cs.channelName=="IOShield")?1:11;
    cs.componentsConfigured=o.value("componentsConfigured").toBool(false);
    for (const auto& value : o.value("components").toArray()) {
        try {
            auto component = Component::fromJson(value.toObject());
            if (component.ledCount <= 0 || component.ledCount > capacity - total) { cs.components.clear();cs.componentsConfigured=false; break; }
            total += component.ledCount;
            cs.components.push_back(std::move(component));
        } catch (const std::invalid_argument&) { cs.components.clear();cs.componentsConfigured=false; break; }
    }
    if(!header && !cs.components.empty() && total!=capacity){cs.components.clear();cs.componentsConfigured=false;}
    const auto overrides = o.value("ledOverrides").toObject();
    for (auto it = overrides.begin(); it != overrides.end(); ++it) {
        bool ok = false;
        const int index = it.key().toInt(&ok);
        if (ok && index >= 0 && index < 144 && it.value().isObject()) cs.ledOverrides[index] = colorFromJson(it.value());
    }

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
    // Same clamp class as placementFromJson's x/y above (and the same
    // reasoning: a hand-edited settings.json can encode e.g. 2e9, which
    // overflows int and is UB when Spiral/SwirlCircles/RadialRainbow compute
    // `x - centerX_`). Read via toDouble() rather than toInt() so an
    // out-of-int-range JSON number is clamped here instead of converted
    // first, then clamped to kMaxPlacementCoord's range (same generous bound,
    // reused rather than duplicated).
    g.referenceX = static_cast<int>(clampPlacementCoord(o.value("referenceX").toDouble(g.referenceX), g.referenceX));
    g.referenceY = static_cast<int>(clampPlacementCoord(o.value("referenceY").toDouble(g.referenceY), g.referenceY));
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
    if (components.size() != other.components.size() || ledOverrides.size() != other.ledOverrides.size()) return false;
    for (size_t i = 0; i < components.size(); ++i) if (components[i].toJson() != other.components[i].toJson()) return false;
    for (const auto& [index, color] : ledOverrides) {
        const auto it = other.ledOverrides.find(index);
        if (it == other.ledOverrides.end() || !colorEquals(color, it->second)) return false;
    }

    return layoutVisible==other.layoutVisible && channelName == other.channelName
        && mode == other.mode
        && source == other.source
        && placement.x == other.placement.x
        && placement.y == other.placement.y
        && placement.width==other.placement.width && placement.height==other.placement.height && placement.rotation==other.placement.rotation
        && hasCustomComponents()==other.hasCustomComponents()
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
    QJsonArray custom;
    for (const auto& definition : customEffects) {
        QJsonArray colors;
        for (const auto& color : definition.colors) colors.append(colorToJson(color));
        QJsonObject item;
        item["name"] = QString::fromStdString(definition.name);
        item["colors"] = colors;
        item["periodMs"] = definition.periodMs;
        item["wavelength"] = definition.wavelength;
        item["pattern"] = QString::fromStdString(definition.pattern);
        item["path"]=QString::fromStdString(definition.path);
        QJsonArray stops;for(double stop:definition.stops)stops.append(stop);item["stops"]=stops;
        writeEffectLayers(item,definition);
        custom.append(item);
    }
    root["customEffects"] = custom;
    QJsonArray catalog;for(const auto& c:hardwareCatalog)catalog.append(c.toJson());root["hardwareCatalog"]=catalog;

    root["activeProfile"] = QString::fromStdString(activeProfileName);

    QSaveFile file(QString::fromStdString(path));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) return false; // I/O failure, e.g. directory doesn't exist

    const QByteArray bytes = QJsonDocument(root).toJson();
    // Save component chains and user-created recipes atomically with profiles.
    if (file.write(bytes) != bytes.size()) return false;
    return file.commit();
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
        customEffects.clear();hardwareCatalog.clear();
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

    std::vector<CustomEffectDefinition> parsedEffects;
    for (const auto& value : root.value("customEffects").toArray()) {
        if (parsedEffects.size() >= 64) break;
        const auto item = value.toObject();
        CustomEffectDefinition definition;
        definition.name = item.value("name").toString().trimmed().left(80).toStdString();
        if (definition.name.empty() || std::any_of(parsedEffects.begin(), parsedEffects.end(), [&](const auto& d) { return d.name == definition.name; })) continue;
        definition.periodMs = std::clamp(item.value("periodMs").toInt(6000), 500, 120000);
        definition.wavelength = std::clamp(item.value("wavelength").toInt(720), 40, 4000);
        for (const auto& color : item.value("colors").toArray()) {
            if (definition.colors.size() >= 16) break;
            if (color.isObject()) definition.colors.push_back(colorFromJson(color));
        }
        definition.path=item.value("path").toString("Component LEDs").toStdString();
        if(!CustomEffectDefinition::validPath(definition.path))definition.path="Component LEDs";
        definition.pattern=item.value("pattern").toString("Wave").toStdString();
        if(!CustomEffectDefinition::validPattern(definition.pattern))definition.pattern="Wave";
        for(const auto& stop:item.value("stops").toArray())definition.stops.push_back(stop.toDouble(-1));
        if(!definition.validStops())definition.stops.clear(); // Corrupt positions keep the usable legacy palette.
        readEffectLayers(item,definition);
        if (definition.colors.size() >= 2) parsedEffects.push_back(std::move(definition));
    }
    std::vector<Component> catalog;
    for(const auto& value:root.value("hardwareCatalog").toArray()) {
        if(catalog.size()>=128)break;
        try {auto c=Component::fromJson(value.toObject());if(c.valid() && c.ledCount<=120 && !c.productName.empty())catalog.push_back(std::move(c));}catch(const std::invalid_argument&){}
    }
    hardwareCatalog=std::move(catalog);
    customEffects = std::move(parsedEffects);
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

bool Settings::addProfile(const std::string& name)
{
    if (name.empty()) return false;
    for (const Profile& p : profiles) {
        if (p.name == name) return false; // duplicate name: would make one of the two unreachable via activeProfile()
    }
    Profile p;
    p.name = name;
    profiles.push_back(std::move(p));
    return true;
}

bool Settings::renameActiveProfile(const std::string& newName)
{
    if (newName.empty()) return false;
    Profile* active = activeProfile();
    if (!active) return false;
    if (newName != active->name) {
        for (const Profile& p : profiles) {
            if (p.name == newName) return false; // collision with a DIFFERENT existing profile
        }
    }
    active->name = newName;
    activeProfileName = newName; // keep it findable by name after the rename
    return true;
}

bool Settings::removeActiveProfile()
{
    // activeProfile()'s own contract (see its doc comment) assumes profiles
    // is never empty elsewhere in this codebase -- refuse to produce an
    // empty list rather than honoring a delete that would.
    if (profiles.size() <= 1) return false;
    Profile* active = activeProfile();
    if (!active) return false;

    // Index-based erase (via pointer arithmetic into the vector's own
    // storage), not a name-based std::remove_if: a hand-edited settings file
    // could in principle contain duplicate profile names, and this must
    // remove exactly the one Profile object `active` points to, never a
    // same-named sibling.
    const auto index = static_cast<std::vector<Profile>::difference_type>(active - profiles.data());
    profiles.erase(profiles.begin() + index);
    activeProfileName = profiles.front().name; // a neighbor becomes active
    return true;
}

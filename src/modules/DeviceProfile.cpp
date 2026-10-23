#include "DeviceProfile.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QSet>
#include <cmath>
#include <stdexcept>

namespace {
int integer(const QJsonObject& object, const QString& key, int minimum, int maximum)
{
    const auto value = object.value(key);
    const double number = value.toDouble(-1);
    if (!value.isDouble() || !std::isfinite(number) || std::floor(number) != number ||
        number < minimum || number > maximum)
        throw std::invalid_argument(("Invalid integer: " + key).toStdString());
    return static_cast<int>(number);
}

QString text(const QJsonObject& object, const QString& key)
{
    const auto value = object.value(key);
    if (!value.isString() || value.toString().trimmed().isEmpty())
        throw std::invalid_argument(("Missing string: " + key).toStdString());
    return value.toString().trimmed();
}
}

DeviceProfile DeviceProfile::fromJson(const QJsonObject& json)
{
    integer(json, "schemaVersion", 1, 1);
    DeviceProfile profile;
    profile.id = text(json, "id");
    profile.driverId = text(json, "driverId");
    profile.name = text(json, "name");
    if (!json.value("match").isObject()) throw std::invalid_argument("Missing match object");
    profile.match = json.value("match").toObject();
    const QSet<QString> matchKeys{"vendorId", "productId", "usagePage", "usage",
        "interfaceNumber", "boardManufacturer", "boardModel"};
    // A misspelled selector must not silently broaden a hardware match.
    for (auto key = profile.match.begin(); key != profile.match.end(); ++key)
        if (!matchKeys.contains(key.key())) throw std::invalid_argument("Unknown match selector");
    integer(profile.match, "vendorId", 1, 65535);
    integer(profile.match, "productId", 1, 65535);
    for (const auto& key : {"usagePage", "usage", "interfaceNumber"}) {
        if (profile.match.contains(key))
            integer(profile.match, key, 0, QString(key) == "interfaceNumber" ? 255 : 65535);
    }
    for (const auto& key : {"boardManufacturer", "boardModel"}) {
        if (profile.match.contains(key)) text(profile.match, key);
    }
    if (!json.value("channels").isArray() || json.value("channels").toArray().isEmpty())
        throw std::invalid_argument("Profile requires channels");
    QSet<QString> ids;
    for (const auto value : json.value("channels").toArray()) {
        if (!value.isObject()) throw std::invalid_argument("Invalid channel object");
        const auto object = value.toObject();
        ChannelProfile channel;
        channel.id = text(object, "id");
        if (ids.contains(channel.id)) throw std::invalid_argument("Duplicate channel ID");
        ids.insert(channel.id);
        channel.maxLeds = integer(object, "maxLeds", 1, 65535);
        if (object.contains("fixedLeds")) channel.fixedLeds = integer(object, "fixedLeds", 1, channel.maxLeds);
        if (!object.value("direct").isBool() || !object.value("nativeModes").isArray() ||
            !object.value("address").isObject()) throw std::invalid_argument("Invalid channel capabilities");
        channel.direct = object.value("direct").toBool();
        for (const auto mode : object.value("nativeModes").toArray()) {
            if (!mode.isString() || mode.toString().isEmpty() || channel.nativeModes.contains(mode.toString()))
                throw std::invalid_argument("Invalid or duplicate native mode");
            channel.nativeModes.append(mode.toString());
        }
        if (!channel.direct && channel.nativeModes.isEmpty())
            throw std::invalid_argument("Channel has no supported mode");
        channel.address = object.value("address").toObject();
        profile.channels.append(channel);
    }
    return profile;
}

DeviceProfile DeviceProfile::load(const QString& fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 1024 * 1024)
        throw std::invalid_argument("Cannot read profile or profile exceeds 1 MiB");
    QJsonParseError error;
    const auto document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject())
        throw std::invalid_argument("Malformed profile JSON");
    return fromJson(document.object());
}

bool DeviceProfile::matches(const DeviceIdentity& device) const
{
    if (match.value("vendorId").toInt() != device.vendorId ||
        match.value("productId").toInt() != device.productId) return false;
    for (const auto& pair : {qMakePair(QString("usagePage"), int(device.usagePage)),
                            qMakePair(QString("usage"), int(device.usage)),
                            qMakePair(QString("interfaceNumber"), device.interfaceNumber)}) {
        if (match.contains(pair.first) && match.value(pair.first).toInt() != pair.second) return false;
    }
    for (const auto& pair : {qMakePair(QString("boardManufacturer"), device.boardManufacturer),
                            qMakePair(QString("boardModel"), device.boardModel)}) {
        if (match.contains(pair.first) &&
            match.value(pair.first).toString().trimmed().compare(pair.second.trimmed(), Qt::CaseInsensitive) != 0)
            return false;
    }
    return true;
}

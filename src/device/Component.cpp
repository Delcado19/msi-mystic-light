#include "device/Component.h"

#include <stdexcept>

#include <QJsonArray>

namespace {

// Truncates (if longer than ledCount) or throws (if shorter than ledCount).
// Shared by the three per-LED arrays so the tolerance rule in the header
// doc comment is enforced identically for all of them.
template <typename T>
void truncateOrThrow(std::vector<T>& values, int ledCount, const char* fieldName)
{
    if (static_cast<int>(values.size()) < ledCount) {
        throw std::invalid_argument(std::string(fieldName) + " is shorter than LedCount");
    }
    values.resize(ledCount);
}

} // namespace

Component Component::fromJson(const QJsonObject& obj)
{
    Component c;
    c.productName = obj["ProductName"].toString().toStdString();
    c.displayName = obj["DisplayName"].toString().toStdString();
    c.brand = obj["Brand"].toString().toStdString();
    c.type = obj["Type"].toString().toStdString();
    c.ledCount = obj["LedCount"].toInt();
    if (c.ledCount < 0) {
        // Guards std::vector::resize(negative) below, which would convert to
        // a huge size_t and throw std::length_error instead of the
        // std::invalid_argument this function's contract promises.
        throw std::invalid_argument("LedCount must not be negative");
    }
    c.width = obj["Width"].toInt();
    c.height = obj["Height"].toInt();
    c.imageUrl = obj["ImageUrl"].toString().toStdString();

    const QJsonArray mappingArr = obj["LedMapping"].toArray();
    for (const QJsonValue& v : mappingArr) {
        c.ledMapping.push_back(v.toInt());
    }

    const QJsonArray coordsArr = obj["LedCoordinates"].toArray();
    for (const QJsonValue& v : coordsArr) {
        const QJsonArray pair = v.toArray();
        c.ledCoordinates.emplace_back(pair.at(0).toInt(), pair.at(1).toInt());
    }

    const QJsonArray namesArr = obj["LedNames"].toArray();
    for (const QJsonValue& v : namesArr) {
        c.ledNames.push_back(v.toString().toStdString());
    }

    truncateOrThrow(c.ledMapping, c.ledCount, "LedMapping");
    truncateOrThrow(c.ledCoordinates, c.ledCount, "LedCoordinates");
    truncateOrThrow(c.ledNames, c.ledCount, "LedNames");

    return c;
}

QJsonObject Component::toJson() const
{
    QJsonObject obj;
    obj["ProductName"] = QString::fromStdString(productName);
    obj["DisplayName"] = QString::fromStdString(displayName);
    obj["Brand"] = QString::fromStdString(brand);
    obj["Type"] = QString::fromStdString(type);
    obj["LedCount"] = ledCount;
    obj["Width"] = width;
    obj["Height"] = height;
    obj["ImageUrl"] = QString::fromStdString(imageUrl);

    QJsonArray mappingArr;
    for (int v : ledMapping) {
        mappingArr.append(v);
    }
    obj["LedMapping"] = mappingArr;

    QJsonArray coordsArr;
    for (const auto& [x, y] : ledCoordinates) {
        coordsArr.append(QJsonArray{x, y});
    }
    obj["LedCoordinates"] = coordsArr;

    QJsonArray namesArr;
    for (const std::string& name : ledNames) {
        namesArr.append(QString::fromStdString(name));
    }
    obj["LedNames"] = namesArr;

    return obj;
}

Component Component::makeGenericPlaceholder(int ledCount)
{
    Component c;
    c.productName = "Generic Placeholder";
    c.displayName = "Generic Placeholder";
    c.ledCount = ledCount;
    c.width = ledCount;
    c.height = 1;

    for (int i = 0; i < ledCount; ++i) {
        c.ledMapping.push_back(i);
        c.ledCoordinates.emplace_back(i, 0);
        c.ledNames.push_back("LED " + std::to_string(i + 1));
    }

    return c;
}

#include "device/Component.h"

#include <stdexcept>
#include <cmath>
#include <algorithm>

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
    if (c.ledCount < 0 || c.ledCount>144) {
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
        if(pair.size()!=2 || !pair[0].isDouble() || !pair[1].isDouble()
            || pair[0].toDouble() < -100000 || pair[0].toDouble() > 100000
            || pair[1].toDouble() < -100000 || pair[1].toDouble() > 100000)
            throw std::invalid_argument("Invalid LED coordinate");
        c.ledCoordinates.emplace_back(pair.at(0).toInt(), pair.at(1).toInt());
    }

    const QJsonArray namesArr = obj["LedNames"].toArray();
    for (const QJsonValue& v : namesArr) {
        c.ledNames.push_back(v.toString().toStdString());
    }

    truncateOrThrow(c.ledMapping, c.ledCount, "LedMapping");
    truncateOrThrow(c.ledCoordinates, c.ledCount, "LedCoordinates");
    truncateOrThrow(c.ledNames, c.ledCount, "LedNames");

    c.physicalLedsPerAddress=std::clamp(obj.value("PhysicalLedsPerAddress").toInt(c.type=="FanPaired"?2:1),1,16);
    for(const auto& value:obj.value("LayoutDefaults").toArray()) {
        const auto pair=value.toArray();
        if(pair.size()!=2 || !pair[0].isDouble() || !pair[1].isDouble() || std::abs(pair[0].toDouble())>100000 || std::abs(pair[1].toDouble())>100000)
            throw std::invalid_argument("Invalid reset coordinate");
        c.layoutDefaults.emplace_back(pair[0].toInt(),pair[1].toInt());
    }
    if(!c.valid() && c.ledCount>0)throw std::invalid_argument("Invalid component topology");
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
    obj["PhysicalLedsPerAddress"]=physicalLedsPerAddress;
    QJsonArray defaults;for(const auto& [x,y]:layoutDefaults)defaults.append(QJsonArray{x,y});obj["LayoutDefaults"]=defaults;

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

Component Component::makeFan(int regions, bool paired, int index)
{
    if (regions < 1 || regions > 120 || (paired && regions != 6) || index < 0 || index > 120) throw std::invalid_argument("Invalid fan geometry");
    Component c = makeGenericPlaceholder(regions);
    c.type = paired ? "FanPaired" : "Fan";
    c.productName = paired ? "Generic fan: 12 LEDs / 6 regions" : "Generic ARGB fan";
    c.displayName = "Fan " + std::to_string(index + 1);
    c.width = c.height = 100;
    c.physicalLedsPerAddress=paired?2:1;
    for (int i = 0; i < regions; ++i) {
        const double angle = 6.283185307179586 * i / regions - 1.5707963267948966;
        c.ledCoordinates[i] = {static_cast<int>(std::lround(45 + 35 * std::cos(angle))) + (index % 3) * 110,
                               static_cast<int>(std::lround(45 + 35 * std::sin(angle))) + (index / 3) * 110};
        c.ledNames[i] = paired ? "Region " + std::to_string(i + 1) + " (LED pair)" : "LED " + std::to_string(i + 1);
    }
    return c;
}

bool Component::valid() const {
    if(ledCount<1 || ledCount>144 || physicalLedsPerAddress<1 || physicalLedsPerAddress>16
        || ledCoordinates.size()!=size_t(ledCount) || ledNames.size()!=size_t(ledCount) || ledMapping.size()!=size_t(ledCount)
        || (!layoutDefaults.empty() && layoutDefaults.size()!=size_t(ledCount)))return false;
    std::vector<bool> used(ledCount,false);
    for(int address:ledMapping){if(address<0 || address>=ledCount || used[address])return false;used[address]=true;}
    for(const auto* points:{&ledCoordinates,&layoutDefaults})for(const auto& p:*points)
        if(p.first < -100000 || p.first>100000 || p.second < -100000 || p.second>100000)return false;
    return true;
}

bool Component::arrange(const std::string& shape) {
    if(!valid() || (shape!="Line" && shape!="Circle" && shape!="Ellipse" && shape!="Rectangle"))return false;
    if(layoutDefaults.empty())layoutDefaults=ledCoordinates;
    int left=ledCoordinates[0].first,top=ledCoordinates[0].second,right=left,bottom=top;
    for(const auto& [x,y]:ledCoordinates){left=std::min(left,x);right=std::max(right,x);top=std::min(top,y);bottom=std::max(bottom,y);}
    const double w=std::max(80,right-left),h=std::max(80,bottom-top);
    for(int i=0;i<ledCount;++i){double x=0,y=0,t=i/double(ledCount),angle=t*6.283185307-1.570796327;
        if(shape=="Line")x=ledCount>1?i*w/(ledCount-1):w/2;
        else if(shape=="Circle" || shape=="Ellipse"){const double ry=shape=="Circle"?w/2:h*.32;x=w/2+w/2*std::cos(angle);y=h/2+ry*std::sin(angle);}
        else {double edge=t*4.;int side=int(edge);double f=edge-side;if(side==0)x=f*w;else if(side==1){x=w;y=f*h;}else if(side==2){x=(1-f)*w;y=h;}else y=(1-f)*h;}
        ledCoordinates[i]={int(std::lround(left+x)),int(std::lround(top+y))};
    }
    return true;
}
void Component::resetLayout(){if(!layoutDefaults.empty())ledCoordinates=layoutDefaults;}

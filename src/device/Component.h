// Component data model: one physical LED product (fan, ARGB strip, etc.)
// described as human-editable JSON. Pure data/parsing — no HID, no I/O
// beyond JSON, no dependency on the protocol/HID transport layers.
#pragma once

#include <string>
#include <utility>
#include <vector>

#include <QJsonObject>

struct Component {
    std::string productName, displayName, brand, type;
    int ledCount = 0;
    int width = 0, height = 0;
    std::vector<int> ledMapping;
    std::vector<std::pair<int,int>> ledCoordinates; // [x,y] per LED
    std::vector<std::string> ledNames;
    std::string imageUrl;

    // Throws std::invalid_argument if ledCoordinates, ledNames, or ledMapping
    // is shorter than ledCount (can't safely synthesize missing per-LED
    // data). If any is LONGER than ledCount, truncates it to ledCount
    // (tolerates trailing junk in a hand-edited file).
    static Component fromJson(const QJsonObject& obj);
    QJsonObject toJson() const;

    // A single horizontal row of `ledCount` LEDs at y=0, x=0..ledCount-1,
    // named "LED 1".."LED N", productName/displayName="Generic Placeholder".
    static Component makeGenericPlaceholder(int ledCount);
};

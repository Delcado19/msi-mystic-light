// Component data model: one physical LED product (fan, ARGB strip, etc.)
// described as human-editable JSON. Pure data/parsing — no HID, no I/O
// beyond JSON, no dependency on the protocol/HID transport layers.
#pragma once

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include <QJsonObject>

struct Component {
    std::string productName, displayName, brand, type;
    int ledCount = 0;
    int width = 0, height = 0;
    std::vector<int> ledMapping; // Logical region -> local wire address; must be a permutation.
    std::vector<std::pair<int,int>> ledCoordinates; // [x,y] per LED
    std::vector<std::string> ledNames;
    std::string imageUrl;
    int physicalLedsPerAddress=1;
    std::vector<std::pair<int,int>> layoutDefaults;
    bool valid() const;
    // Coincident logical LEDs form one software group, including LED-path effects.
    int effectAddress(int logical) const {
        int address=ledMapping[logical];
        for(int i=0;i<ledCount;++i)if(ledCoordinates[i]==ledCoordinates[logical])address=std::min(address,ledMapping[i]);
        return address;
    }
    // Shape changes move logical addresses; wire order never changes.
    bool arrange(const std::string& shape);
    void resetLayout();

    // Throws std::invalid_argument if ledCoordinates, ledNames, or ledMapping
    // is shorter than ledCount (can't safely synthesize missing per-LED
    // data). If any is LONGER than ledCount, truncates it to ledCount
    // (tolerates trailing junk in a hand-edited file). Also throws
    // std::invalid_argument if ledCount itself is negative (otherwise the
    // truncate step's vector::resize(negative) would throw std::length_error
    // instead, breaking this contract).
    static Component fromJson(const QJsonObject& obj);
    QJsonObject toJson() const;

    // A single horizontal row of `ledCount` LEDs at y=0, x=0..ledCount-1,
    // named "LED 1".."LED N", productName/displayName="Generic Placeholder".
    static Component makeGenericPlaceholder(int ledCount);
    // Generic, explicitly configured addressable regions; no fan-model autodetection.
    static Component makeFan(int regions, bool paired, int index = 0);
};

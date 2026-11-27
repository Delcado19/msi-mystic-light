#pragma once

#include <QFileInfoList>
#include <QJsonObject>
#include <QString>
#include <QStringList>
#include <QVector>

struct DeviceIdentity {
    QString path;
    quint16 vendorId = 0, productId = 0;
    // -1 distinguishes unavailable descriptor fields from a reported zero.
    int usagePage = -1, usage = -1, interfaceNumber = -1;
    QString boardManufacturer, boardModel;
    QString transport = "hid";
};

struct ChannelProfile {
    QString id;
    int maxLeds = 0;
    int fixedLeds = 0; // Zero means the attached LED count is not known.
    bool direct = false;
    QStringList nativeModes;
    QJsonObject address; // Validated by the owning protocol module.
    // Optional: the connector name printed on the board (e.g. JARGB_V2_1) and its electrical kind
    // ("argb-5v" or "rgb-12v"), shown in Devices and the Attach dialog (plan 2026-11-27, Task 1).
    // Last, so existing aggregate initializers keep their member order.
    QString label, kind;
};

struct DeviceProfile {
    QString id, driverId, name;
    QJsonObject match;
    QVector<ChannelProfile> channels;

    // Descriptor matching only: never opens a device or sends probe reports.
    bool matches(const DeviceIdentity& device) const;
    static DeviceProfile fromJson(const QJsonObject& json);
    static DeviceProfile load(const QString& fileName);
};

// Printed label when the profile has one, otherwise the channel ID.
QString channelDisplayName(const ChannelProfile& channel);
// "5 V ARGB", "12 V RGB" or empty when the profile does not state the kind.
QString channelKindText(const ChannelProfile& channel);

// Profile files of a profiles folder, sorted by name. Files in its "experimental" subfolder are added
// only with includeExperimental: the AJAZZ AK35I V4 shares VID:PID 0C45:8009 with other keyboards, so the
// editor loads it only after the owner's explicit opt-in in Edit > Settings (decision D1, 2026-11-26).
// The service always loads it; it sends only to channels a project assigns, and only an opted-in editor
// can assign it (docs/AJAZZ_AK35I_V4_PROTOCOL.md).
QFileInfoList deviceProfileFiles(const QString& directory, bool includeExperimental);

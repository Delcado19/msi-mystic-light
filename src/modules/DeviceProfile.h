#pragma once

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

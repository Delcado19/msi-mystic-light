#pragma once

#include "modules/DeviceProfile.h"
#include <QMap>
#include <QPointF>

struct ComponentDefinition {
    QString id, name;
    // Optional catalog identity; older documents use presentation fallbacks.
    QString manufacturer, category;
    QVector<QPointF> positions;
    // Logical LED index -> wire index; a permutation, never extra LEDs.
    QVector<int> mapping;
    // Reserved wire slots remain in reports but have no physical LED to arrange.
    QVector<int> nonPhysicalSlots;
    QJsonObject toJson() const;
    static ComponentDefinition fromJson(const QJsonObject& json);
};

using ComponentCatalog = QMap<QString, ComponentDefinition>;
// All-or-nothing import of data only. Duplicate IDs are errors, not overrides.
ComponentDefinition loadComponentDefinition(const QString& fileName);
ComponentCatalog loadComponentCatalog(const QString& directory);

// Instances are listed in physical chain order; repeating a product is valid.
// Returns logical LED -> channel wire index, checking capacity and fixed counts.
QVector<int> channelMapping(const ChannelProfile& channel,
                           const QStringList& componentIds,
                           const ComponentCatalog& catalog);

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
// starts is empty (serial chain) or has one entry per component: 0 places it directly
// after the previous component, n >= 1 is an explicit one-based output position.
// Explicit positions may overlap: mirrored components on one header share addresses
// (docs/JARGB2_TOPOLOGY_TEST.md).
QVector<int> channelMapping(const ChannelProfile& channel,
                           const QStringList& componentIds,
                           const ComponentCatalog& catalog,
                           const QVector<int>& starts = {});
// Zero-based first output position of each chain component, using the starts rules above.
QVector<int> chainOffsets(const QVector<int>& counts, const QVector<int>& starts);
// A run of one chain item's LEDs (one-based, inclusive, in wire order) and the chain item
// that supplies their colors: the item itself, or an earlier item whose output positions
// overlap (the earlier item wins, as in LightingDocument::sampleChannels).
struct SharedRun { int firstLed = 1, lastLed = 1, source = 0; };
QVector<QVector<SharedRun>> chainSharing(const QVector<int>& counts, const QVector<int>& starts);

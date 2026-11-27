#pragma once

#include "modules/DeviceProfile.h"
#include <QMap>
#include <QPointF>
#include <QRectF>

struct ComponentDefinition {
    QString id, name;
    // Optional catalog identity; older documents use presentation fallbacks.
    QString manufacturer, category;
    // How the LED layout was confirmed (plan 2026-11-27 Task 4): "verified" (tested on real hardware),
    // "manufacturer" (from vendor data) or "unverified"; empty counts as unverified.
    QString verification;
    QVector<QPointF> positions;
    // Each physical point has a wire index. Explicit address counts allow several points per address.
    int addressCount = 0;
    int addresses() const { return addressCount ? addressCount : int(mapping.size()); }
    QVector<int> mapping;
    // Reserved wire slots remain in reports but have no physical LED to arrange.
    QVector<int> nonPhysicalSlots;
    // Display-only depth layer per position (0 = front), for LEDs physically behind each other.
    // Empty means all layer 0. Never used for sampling or output (owner request 2026-11-25).
    QVector<int> layers;
    int layerOf(int row) const { return layers.value(row, 0); }
    // Housing frame in component units (owner decision "Option A", 2026-11-26): the artwork is drawn
    // into it and LEDs are kept inside it in the Component Editor. Null = derived from the LED bounds
    // (older definitions, and strips/cables, whose housing grows with the LEDs).
    QRectF housing;
    QJsonObject toJson() const;
    static ComponentDefinition fromJson(const QJsonObject& json);
};

using ComponentCatalog = QMap<QString, ComponentDefinition>;
// The n-th repeat of an exactly equal point gets layer n; empty when no point repeats.
QVector<int> overlapLayers(const QVector<QPointF>& positions);
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

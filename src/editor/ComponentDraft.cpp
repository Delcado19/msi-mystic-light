#include "ComponentDraft.h"
#include <QMap>
#include <QRectF>
#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <tuple>

namespace {
constexpr double circle = 6.283185307179586;

void requireUnlocked(const ComponentDraft& draft)
{
    if (draftLocksCount(draft)) throw std::invalid_argument("This device has a fixed LED count");
}
// Renumbers addresses to 0..n-1 in ascending order of their current value.
void compress(ComponentDraft& draft)
{
    auto order = QSet<int>(draft.mapping.cbegin(), draft.mapping.cend()).values();
    std::sort(order.begin(), order.end());
    for (auto& address : draft.mapping) address = int(order.indexOf(address));
}
QVector<int> targetRows(const ComponentDraft& draft, const QSet<int>& rows)
{
    QVector<int> result;
    for (int row = 0; row < draft.positions.size(); ++row)
        if (rows.isEmpty() ? !draft.nonPhysicalSlots.contains(draft.mapping[row]) : rows.contains(row)) result.append(row);
    return result;
}
}

ComponentDraft ComponentDraft::fromDefinition(const ComponentDefinition& definition)
{
    return {definition.id, definition.name, definition.manufacturer, definition.category,
            definition.positions, definition.mapping, definition.nonPhysicalSlots, definition.layers};
}

ComponentDefinition ComponentDraft::toDefinitionUnchecked() const
{
    ComponentDefinition definition;
    definition.id = id; definition.name = name; definition.manufacturer = manufacturer; definition.category = category;
    definition.positions = positions; definition.mapping = mapping; definition.nonPhysicalSlots = nonPhysicalSlots;
    // Saved definitions carry no empty layers: renumber the used ones in their order.
    auto used = QSet<int>(layers.cbegin(), layers.cend()).values();
    std::sort(used.begin(), used.end());
    for (const int layer : layers) definition.layers.append(int(used.indexOf(layer)));
    return definition;
}

ComponentDefinition ComponentDraft::toDefinition() const
{
    auto definition = toDefinitionUnchecked();
    const int distinct = int(QSet<int>(mapping.cbegin(), mapping.cend()).size());
    if (distinct < mapping.size()) definition.addressCount = distinct;
    return ComponentDefinition::fromJson(definition.toJson());
}

bool ComponentDraft::operator==(const ComponentDraft& other) const
{
    return id == other.id && name == other.name && manufacturer == other.manufacturer && category == other.category
        && positions == other.positions && mapping == other.mapping && nonPhysicalSlots == other.nonPhysicalSlots
        && layers == other.layers;
}

int ComponentDraft::layerCount() const
{
    return layers.isEmpty() ? 1 : *std::max_element(layers.cbegin(), layers.cend()) + 1;
}

ComponentDraft withLayer(ComponentDraft draft, const QSet<int>& rows, int layer)
{
    if (layer < 0 || layer > draft.layerCount() || layer > 15) throw std::invalid_argument("Invalid LED layer");
    draft.layers.resize(draft.positions.size());
    for (const int row : rows) if (row >= 0 && row < draft.layers.size()) draft.layers[row] = layer;
    if (std::all_of(draft.layers.cbegin(), draft.layers.cend(), [](int value) { return value == 0; })) draft.layers.clear();
    return draft;
}

bool draftLocksCount(const ComponentDraft& draft) { return !draft.nonPhysicalSlots.isEmpty(); }

ComponentDraft withShape(ComponentDraft draft, DraftShape shape, int count)
{
    requireUnlocked(draft);
    count = std::max(1, count);
    draft.positions.clear(); draft.mapping.clear(); draft.layers.clear();
    // Matrix, L and U use one component unit between neighbours; ring/fan/strip keep the
    // geometry of the hardware picker templates (radius 1.3, strip width 3).
    const int columns = int(std::ceil(std::sqrt(double(count))));
    const int length = count - 1;
    const int legL = length / 2, legU = length / 3;
    for (int i = 0; i < count; ++i) {
        QPointF point;
        switch (shape) {
        case DraftShape::Ring: case DraftShape::Fan: {
            const double angle = circle * i / count;
            point = {1.3 + 1.3 * std::cos(angle), 1.3 + 1.3 * std::sin(angle)};
            break;
        }
        case DraftShape::Strip: point = {count > 1 ? 3.0 * i / length : 0.0, 0}; break;
        case DraftShape::Matrix: point = {double(i % columns), double(i / columns)}; break;
        case DraftShape::LShape:
            point = i <= legL ? QPointF(0, i) : QPointF(i - legL, legL);
            break;
        case DraftShape::UShape:
            if (i <= legU) point = {0, double(i)};
            else if (i <= length - legU) point = {double(i - legU), double(legU)};
            else point = {double(length - 2 * legU), double(length - i)};
            break;
        }
        draft.positions.append(point); draft.mapping.append(i);
    }
    if (shape == DraftShape::Fan) draft.category = "Fans";
    if (shape == DraftShape::Strip) draft.category = "Strips";
    return draft;
}

ComponentDraft withReshaped(ComponentDraft draft, DraftShape shape)
{
    const int count = int(draft.positions.size());
    if (count == 0) return draft;
    // Positions come from the unlocked template generator; nothing but positions is taken over.
    const auto templatePoints = withShape(ComponentDraft{}, shape, count).positions;
    QVector<int> rows(count);
    for (int i = 0; i < count; ++i) rows[i] = i;
    std::stable_sort(rows.begin(), rows.end(), [&](int a, int b) { return draft.mapping.value(a) < draft.mapping.value(b); });
    // Keep the part where it was: the template is scaled uniformly to the current LEDs' larger extent and
    // centred on them, so a reshaped RAM stick or fan stays on its artwork instead of the template origin.
    // Min/max by hand: QRectF::united ignores zero-size rects, so point rects cannot build the box.
    const auto bounds = [](const QVector<QPointF>& points) {
        double left = points.first().x(), right = left, top = points.first().y(), bottom = top;
        for (const auto& point : points) {
            left = std::min(left, point.x()); right = std::max(right, point.x());
            top = std::min(top, point.y()); bottom = std::max(bottom, point.y());
        }
        return QRectF(QPointF(left, top), QPointF(right, bottom));
    };
    const auto current = bounds(draft.positions), shapeBox = bounds(templatePoints);
    const double currentExtent = std::max(current.width(), current.height());
    const double shapeExtent = std::max(shapeBox.width(), shapeBox.height());
    const double scale = currentExtent > 1e-9 && shapeExtent > 1e-9 ? currentExtent / shapeExtent : 1.0;
    for (int i = 0; i < count; ++i) draft.positions[rows[i]] = current.center() + (templatePoints[i] - shapeBox.center()) * scale;
    return draft;
}

ComponentDraft withWireOrder(ComponentDraft draft, const QVector<int>& addressOrder)
{
    QHash<int, int> addresses;
    for (int i = 0; i < addressOrder.size(); ++i) addresses.insert(addressOrder[i], i);
    for (auto& address : draft.mapping) address = addresses.value(address, address);
    return draft;
}

ComponentDraft withPreset(ComponentDraft draft, WiringPreset preset)
{
    if (draft.positions.isEmpty()) return draft;
    // Ported from the former ComponentEditor: one reference point per address.
    QMap<int, QPointF> positions;
    for (int row = 0; row < draft.positions.size(); ++row)
        if (!positions.contains(draft.mapping[row])) positions.insert(draft.mapping[row], draft.positions[row]);
    auto order = positions.keys();
    QPointF center;
    for (const auto& point : positions) center += point;
    center /= positions.size();
    const auto key = [&](int id) {
        const auto point = positions.value(id);
        if (preset == WiringPreset::Clockwise) {
            const auto delta = point - center;
            return std::tuple{std::fmod(std::atan2(delta.x(), -delta.y()) + circle, circle), 0.0, id};
        }
        if (preset == WiringPreset::ColumnsTopToBottom) return std::tuple{double(qRound(point.x() * 10)), point.y(), id};
        return std::tuple{double(qRound(point.y() * 10)), point.x(), id};
    };
    std::sort(order.begin(), order.end(), [&](int a, int b) { return key(a) < key(b); });
    if (preset == WiringPreset::RowsSerpentine) {
        // Alternate the traversal of successive occupied rows, including across gaps.
        bool reverse = false;
        for (auto first = order.begin(); first != order.end();) {
            auto last = std::find_if(first, order.end(), [&](int id) { return std::get<0>(key(id)) != std::get<0>(key(*first)); });
            if (reverse) std::reverse(first, last);
            reverse = !reverse;
            first = last;
        }
    }
    return withWireOrder(draft, order);
}

ComponentDraft withSharedAddress(ComponentDraft draft, const QSet<int>& rows, int address)
{
    requireUnlocked(draft);
    for (const int row : rows) if (row >= 0 && row < draft.mapping.size()) draft.mapping[row] = address;
    compress(draft);
    return draft;
}

ComponentDraft withMirror(ComponentDraft draft, const QSet<int>& rows, Qt::Orientation orientation)
{
    const auto targets = targetRows(draft, rows);
    if (targets.isEmpty()) return draft;
    const auto coordinate = [&](int row) -> qreal& {
        return orientation == Qt::Horizontal ? draft.positions[row].rx() : draft.positions[row].ry();
    };
    double low = coordinate(targets.first()), high = low;
    for (const int row : targets) { low = std::min(low, double(coordinate(row))); high = std::max(high, double(coordinate(row))); }
    for (const int row : targets) coordinate(row) = low + high - coordinate(row);
    return draft;
}

ComponentDraft withRotation90(ComponentDraft draft, const QSet<int>& rows)
{
    const auto targets = targetRows(draft, rows);
    if (targets.isEmpty()) return draft;
    QRectF bounds(draft.positions[targets.first()], QSizeF());
    for (const int row : targets) {
        const auto point = draft.positions[row];
        bounds = QRectF(QPointF(std::min(bounds.left(), point.x()), std::min(bounds.top(), point.y())),
                        QPointF(std::max(bounds.right(), point.x()), std::max(bounds.bottom(), point.y())));
    }
    const auto center = bounds.center();
    // Clockwise on screen (y points down).
    for (const int row : targets) {
        const auto delta = draft.positions[row] - center;
        draft.positions[row] = {center.x() - delta.y(), center.y() + delta.x()};
    }
    return draft;
}

ComponentDraft withAdded(ComponentDraft draft, QPointF point, int layer)
{
    requireUnlocked(draft);
    const int next = draft.mapping.isEmpty() ? 0 : *std::max_element(draft.mapping.cbegin(), draft.mapping.cend()) + 1;
    if (layer > 0 || !draft.layers.isEmpty()) { draft.layers.resize(draft.positions.size()); draft.layers.append(layer); }
    draft.positions.append(point); draft.mapping.append(next);
    return draft;
}

ComponentDraft withRemoved(ComponentDraft draft, const QSet<int>& rows)
{
    requireUnlocked(draft);
    auto sorted = rows.values();
    std::sort(sorted.begin(), sorted.end(), std::greater<int>());
    for (const int row : sorted)
        if (row >= 0 && row < draft.positions.size()) {
            draft.positions.removeAt(row); draft.mapping.removeAt(row);
            if (!draft.layers.isEmpty()) draft.layers.removeAt(row);
        }
    compress(draft);
    return draft;
}

ComponentDraft withDuplicated(ComponentDraft draft, const QSet<int>& rows, QPointF offset)
{
    requireUnlocked(draft);
    auto sorted = rows.values();
    std::sort(sorted.begin(), sorted.end());
    for (const int row : sorted)
        if (row >= 0 && row < draft.positions.size())
            draft = withAdded(draft, draft.positions[row] + offset, draft.layers.value(row, 0));
    return draft;
}

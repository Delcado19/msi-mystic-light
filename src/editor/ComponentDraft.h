#pragma once
#include "components/ComponentCatalog.h"
#include <QSet>

// Editable value form of a ComponentDefinition for the Component Editor. Plain value semantics make
// whole-draft snapshots a complete undo record; ComponentDefinition::fromJson stays the validator.
struct ComponentDraft {
    QString id, name, manufacturer, category;
    QVector<QPointF> positions;   // component units
    QVector<int> mapping;         // zero-based address per position
    QVector<int> nonPhysicalSlots;
    static ComponentDraft fromDefinition(const ComponentDefinition& definition);
    ComponentDefinition toDefinition() const;
    // Field copy without validation, for presentation (names, artwork) of drafts that may be incomplete.
    ComponentDefinition toDefinitionUnchecked() const;
    bool operator==(const ComponentDraft& other) const;
};

enum class DraftShape { Matrix, Strip, Ring, Fan, LShape, UShape };
enum class WiringPreset { RowsLeftToRight, RowsSerpentine, ColumnsTopToBottom, Clockwise };

// Replaces every LED; addresses become 0..count-1. Fan and Strip also set the category.
ComponentDraft withShape(ComponentDraft draft, DraftShape shape, int count);
ComponentDraft withPreset(ComponentDraft draft, WiringPreset preset);
// Rows share `address` (zero-based); unused addresses are compressed afterwards.
ComponentDraft withSharedAddress(ComponentDraft draft, const QSet<int>& rows, int address);
// addressOrder[i] becomes address i; addresses missing from the order keep their value.
ComponentDraft withWireOrder(ComponentDraft draft, const QVector<int>& addressOrder);
// Empty rows means every physical LED.
ComponentDraft withMirror(ComponentDraft draft, const QSet<int>& rows, Qt::Orientation orientation);
ComponentDraft withRotation90(ComponentDraft draft, const QSet<int>& rows);
// Count-changing edits throw std::invalid_argument for protocol-locked drafts (nonPhysicalSlots).
ComponentDraft withAdded(ComponentDraft draft, QPointF point);
ComponentDraft withRemoved(ComponentDraft draft, const QSet<int>& rows);
ComponentDraft withDuplicated(ComponentDraft draft, const QSet<int>& rows, QPointF offset);
bool draftLocksCount(const ComponentDraft& draft);

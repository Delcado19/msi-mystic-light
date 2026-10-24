#pragma once
#include "components/ComponentCatalog.h"
#include <QDialog>

// Definition mode edits shared geometry; instance mode locks identity, count and wire mapping.
class ComponentEditor : public QDialog {
public:
    ComponentEditor(const ComponentDefinition& initial, bool existing, QWidget* parent = nullptr, bool instance = false);
    ComponentDefinition component() const;
private:
    QVector<int> nonPhysicalSlots_;
};

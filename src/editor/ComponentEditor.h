#pragma once
#include "components/ComponentCatalog.h"
#include <QDialog>

// Edits a document-owned definition; committing affects all instances of its ID.
class ComponentEditor : public QDialog {
public:
    ComponentEditor(const ComponentDefinition& initial, bool existing, QWidget* parent = nullptr);
    ComponentDefinition component() const;
};
